/****************************************************************************
* Copyright (C) 2014-2015 Intel Corporation.   All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
* @file backend.cpp
*
* @brief Backend handles rasterization, pixel shading and output merger
*        operations.
*
******************************************************************************/

#include <smmintrin.h>

#include "rdtsc_core.h"
#include "backend.h"
#include "depthstencil.h"
#include "tilemgr.h"
#include "memory/tilingtraits.h"
#include "core/multisample.h"

#include <algorithm>

const __m128 vTileOffsetsX = {0.5, KNOB_TILE_X_DIM - 0.5, 0.5, KNOB_TILE_X_DIM - 0.5};
const __m128 vTileOffsetsY = {0.5, 0.5, KNOB_TILE_Y_DIM - 0.5, KNOB_TILE_Y_DIM - 0.5};

/// @todo move to common lib
#define MASKTOVEC(i3,i2,i1,i0) {-i0,-i1,-i2,-i3}
static const __m128 gMaskToVec[] = {
    MASKTOVEC(0,0,0,0),
    MASKTOVEC(0,0,0,1),
    MASKTOVEC(0,0,1,0),
    MASKTOVEC(0,0,1,1),
    MASKTOVEC(0,1,0,0),
    MASKTOVEC(0,1,0,1),
    MASKTOVEC(0,1,1,0),
    MASKTOVEC(0,1,1,1),
    MASKTOVEC(1,0,0,0),
    MASKTOVEC(1,0,0,1),
    MASKTOVEC(1,0,1,0),
    MASKTOVEC(1,0,1,1),
    MASKTOVEC(1,1,0,0),
    MASKTOVEC(1,1,0,1),
    MASKTOVEC(1,1,1,0),
    MASKTOVEC(1,1,1,1),
};

typedef void(*PFN_CLEAR_TILES)(DRAW_CONTEXT*, SWR_RENDERTARGET_ATTACHMENT rt, uint32_t, DWORD[4]);
static PFN_CLEAR_TILES sClearTilesTable[NUM_SWR_FORMATS];

//////////////////////////////////////////////////////////////////////////
/// @brief Process compute work.
/// @param pDC - pointer to draw context (dispatch).
/// @param workerId - The unique worker ID that is assigned to this thread.
/// @param threadGroupId - the linear index for the thread group within the dispatch.
void ProcessComputeBE(DRAW_CONTEXT* pDC, uint32_t workerId, uint32_t threadGroupId)
{
    RDTSC_START(BEDispatch);

    SWR_CONTEXT *pContext = pDC->pContext;

    const COMPUTE_DESC* pTaskData = (COMPUTE_DESC*)pDC->pDispatch->GetTasksData();
    SWR_ASSERT(pTaskData != nullptr);

    const API_STATE& state = GetApiState(pDC);

    SWR_CS_CONTEXT csContext{ 0 };
    csContext.tileCounter = threadGroupId;
    csContext.dispatchDims[0] = pTaskData->threadGroupCountX;
    csContext.dispatchDims[1] = pTaskData->threadGroupCountY;
    csContext.dispatchDims[2] = pTaskData->threadGroupCountZ;
    csContext.pTGSM = pContext->pScratch[workerId];

    state.pfnCsFunc(GetPrivateState(pDC), &csContext);

    UPDATE_STAT(CsInvocations, state.totalThreadsInGroup);

    RDTSC_STOP(BEDispatch, 1, 0);
}

void ProcessSyncBE(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t macroTile, void *pUserData)
{
    SYNC_DESC *pSync = (SYNC_DESC*)pUserData;

    uint32_t x, y;
    MacroTileMgr::getTileIndices(macroTile, x, y);
    SWR_ASSERT(x == 0 && y == 0);

    if (pSync->pfnCallbackFunc != nullptr)
    {
        pSync->pfnCallbackFunc(pSync->userData, pSync->userData2);
    }
}

void ProcessQueryStatsBE(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t macroTile, void *pUserData)
{
    QUERY_DESC* pQueryDesc = (QUERY_DESC*)pUserData;
    SWR_STATS* pStats = pQueryDesc->pStats;
    SWR_CONTEXT *pContext = pDC->pContext;

    SWR_ASSERT(pStats != nullptr);

    for (uint32_t i = 0; i < pContext->NumWorkerThreads; ++i)
    {
        pStats->DepthPassCount += pContext->stats[i].DepthPassCount;

        pStats->IaVertices    += pContext->stats[i].IaVertices;
        pStats->IaPrimitives  += pContext->stats[i].IaPrimitives;
        pStats->VsInvocations += pContext->stats[i].VsInvocations;
        pStats->HsInvocations += pContext->stats[i].HsInvocations;
        pStats->DsInvocations += pContext->stats[i].DsInvocations;
        pStats->GsInvocations += pContext->stats[i].GsInvocations;
        pStats->PsInvocations += pContext->stats[i].PsInvocations;
        pStats->CInvocations  += pContext->stats[i].CInvocations;
        pStats->CsInvocations += pContext->stats[i].CsInvocations;
        pStats->CPrimitives   += pContext->stats[i].CPrimitives;
        pStats->GsPrimitives  += pContext->stats[i].GsPrimitives;

        for (uint32_t stream = 0; stream < MAX_SO_STREAMS; ++stream)
        {
            pStats->SoWriteOffset[stream] += pContext->stats[i].SoWriteOffset[stream];

            /// @note client is required to provide valid write offset before every draw, so we clear
            /// out the contents of the write offset when storing stats
            pContext->stats[i].SoWriteOffset[stream] = 0;

            pStats->SoPrimStorageNeeded[stream] += pContext->stats[i].SoPrimStorageNeeded[stream];
            pStats->SoNumPrimsWritten[stream] += pContext->stats[i].SoNumPrimsWritten[stream];
        }
    }
}

template<SWR_FORMAT format>
void ClearRasterTile(BYTE *pTileBuffer, simdvector &value)
{
    auto lambda = [&](int comp)
    {
        FormatTraits<format>::storeSOA(comp, pTileBuffer, value.v[comp]);
        pTileBuffer += (KNOB_SIMD_WIDTH * FormatTraits<format>::GetBPC(comp) / 8);
    };

    const uint32_t numIter = (KNOB_TILE_Y_DIM / SIMD_TILE_Y_DIM) * (KNOB_TILE_X_DIM / SIMD_TILE_X_DIM);
    for (uint32_t i = 0; i < numIter; ++i)
    {
        UnrollerL<0, FormatTraits<format>::numComps, 1>::step(lambda);
    }
}

template<SWR_FORMAT format>
INLINE void ClearMacroTile(DRAW_CONTEXT *pDC, SWR_RENDERTARGET_ATTACHMENT rt, uint32_t macroTile, DWORD clear[4])
{
    // convert clear color to hottile format
    // clear color is in RGBA float/uint32
    simdvector vClear;
    for (uint32_t comp = 0; comp < FormatTraits<format>::numComps; ++comp)
    {
        simdscalar vComp;
        vComp = _simd_load1_ps((const float*)&clear[comp]);
        if (FormatTraits<format>::isNormalized(comp))
        {
            vComp = _simd_mul_ps(vComp, _simd_set1_ps(FormatTraits<format>::fromFloat(comp)));
            vComp = _simd_castsi_ps(_simd_cvtps_epi32(vComp));
        }
        vComp = FormatTraits<format>::pack(comp, vComp);
        vClear.v[FormatTraits<format>::swizzle(comp)] = vComp;
    }

    uint32_t tileX, tileY;
    MacroTileMgr::getTileIndices(macroTile, tileX, tileY);
    const API_STATE& state = GetApiState(pDC);
    
    int top = KNOB_MACROTILE_Y_DIM_FIXED * tileY;
    int bottom = top + KNOB_MACROTILE_Y_DIM_FIXED - 1;
    int left = KNOB_MACROTILE_X_DIM_FIXED * tileX;
    int right = left + KNOB_MACROTILE_X_DIM_FIXED - 1;

    // intersect with scissor
    top = std::max(top, state.scissorInFixedPoint.top);
    left = std::max(left, state.scissorInFixedPoint.left);
    bottom = std::min(bottom, state.scissorInFixedPoint.bottom);
    right = std::min(right, state.scissorInFixedPoint.right);

    // translate to local hottile origin
    top -= KNOB_MACROTILE_Y_DIM_FIXED * tileY;
    bottom -= KNOB_MACROTILE_Y_DIM_FIXED * tileY;
    left -= KNOB_MACROTILE_X_DIM_FIXED * tileX;
    right -= KNOB_MACROTILE_X_DIM_FIXED * tileX;

    // convert to raster tiles
    top >>= (KNOB_TILE_Y_DIM_SHIFT + FIXED_POINT_SHIFT);
    bottom >>= (KNOB_TILE_Y_DIM_SHIFT + FIXED_POINT_SHIFT);
    left >>= (KNOB_TILE_X_DIM_SHIFT + FIXED_POINT_SHIFT);
    right >>= (KNOB_TILE_X_DIM_SHIFT + FIXED_POINT_SHIFT);

    const int numSamples = GetNumSamples(pDC->pState->state.rastState.sampleCount);
    // compute steps between raster tile samples / raster tiles / macro tile rows
    const uint32_t rasterTileSampleStep = KNOB_TILE_X_DIM * KNOB_TILE_Y_DIM * FormatTraits<format>::bpp / 8;
    const uint32_t rasterTileStep = (KNOB_TILE_X_DIM * KNOB_TILE_Y_DIM * (FormatTraits<format>::bpp / 8)) * numSamples;
    const uint32_t macroTileRowStep = (KNOB_MACROTILE_X_DIM / KNOB_TILE_X_DIM) * rasterTileStep;
    const uint32_t pitch = (FormatTraits<format>::bpp * KNOB_MACROTILE_X_DIM / 8);

    HOTTILE *pHotTile = pDC->pContext->pHotTileMgr->GetHotTile(pDC->pContext, pDC, macroTile, rt, true, numSamples);
    uint32_t rasterTileStartOffset = (ComputeTileOffset2D< TilingTraits<SWR_TILE_SWRZ, FormatTraits<format>::bpp > >(pitch, left, top)) * numSamples;
    uint8_t* pRasterTileRow = pHotTile->pBuffer + rasterTileStartOffset; //(ComputeTileOffset2D< TilingTraits<SWR_TILE_SWRZ, FormatTraits<format>::bpp > >(pitch, x, y)) * numSamples;

    // loop over all raster tiles in the current hot tile
    for (int y = top; y <= bottom; ++y)
    {
        uint8_t* pRasterTile = pRasterTileRow;
        for (int x = left; x <= right; ++x)
        {
            for( int sampleNum = 0; sampleNum < numSamples; sampleNum++)
            {
                ClearRasterTile<format>(pRasterTile, vClear);
                pRasterTile += rasterTileSampleStep;
            }
        }
        pRasterTileRow += macroTileRowStep;
    }

    pHotTile->state = HOTTILE_DIRTY;
}


void ProcessClearBE(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t macroTile, void *pUserData)
{
    if (KNOB_FAST_CLEAR)
    {
        CLEAR_DESC *pClear = (CLEAR_DESC*)pUserData;
        SWR_CONTEXT *pContext = pDC->pContext;
        SWR_MULTISAMPLE_COUNT sampleCount = pDC->pState->state.rastState.sampleCount;
        uint32_t numSamples = GetNumSamples(sampleCount);

        SWR_ASSERT(pClear->flags.bits != 0); // shouldn't be here without a reason.

        RDTSC_START(BEClear);

        if (pClear->flags.mask & SWR_CLEAR_COLOR)
        {
            HOTTILE *pHotTile = pContext->pHotTileMgr->GetHotTile(pContext, pDC, macroTile, SWR_ATTACHMENT_COLOR0, true, numSamples);
            // All we want to do here is to mark the hot tile as being in a "needs clear" state.
            pHotTile->clearData[0] = *(DWORD*)&(pClear->clearRTColor[0]);
            pHotTile->clearData[1] = *(DWORD*)&(pClear->clearRTColor[1]);
            pHotTile->clearData[2] = *(DWORD*)&(pClear->clearRTColor[2]);
            pHotTile->clearData[3] = *(DWORD*)&(pClear->clearRTColor[3]);
            pHotTile->state = HOTTILE_CLEAR;
        }

        if (pClear->flags.mask & SWR_CLEAR_DEPTH)
        {
            HOTTILE *pHotTile = pContext->pHotTileMgr->GetHotTile(pContext, pDC, macroTile, SWR_ATTACHMENT_DEPTH, true, numSamples);
            pHotTile->clearData[0] = *(DWORD*)&pClear->clearDepth;
            pHotTile->state = HOTTILE_CLEAR;
        }

        if (pClear->flags.mask & SWR_CLEAR_STENCIL)
        {
            HOTTILE *pHotTile = pContext->pHotTileMgr->GetHotTile(pContext, pDC, macroTile, SWR_ATTACHMENT_STENCIL, true, numSamples);

            pHotTile->clearData[0] = *(DWORD*)&pClear->clearStencil;
            pHotTile->state = HOTTILE_CLEAR;
        }

        RDTSC_STOP(BEClear, 0, 0);
    }
    else
    {
        // Legacy clear
        CLEAR_DESC *pClear = (CLEAR_DESC*)pUserData;
        RDTSC_START(BEClear);

        if (pClear->flags.mask & SWR_CLEAR_COLOR)
        {
            /// @todo clear data should come in as RGBA32_FLOAT
            DWORD clearData[4];
            float clearFloat[4];
            clearFloat[0] = ((BYTE*)(&pClear->clearRTColor))[0] / 255.0f;
            clearFloat[1] = ((BYTE*)(&pClear->clearRTColor))[1] / 255.0f;
            clearFloat[2] = ((BYTE*)(&pClear->clearRTColor))[2] / 255.0f;
            clearFloat[3] = ((BYTE*)(&pClear->clearRTColor))[3] / 255.0f;
            clearData[0] = *(DWORD*)&clearFloat[0];
            clearData[1] = *(DWORD*)&clearFloat[1];
            clearData[2] = *(DWORD*)&clearFloat[2];
            clearData[3] = *(DWORD*)&clearFloat[3];

            PFN_CLEAR_TILES pfnClearTiles = sClearTilesTable[KNOB_COLOR_HOT_TILE_FORMAT];
            SWR_ASSERT(pfnClearTiles != nullptr);

            pfnClearTiles(pDC, SWR_ATTACHMENT_COLOR0, macroTile, clearData);
        }

        if (pClear->flags.mask & SWR_CLEAR_DEPTH)
        {
            DWORD clearData[4];
            clearData[0] = *(DWORD*)&pClear->clearDepth;
            PFN_CLEAR_TILES pfnClearTiles = sClearTilesTable[KNOB_DEPTH_HOT_TILE_FORMAT];
            SWR_ASSERT(pfnClearTiles != nullptr);

            pfnClearTiles(pDC, SWR_ATTACHMENT_DEPTH, macroTile, clearData);
        }

        if (pClear->flags.mask & SWR_CLEAR_STENCIL)
        {
            uint32_t value = pClear->clearStencil;
            DWORD clearData[4];
            clearData[0] = *(DWORD*)&value;
            PFN_CLEAR_TILES pfnClearTiles = sClearTilesTable[KNOB_STENCIL_HOT_TILE_FORMAT];

            pfnClearTiles(pDC, SWR_ATTACHMENT_STENCIL, macroTile, clearData);
        }

        RDTSC_STOP(BEClear, 0, 0);
    }
}


void ProcessStoreTileBE(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t macroTile, void *pData)
{
    RDTSC_START(BEStoreTiles);
    STORE_TILES_DESC *pDesc = (STORE_TILES_DESC*)pData;
    SWR_CONTEXT *pContext = pDC->pContext;

#ifdef KNOB_ENABLE_RDTSC
    uint32_t numTiles = 0;
#endif
    SWR_FORMAT srcFormat;
    switch (pDesc->attachment)
    {
    case SWR_ATTACHMENT_COLOR0:
    case SWR_ATTACHMENT_COLOR1:
    case SWR_ATTACHMENT_COLOR2:
    case SWR_ATTACHMENT_COLOR3:
    case SWR_ATTACHMENT_COLOR4:
    case SWR_ATTACHMENT_COLOR5:
    case SWR_ATTACHMENT_COLOR6:
    case SWR_ATTACHMENT_COLOR7: srcFormat = KNOB_COLOR_HOT_TILE_FORMAT; break;
    case SWR_ATTACHMENT_DEPTH: srcFormat = KNOB_DEPTH_HOT_TILE_FORMAT; break;
    case SWR_ATTACHMENT_STENCIL: srcFormat = KNOB_STENCIL_HOT_TILE_FORMAT; break;
    default: SWR_ASSERT(false, "Unknown attachment: %d", pDesc->attachment); srcFormat = KNOB_COLOR_HOT_TILE_FORMAT; break;
    }

    uint32_t x, y;
    MacroTileMgr::getTileIndices(macroTile, x, y);

    // Only need to store the hottile if it's been rendered to...
    HOTTILE *pHotTile = pContext->pHotTileMgr->GetHotTile(pContext, pDC, macroTile, pDesc->attachment, false);
    if (pHotTile)
    {
        // clear if clear is pending (i.e., not rendered to), then mark as dirty for store.
        if (pHotTile->state == HOTTILE_CLEAR)
        {
            PFN_CLEAR_TILES pfnClearTiles = sClearTilesTable[srcFormat];
            SWR_ASSERT(pfnClearTiles != nullptr);

            pfnClearTiles(pDC, pDesc->attachment, macroTile, pHotTile->clearData);
        }

        if (pHotTile->state == HOTTILE_DIRTY || pDesc->postStoreTileState == (SWR_TILE_STATE)HOTTILE_DIRTY)
        {
            int destX = KNOB_MACROTILE_X_DIM * x;
            int destY = KNOB_MACROTILE_Y_DIM * y;

            pContext->pfnStoreTile(GetPrivateState(pDC), srcFormat,
                pDesc->attachment, destX, destY, pHotTile->renderTargetArrayIndex, pHotTile->pBuffer);
        }
        

        if (pHotTile->state == HOTTILE_DIRTY || pHotTile->state == HOTTILE_RESOLVED)
        {
            pHotTile->state = (HOTTILE_STATE)pDesc->postStoreTileState;
        }
    }
    RDTSC_STOP(BEStoreTiles, numTiles, pDC->drawId);
}


void ProcessInvalidateTilesBE(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t macroTile, void *pData)
{
    INVALIDATE_TILES_DESC *pDesc = (INVALIDATE_TILES_DESC*)pData;
    SWR_CONTEXT *pContext = pDC->pContext;

    for (uint32_t i = 0; i < SWR_NUM_ATTACHMENTS; ++i)
    {
        if (pDesc->attachmentMask & (1 << i))
        {
            HOTTILE *pHotTile = pContext->pHotTileMgr->GetHotTile(pContext, pDC, macroTile, (SWR_RENDERTARGET_ATTACHMENT)i, false);
            if (pHotTile)
            {
                pHotTile->state = HOTTILE_INVALID;
            }
        }
    }
}

#if KNOB_SIMD_WIDTH == 8
const __m256 vQuadCenterOffsetsX = { 0.5, 1.5, 0.5, 1.5, 2.5, 3.5, 2.5, 3.5 };
const __m256 vQuadCenterOffsetsY = { 0.5, 0.5, 1.5, 1.5, 0.5, 0.5, 1.5, 1.5 };
const __m256 vQuadULOffsetsX ={0.0, 1.0, 0.0, 1.0, 2.0, 3.0, 2.0, 3.0};
const __m256 vQuadULOffsetsY ={0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0};
#define MASK 0xff
#else
#error Unsupported vector width
#endif

INLINE
bool CanEarlyZ(const SWR_PS_STATE *pPSState)
{
    return (!pPSState->writesODepth && !pPSState->usesSourceDepth);
}

simdmask ComputeUserClipMask(uint8_t clipMask, float* pUserClipBuffer, simdscalar vI, simdscalar vJ)
{
    simdscalar vClipMask = _simd_setzero_ps();
    uint32_t numClipDistance = _mm_popcnt_u32(clipMask);

    for (uint32_t i = 0; i < numClipDistance; ++i)
    {
        // pull triangle clip distance values from clip buffer
        simdscalar vA = _simd_broadcast_ss(pUserClipBuffer++);
        simdscalar vB = _simd_broadcast_ss(pUserClipBuffer++);
        simdscalar vC = _simd_broadcast_ss(pUserClipBuffer++);

        // interpolate
        simdscalar vInterp = vplaneps(vA, vB, vC, vI, vJ);
        
        // clip if interpolated clip distance is < 0 || NAN
        simdscalar vCull = _simd_cmp_ps(_simd_setzero_ps(), vInterp, _CMP_NLE_UQ);

        vClipMask = _simd_or_ps(vClipMask, vCull);
    }

    return _simd_movemask_ps(vClipMask);
}

template<uint32_t MaxRT, SWR_MULTISAMPLE_COUNT sampleCount>
void BackendSampleRate(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t x, uint32_t y, SWR_TRIANGLE_DESC &work, RenderOutputBuffers &renderBuffers)
{
    RDTSC_START(BESetup);

    SWR_CONTEXT *pContext = pDC->pContext;
    const API_STATE& state = GetApiState(pDC);
    const SWR_RASTSTATE& rastState = state.rastState;
    const SWR_PS_STATE *pPSState = &state.psState;
    const SWR_BLEND_STATE *pBlendState = &state.blendState;

    // broadcast scalars
    simdscalar vIa = _simd_broadcast_ss(&work.I[0]);
    simdscalar vIb = _simd_broadcast_ss(&work.I[1]);
    simdscalar vIc = _simd_broadcast_ss(&work.I[2]);

    simdscalar vJa = _simd_broadcast_ss(&work.J[0]);
    simdscalar vJb = _simd_broadcast_ss(&work.J[1]);
    simdscalar vJc = _simd_broadcast_ss(&work.J[2]);

    simdscalar vZa = _simd_broadcast_ss(&work.Z[0]);
    simdscalar vZb = _simd_broadcast_ss(&work.Z[1]);
    simdscalar vZc = _simd_broadcast_ss(&work.Z[2]);

    simdscalar vRecipDet = _simd_broadcast_ss(&work.recipDet);

    simdscalar vAOneOverW = _simd_broadcast_ss(&work.OneOverW[0]);
    simdscalar vBOneOverW = _simd_broadcast_ss(&work.OneOverW[1]);
    simdscalar vCOneOverW = _simd_broadcast_ss(&work.OneOverW[2]);

    uint8_t *pColorBase[SWR_NUM_RENDERTARGETS];
    for(uint32_t rt = 0; rt <= MaxRT; ++rt)
    {
        pColorBase[rt] = renderBuffers.pColor[rt];
    }
    uint8_t *pDepthBase = renderBuffers.pDepth, *pStencilBase = renderBuffers.pStencil;
    RDTSC_STOP(BESetup, 0, 0);

    SWR_PS_CONTEXT psContext;
    psContext.pAttribs = work.pAttribs;
    psContext.pPerspAttribs = work.pPerspAttribs;
    psContext.frontFace = work.triFlags.frontFacing;
    psContext.primID = work.triFlags.primID;

    // save Ia/Ib/Ic and Ja/Jb/Jc if we need to reevaluate i/j/k in the shader because of pull attribs
    psContext.I = work.I;
    psContext.J = work.J;
    psContext.recipDet = work.recipDet;
    psContext.pSamplePos = work.pSamplePos;
    const uint32_t numSamples = MultisampleTraits<sampleCount>::numSamples;

    for (uint32_t yy = y; yy < y + KNOB_TILE_Y_DIM; yy += SIMD_TILE_Y_DIM)
    {
        simdscalar vYSamplePosUL;
        if(sampleCount == SWR_MULTISAMPLE_1X)
        {
            // pixel center
            psContext.vY = _simd_add_ps(vQuadCenterOffsetsY, _simd_set1_ps((float)yy));
        }
        else
        {
            // UL pixel corner
            vYSamplePosUL = _simd_add_ps(vQuadULOffsetsY, _simd_set1_ps((float)yy));
        }

        for (uint32_t xx = x; xx < x + KNOB_TILE_X_DIM; xx += SIMD_TILE_X_DIM)
        {
            simdscalar vXSamplePosUL;
            if(sampleCount > SWR_MULTISAMPLE_1X)
            {
                // UL pixel corner
                vXSamplePosUL = _simd_add_ps(vQuadULOffsetsX, _simd_set1_ps((float)xx));
            }

            // @todo: uint32_t sampleMask = state.rastState.sampleMask & MultisampleTraits<sampleCount>::sampleMask;
            for(uint32_t sample = 0; sample < numSamples; sample++)
            {
                /// @todo: sampleMask / inputcoverage
                if (work.coverageMask[sample] & MASK)
                {
                    RDTSC_START(BEBarycentric);

                    if(sampleCount == SWR_MULTISAMPLE_1X)
                    {
                        // pixel center
                        psContext.vX = _simd_add_ps(vQuadCenterOffsetsX, _simd_set1_ps((float)xx));
                    }
                    else
                    {
                        // calculate per sample positions
                        psContext.vX = _simd_add_ps(vXSamplePosUL, MultisampleTraits<sampleCount>::vX(sample));
                        psContext.vY = _simd_add_ps(vYSamplePosUL, MultisampleTraits<sampleCount>::vY(sample));
                    }

                    // evaluate I,J
                    psContext.vI = vplaneps(vIa, vIb, vIc, psContext.vX, psContext.vY);
                    psContext.vJ = vplaneps(vJa, vJb, vJc, psContext.vX, psContext.vY);
                    psContext.vI = _simd_mul_ps(psContext.vI, vRecipDet);
                    psContext.vJ = _simd_mul_ps(psContext.vJ, vRecipDet);

                    // interpolate z
                    psContext.vZ = vplaneps(vZa, vZb, vZc, psContext.vI, psContext.vJ);
                    RDTSC_STOP(BEBarycentric, 0, 0);

                    simdmask coverageMask = work.coverageMask[sample] & MASK;

                    // interpolate user clip distance if available
                    if (rastState.clipDistanceMask)
                    {
                        coverageMask &= ~ComputeUserClipMask(rastState.clipDistanceMask, work.pUserClipBuffer,
                            psContext.vI, psContext.vJ);
                    }

                    simdscalar depthPassMask = vMask(coverageMask);

                    uint8_t *pDepthSample, *pStencilSample;
                    if(sampleCount == SWR_MULTISAMPLE_1X)
                    {
                        pDepthSample = pDepthBase;
                        pStencilSample = pStencilBase;
                    }
                    else
                    {
                        // offset depth/stencil buffers current sample
                        pDepthSample = pDepthBase + MultisampleTraits<sampleCount>::RasterTileDepthOffset(sample);
                        pStencilSample = pStencilBase + MultisampleTraits<sampleCount>::RasterTileStencilOffset(sample);
                    }

                    // Early-Z?
                    if (CanEarlyZ(pPSState))
                    {
                        RDTSC_START(BEEarlyDepthTest);
                        depthPassMask = ZTest(&state.vp[0], &state.depthStencilState, work.triFlags.frontFacing,
                                              psContext.vZ, pDepthBase, depthPassMask, pStencilBase, pPSState->killsPixel);
                        RDTSC_STOP(BEEarlyDepthTest, 0, 0);

                        if (!_simd_movemask_ps(depthPassMask))
                        {
                            work.coverageMask[sample] >>= (SIMD_TILE_Y_DIM * SIMD_TILE_X_DIM);
                            continue;
                        }
                    }

                    // interpolate 1/w
                    psContext.vOneOverW = vplaneps(vAOneOverW, vBOneOverW, vCOneOverW, psContext.vI, psContext.vJ);
                    psContext.sampleIndex = sample;
                    psContext.mask = _simd_castps_si(depthPassMask);

                    // execute pixel shader
                    RDTSC_START(BEPixelShader);
                    state.psState.pfnPixelShader(GetPrivateState(pDC), &psContext);
                    RDTSC_STOP(BEPixelShader, 0, 0);

                    depthPassMask = _simd_castsi_ps(psContext.mask);

                    //// late-Z
                    if (!CanEarlyZ(pPSState) || pPSState->killsPixel)
                    {
                        RDTSC_START(BELateDepthTest);
                        depthPassMask = ZTest(&state.vp[0], &state.depthStencilState, work.triFlags.frontFacing,
                                              psContext.vZ, pDepthSample, depthPassMask, pStencilSample, false);
                        RDTSC_STOP(BELateDepthTest, 0, 0);

                        if (!_simd_movemask_ps(depthPassMask))
                        {
                            work.coverageMask[sample] >>= (SIMD_TILE_Y_DIM * SIMD_TILE_X_DIM);
                            continue;
                        }
                    }

                    uint32_t statMask = _simd_movemask_ps(depthPassMask);
                    uint32_t statCount = _mm_popcnt_u32(statMask);
                    UPDATE_STAT(DepthPassCount, statCount);

                    simdscalari mask = _simd_castps_si(depthPassMask);

                    // output merger
                    RDTSC_START(BEOutputMerger);

                    if(sampleCount != SWR_MULTISAMPLE_1X)
                    {
                        if(rastState.isSampleMasked[sample])
                        {
                            continue;
                        }
                    }

                    uint32_t rasterTileColorOffset = MultisampleTraits<sampleCount>::RasterTileColorOffset(sample);
                    for (uint32_t rt = 0; rt <= MaxRT; ++rt)
                    {
                        uint8_t *pColorSample;
                        if(sampleCount == SWR_MULTISAMPLE_1X)
                        {
                            pColorSample = pColorBase[rt];
                        }
                        else
                        {
                            pColorSample = pColorBase[rt] + rasterTileColorOffset;
                        }

                        const SWR_RENDER_TARGET_BLEND_STATE *pRTBlend = &pBlendState->renderTarget[rt];

                        // Blend outputs
                        if (pRTBlend->colorBlendEnable)
                        {
                            state.pfnBlendFunc[rt](pBlendState, psContext.shaded[rt], psContext.shaded[1], pColorSample, psContext.shaded[rt]);
                        }

                        ///@todo can only use maskstore fast path if bpc is 32. Assuming hot tile is RGBA32_FLOAT.
                        static_assert(KNOB_COLOR_HOT_TILE_FORMAT == R32G32B32A32_FLOAT, "Unsupported hot tile format");

                        const uint32_t simd = KNOB_SIMD_WIDTH * sizeof(float);

                        // store with color mask
                        if (!pRTBlend->writeDisableRed)
                        {
                            _simd_maskstore_ps((float*)pColorSample, mask, psContext.shaded[rt].x);
                        }
                        if (!pRTBlend->writeDisableGreen)
                        {
                            _simd_maskstore_ps((float*)(pColorSample + simd), mask, psContext.shaded[rt].y);
                        }
                        if (!pRTBlend->writeDisableBlue)
                        {
                            _simd_maskstore_ps((float*)(pColorSample + simd * 2), mask, psContext.shaded[rt].z);
                        }
                        if (!pRTBlend->writeDisableAlpha)
                        {
                            _simd_maskstore_ps((float*)(pColorSample + simd * 3), mask, psContext.shaded[rt].w);
                        }
                    }

                    RDTSC_STOP(BEOutputMerger, 0, 0);
                }
                work.coverageMask[sample] >>= (SIMD_TILE_Y_DIM * SIMD_TILE_X_DIM);
            }
            RDTSC_START(BEEndTile);
            pDepthBase += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_DEPTH_HOT_TILE_FORMAT>::bpp) / 8;
            pStencilBase += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_STENCIL_HOT_TILE_FORMAT>::bpp) / 8;

            for (uint32_t rt = 0; rt <= MaxRT; ++rt)
            {
                pColorBase[rt] += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_COLOR_HOT_TILE_FORMAT>::bpp) / 8;
            }
            RDTSC_STOP(BEEndTile, 0, 0);
        }
    }
}

template<uint32_t MaxRT, SWR_MULTISAMPLE_COUNT sampleCount>
void BackendPixelRate(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t x, uint32_t y, SWR_TRIANGLE_DESC &work, RenderOutputBuffers &renderBuffers)
{
    RDTSC_START(BESetup);

    SWR_CONTEXT *pContext = pDC->pContext;
    const API_STATE& state = GetApiState(pDC);
    const SWR_RASTSTATE& rastState = state.rastState;
    const SWR_PS_STATE *pPSState = &state.psState;
    const SWR_BLEND_STATE *pBlendState = &state.blendState;

    // broadcast scalars
    simdscalar vIa = _simd_broadcast_ss(&work.I[0]);
    simdscalar vIb = _simd_broadcast_ss(&work.I[1]);
    simdscalar vIc = _simd_broadcast_ss(&work.I[2]);

    simdscalar vJa = _simd_broadcast_ss(&work.J[0]);
    simdscalar vJb = _simd_broadcast_ss(&work.J[1]);
    simdscalar vJc = _simd_broadcast_ss(&work.J[2]);

    simdscalar vZa = _simd_broadcast_ss(&work.Z[0]);
    simdscalar vZb = _simd_broadcast_ss(&work.Z[1]);
    simdscalar vZc = _simd_broadcast_ss(&work.Z[2]);

    simdscalar vRecipDet = _simd_broadcast_ss(&work.recipDet);

    simdscalar vAOneOverW = _simd_broadcast_ss(&work.OneOverW[0]);
    simdscalar vBOneOverW = _simd_broadcast_ss(&work.OneOverW[1]);
    simdscalar vCOneOverW = _simd_broadcast_ss(&work.OneOverW[2]);

    uint8_t *pColorBase[SWR_NUM_RENDERTARGETS];
    for(uint32_t rt = 0; rt <= MaxRT; ++rt)
    {
        pColorBase[rt] = renderBuffers.pColor[rt];
    }
    uint8_t *pDepthBase = renderBuffers.pDepth, *pStencilBase = renderBuffers.pStencil;
    RDTSC_STOP(BESetup, 0, 0);

    SWR_PS_CONTEXT psContext;
    psContext.pAttribs = work.pAttribs;
    psContext.pPerspAttribs = work.pPerspAttribs;
    psContext.frontFace = work.triFlags.frontFacing;
    psContext.primID = work.triFlags.primID;

    // save Ia/Ib/Ic and Ja/Jb/Jc if we need to reevaluate i/j/k in the shader because of pull attribs
    psContext.I = work.I;
    psContext.J = work.J;
    psContext.recipDet = work.recipDet;
    psContext.pSamplePos = work.pSamplePos;
    psContext.sampleIndex = 0;

    const uint32_t numSamples = MultisampleTraits<sampleCount>::numSamples;
    for(uint32_t yy = y; yy < y + KNOB_TILE_Y_DIM; yy += SIMD_TILE_Y_DIM)
    {
        simdscalar vYSamplePosUL = _simd_add_ps(vQuadULOffsetsY, _simd_set1_ps((float)yy));
        simdscalar vYSamplePosCenter = _simd_add_ps(vQuadCenterOffsetsY, _simd_set1_ps((float)yy));
        for(uint32_t xx = x; xx < x + KNOB_TILE_X_DIM; xx += SIMD_TILE_X_DIM)
        {
            simdscalar vXSamplePosUL = _simd_add_ps(vQuadULOffsetsX, _simd_set1_ps((float)xx));
            simdscalar vXSamplePosCenter = _simd_add_ps(vQuadCenterOffsetsX, _simd_set1_ps((float)xx));

            // if oDepth written to, or there is a potential to discard any samples, we need to 
            // run the PS early, then interp or broadcast Z and test
            if(pPSState->writesODepth || pPSState->killsPixel)
            {
                RDTSC_START(BEBarycentric);
                // set pixel center positions
                psContext.vX = vXSamplePosCenter;
                psContext.vY = vYSamplePosCenter;

                // evaluate I, J at pixel center
                psContext.vI = vplaneps(vIa, vIb, vIc, psContext.vX, psContext.vY);
                psContext.vJ = vplaneps(vJa, vJb, vJc, psContext.vX, psContext.vY);
                psContext.vI = _simd_mul_ps(psContext.vI, vRecipDet);
                psContext.vJ = _simd_mul_ps(psContext.vJ, vRecipDet);

                // interpolate z
                psContext.vZ = vplaneps(vZa, vZb, vZc, psContext.vI, psContext.vJ);

                RDTSC_STOP(BEBarycentric, 0, 0);

                // interpolate 1/w
                psContext.vOneOverW = vplaneps(vAOneOverW, vBOneOverW, vCOneOverW, psContext.vI, psContext.vJ);

                /// @todo: sampleMask / inputcoverage
                // for now just pass in all 1s
                psContext.mask = _simd_set1_epi32(-1);

                // execute pixel shader
                RDTSC_START(BEPixelShader);
                state.psState.pfnPixelShader(GetPrivateState(pDC), &psContext);
                RDTSC_STOP(BEPixelShader, 0, 0);
            }
            else
            {
                /// @todo: sampleMask / inputcoverage
                // for now just through full pixel output
                psContext.mask = _simd_set1_epi32(-1);
            }

            simdscalar depthPassMask[numSamples];
            simdscalar anyDepthSamplePassed = _simd_setzero_ps();
            for(uint32_t sample = 0; sample < numSamples; sample++)
            {
                /// @todo: sampleMask / inputcoverage
                depthPassMask[sample] = vMask(work.coverageMask[sample] & MASK);
                // pull mask back out for any discards and and with coverage
                depthPassMask[sample] = _simd_and_ps(depthPassMask[sample], _simd_castsi_ps(psContext.mask));

                if (!_simd_movemask_ps(depthPassMask[sample]))
                {
                    depthPassMask[sample] = _simd_setzero_ps();
                    continue;
                }

                // if oDepth isn't written to, we need to interpolate Z for each sample
                // if clip distances are enabled, we need to interpolate for each sample
                if(!pPSState->writesODepth || rastState.clipDistanceMask)
                {
                    RDTSC_START(BEBarycentric);
                    // calculate per sample positions
                    simdscalar vSamplePosX = _simd_add_ps(vXSamplePosUL, MultisampleTraits<sampleCount>::vX(sample));
                    simdscalar vSamplePosY = _simd_add_ps(vYSamplePosUL, MultisampleTraits<sampleCount>::vY(sample));

                    // evaluate I,J at sample positions
                    psContext.vI = vplaneps(vIa, vIb, vIc, vSamplePosX, vSamplePosY);
                    psContext.vJ = vplaneps(vJa, vJb, vJc, vSamplePosX, vSamplePosY);
                    psContext.vI = _simd_mul_ps(psContext.vI, vRecipDet);
                    psContext.vJ = _simd_mul_ps(psContext.vJ, vRecipDet);

                    // interpolate z
                    if (!pPSState->writesODepth)
                    {
                        psContext.vZ = vplaneps(vZa, vZb, vZc, psContext.vI, psContext.vJ);
                    }
                    
                    // interpolate clip distances
                    if (rastState.clipDistanceMask)
                    {
                        uint8_t clipMask = ComputeUserClipMask(rastState.clipDistanceMask, work.pUserClipBuffer,
                            psContext.vI, psContext.vJ);
                        depthPassMask[sample] = _simd_and_ps(depthPassMask[sample], vMask(~clipMask));
                    }
                    RDTSC_STOP(BEBarycentric, 0, 0);
                }
                // else 'broadcast' and test psContext.vZ from the PS invocation for each sample

                // offset depth/stencil buffers current sample
                uint8_t *pDepthSample = pDepthBase + MultisampleTraits<sampleCount>::RasterTileDepthOffset(sample);
                uint8_t * pStencilSample = pStencilBase + MultisampleTraits<sampleCount>::RasterTileStencilOffset(sample);

                // ZTest for this sample
                RDTSC_START(BEEarlyDepthTest);
                depthPassMask[sample] = ZTest(&state.vp[0], &state.depthStencilState, work.triFlags.frontFacing,
                                        psContext.vZ, pDepthSample, depthPassMask[sample], pStencilSample, false);
                RDTSC_STOP(BEEarlyDepthTest, 0, 0);

                anyDepthSamplePassed = _simd_or_ps(anyDepthSamplePassed, depthPassMask[sample]);

                uint32_t statMask = _simd_movemask_ps(depthPassMask[sample]);
                uint32_t statCount = _mm_popcnt_u32(statMask);
                UPDATE_STAT(DepthPassCount, statCount);
            }

            // if we didn't have to execute the PS early, and at least 1 sample passed the depth test, run the PS
            if(!pPSState->writesODepth && !pPSState->killsPixel && _simd_movemask_ps(anyDepthSamplePassed))
            {
                RDTSC_START(BEBarycentric);
                // set pixel center positions
                psContext.vX = vXSamplePosCenter;
                psContext.vY = vYSamplePosCenter;

                // evaluate I,J at pixel center
                psContext.vI = vplaneps(vIa, vIb, vIc, psContext.vX, psContext.vY);
                psContext.vJ = vplaneps(vJa, vJb, vJc, psContext.vX, psContext.vY);
                psContext.vI = _simd_mul_ps(psContext.vI, vRecipDet);
                psContext.vJ = _simd_mul_ps(psContext.vJ, vRecipDet);

                // interpolate z
                psContext.vZ = vplaneps(vZa, vZb, vZc, psContext.vI, psContext.vJ);
                RDTSC_STOP(BEBarycentric, 0, 0);

                // interpolate 1/w
                psContext.vOneOverW = vplaneps(vAOneOverW, vBOneOverW, vCOneOverW, psContext.vI, psContext.vJ);

                // execute pixel shader
                RDTSC_START(BEPixelShader);
                state.psState.pfnPixelShader(GetPrivateState(pDC), &psContext);
                RDTSC_STOP(BEPixelShader, 0, 0);
            }
            else
            {
                goto Endtile;
            }

            // loop over all samples, broadcasting the results of the PS to all passing pixels
            for(uint32_t sample = 0; sample < numSamples; sample++)
            {
                if(sampleCount != SWR_MULTISAMPLE_1X)
                {
                    if(rastState.isSampleMasked[sample])
                        continue;
                }

                // output merger
                RDTSC_START(BEOutputMerger);
                // skip if none of the pixels for this sample passed
                if(!_simd_movemask_ps(depthPassMask[sample]))
                {
                    depthPassMask[sample] = _simd_setzero_ps();
                    continue;
                }
                simdscalari mask = _simd_castps_si(depthPassMask[sample]);
                uint32_t rasterTileColorOffset = MultisampleTraits<sampleCount>::RasterTileColorOffset(sample);
                for(uint32_t rt = 0; rt <= MaxRT; ++rt)
                {
                    uint8_t *pColorSample = pColorBase[rt] + rasterTileColorOffset;

                    const SWR_RENDER_TARGET_BLEND_STATE *pRTBlend = &pBlendState->renderTarget[rt];

                    // Blend outputs
                    if(pRTBlend->colorBlendEnable)
                    {
                        state.pfnBlendFunc[rt](pBlendState, psContext.shaded[rt], psContext.shaded[1], pColorSample, psContext.shaded[rt]);
                    }

                    ///@todo can only use maskstore fast path if bpc is 32. Assuming hot tile is RGBA32_FLOAT.
                    static_assert(KNOB_COLOR_HOT_TILE_FORMAT == R32G32B32A32_FLOAT, "Unsupported hot tile format");

                    const uint32_t simd = KNOB_SIMD_WIDTH * sizeof(float);

                    // store with color mask
                    if(!pRTBlend->writeDisableRed)
                    {
                        _simd_maskstore_ps((float*)pColorSample, mask, psContext.shaded[rt].x);
                    }
                    if(!pRTBlend->writeDisableGreen)
                    {
                        _simd_maskstore_ps((float*)(pColorSample + simd), mask, psContext.shaded[rt].y);
                    }
                    if(!pRTBlend->writeDisableBlue)
                    {
                        _simd_maskstore_ps((float*)(pColorSample + simd * 2), mask, psContext.shaded[rt].z);
                    }
                    if(!pRTBlend->writeDisableAlpha)
                    {
                        _simd_maskstore_ps((float*)(pColorSample + simd * 3), mask, psContext.shaded[rt].w);
                    }
                }
                RDTSC_STOP(BEOutputMerger, 0, 0);
            }

Endtile:
            RDTSC_START(BEEndTile);
            for(uint32_t sample = 0; sample < numSamples; sample++)
            {
                work.coverageMask[sample] >>= (SIMD_TILE_Y_DIM * SIMD_TILE_X_DIM);
            }

            pDepthBase += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_DEPTH_HOT_TILE_FORMAT>::bpp) / 8;
            pStencilBase += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_STENCIL_HOT_TILE_FORMAT>::bpp) / 8;

            for(uint32_t rt = 0; rt <= MaxRT; ++rt)
            {
                pColorBase[rt] += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_COLOR_HOT_TILE_FORMAT>::bpp) / 8;
            }
            RDTSC_STOP(BEEndTile, 0, 0);
        }
    }
}
// optimized backend flow with NULL PS
void BackendNullPS(DRAW_CONTEXT *pDC, uint32_t workerId, uint32_t x, uint32_t y, SWR_TRIANGLE_DESC &work, RenderOutputBuffers &renderBuffers)
{
    RDTSC_START(BESetup);

    SWR_CONTEXT *pContext = pDC->pContext;
    const API_STATE& state = GetApiState(pDC);
    // todo multisample
    uint64_t coverageMask = work.coverageMask[0];

    // broadcast scalars
    simdscalar vIa = _simd_broadcast_ss(&work.I[0]);
    simdscalar vIb = _simd_broadcast_ss(&work.I[1]);
    simdscalar vIc = _simd_broadcast_ss(&work.I[2]);

    simdscalar vJa = _simd_broadcast_ss(&work.J[0]);
    simdscalar vJb = _simd_broadcast_ss(&work.J[1]);
    simdscalar vJc = _simd_broadcast_ss(&work.J[2]);

    simdscalar vZa = _simd_broadcast_ss(&work.Z[0]);
    simdscalar vZb = _simd_broadcast_ss(&work.Z[1]);
    simdscalar vZc = _simd_broadcast_ss(&work.Z[2]);

    simdscalar vRecipDet = _simd_broadcast_ss(&work.recipDet);

    BYTE *pDepthBase = renderBuffers.pDepth, *pStencilBase = renderBuffers.pStencil;

    RDTSC_STOP(BESetup, 0, 0);

    SWR_PS_CONTEXT psContext;
    for (uint32_t yy = y; yy < y + KNOB_TILE_Y_DIM; yy += SIMD_TILE_Y_DIM)
    {
        psContext.vY = _simd_add_ps(vQuadCenterOffsetsY, _simd_set1_ps((float)yy));
        for (uint32_t xx = x; xx < x + KNOB_TILE_X_DIM; xx += SIMD_TILE_X_DIM)
        {
            if (coverageMask & MASK)
            {
                RDTSC_START(BEBarycentric);

                // calculate pixel positions
                psContext.vX = _simd_add_ps(vQuadCenterOffsetsX, _simd_set1_ps((float)xx));

                // evaluate I,J
                psContext.vI = vplaneps(vIa, vIb, vIc, psContext.vX, psContext.vY);
                psContext.vJ = vplaneps(vJa, vJb, vJc, psContext.vX, psContext.vY);
                psContext.vI = _simd_mul_ps(psContext.vI, vRecipDet);
                psContext.vJ = _simd_mul_ps(psContext.vJ, vRecipDet);

                // interpolate z
                psContext.vZ = vplaneps(vZa, vZb, vZc, psContext.vI, psContext.vJ);

                RDTSC_STOP(BEBarycentric, 0, 0);

                simdscalar depthPassMask = vMask(coverageMask & MASK);
                RDTSC_START(BEEarlyDepthTest);
                depthPassMask = ZTest(&state.vp[0], &state.depthStencilState, work.triFlags.frontFacing,
                                      psContext.vZ, pDepthBase, depthPassMask, pStencilBase, false);
                RDTSC_STOP(BEEarlyDepthTest, 0, 0);

                uint32_t statMask = _simd_movemask_ps(depthPassMask);
                uint32_t statCount = _mm_popcnt_u32(statMask);
                UPDATE_STAT(DepthPassCount, statCount);
            }
            coverageMask >>= (SIMD_TILE_Y_DIM * SIMD_TILE_X_DIM);
            pDepthBase += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_DEPTH_HOT_TILE_FORMAT>::bpp) / 8;
            pStencilBase += (KNOB_SIMD_WIDTH * FormatTraits<KNOB_STENCIL_HOT_TILE_FORMAT>::bpp) / 8;
        }
    }
}

void InitClearTilesTable()
{
    memset(sClearTilesTable, 0, sizeof(sClearTilesTable));

    sClearTilesTable[R8G8B8A8_UNORM] = ClearMacroTile<R8G8B8A8_UNORM>;
    sClearTilesTable[B8G8R8A8_UNORM] = ClearMacroTile<B8G8R8A8_UNORM>;
    sClearTilesTable[R32_FLOAT] = ClearMacroTile<R32_FLOAT>;
    sClearTilesTable[R32G32B32A32_FLOAT] = ClearMacroTile<R32G32B32A32_FLOAT>;
    sClearTilesTable[R8_UINT] = ClearMacroTile<R8_UINT>;
}

// initialize backend function tables
PFN_BACKEND_FUNC gSingleSampleBackendTable[] = {
    BackendSampleRate<0, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<1, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<2, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<3, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<4, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<5, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<6, SWR_MULTISAMPLE_1X>,
    BackendSampleRate<7, SWR_MULTISAMPLE_1X>,
};

// MSAA per sample shading rate
PFN_BACKEND_FUNC gSampleRateBackendTable[SWR_MULTISAMPLE_TYPE_MAX-1][SWR_NUM_RENDERTARGETS] ={
    {
        BackendSampleRate<0, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<1, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<2, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<3, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<4, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<5, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<6, SWR_MULTISAMPLE_2X>,
        BackendSampleRate<7, SWR_MULTISAMPLE_2X>,
    },
    {
        BackendSampleRate<0, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<1, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<2, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<3, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<4, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<5, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<6, SWR_MULTISAMPLE_4X>,
        BackendSampleRate<7, SWR_MULTISAMPLE_4X>,
    },
    {
        BackendSampleRate<0, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<1, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<2, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<3, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<4, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<5, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<6, SWR_MULTISAMPLE_8X>,
        BackendSampleRate<7, SWR_MULTISAMPLE_8X>,
    },
    {
        BackendSampleRate<0, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<1, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<2, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<3, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<4, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<5, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<6, SWR_MULTISAMPLE_16X>,
        BackendSampleRate<7, SWR_MULTISAMPLE_16X>,
    }
};

// MSAA per pixel shading rate
PFN_BACKEND_FUNC gPixelRateBackendTable[SWR_MULTISAMPLE_TYPE_MAX-1][SWR_NUM_RENDERTARGETS] ={
    {
        BackendPixelRate<0, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<1, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<2, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<3, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<4, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<5, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<6, SWR_MULTISAMPLE_2X>,
        BackendPixelRate<7, SWR_MULTISAMPLE_2X>,
    },
    {
        BackendPixelRate<0, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<1, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<2, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<3, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<4, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<5, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<6, SWR_MULTISAMPLE_4X>,
        BackendPixelRate<7, SWR_MULTISAMPLE_4X>,
    },
    {
        BackendPixelRate<0, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<1, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<2, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<3, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<4, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<5, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<6, SWR_MULTISAMPLE_8X>,
        BackendPixelRate<7, SWR_MULTISAMPLE_8X>,
    },
    {
        BackendPixelRate<0, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<1, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<2, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<3, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<4, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<5, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<6, SWR_MULTISAMPLE_16X>,
        BackendPixelRate<7, SWR_MULTISAMPLE_16X>,
    }
};
