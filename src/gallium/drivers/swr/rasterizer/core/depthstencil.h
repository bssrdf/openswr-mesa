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
* @file depthstencil.h
*
* @brief Implements depth/stencil functionality
*
******************************************************************************/
#pragma once
#include "common/os.h"
#include "format_conversion.h"

INLINE
void StencilOp(SWR_STENCILOP op, simdscalar mask, simdscalar stencilRefps, simdscalar &stencilps)
{
    simdscalari stencil = _simd_castps_si(stencilps);

    switch (op)
    {
    case STENCILOP_KEEP:
        break;
    case STENCILOP_ZERO:
        stencilps = _simd_blendv_ps(stencilps, _simd_setzero_ps(), mask);
        break;
    case STENCILOP_REPLACE:
        stencilps = _simd_blendv_ps(stencilps, stencilRefps, mask);
        break;
    case STENCILOP_INCRSAT:
    {
        simdscalari stencilincr = _simd_adds_epu8(stencil, _simd_set1_epi32(1));
        stencilps = _simd_blendv_ps(stencilps, _simd_castsi_ps(stencilincr), mask);
        break;
    }
    case STENCILOP_DECRSAT:
    {
        simdscalari stencildecr = _simd_subs_epu8(stencil, _simd_set1_epi32(1));
        stencilps = _simd_blendv_ps(stencilps, _simd_castsi_ps(stencildecr), mask);
        break;
    }
    case STENCILOP_INCR:
    {
        simdscalari stencilincr = _simd_add_epi8(stencil, _simd_set1_epi32(1));
        stencilps = _simd_blendv_ps(stencilps, _simd_castsi_ps(stencilincr), mask);
        break;
    }
    case STENCILOP_DECR:
    {
        simdscalari stencildecr = _simd_add_epi8(stencil, _simd_set1_epi32((-1) & 0xff));
        stencilps = _simd_blendv_ps(stencilps, _simd_castsi_ps(stencildecr), mask);
        break;
    }
    case STENCILOP_INVERT:
    {
        simdscalar stencilinvert = _simd_andnot_ps(stencilps, _simd_cmpeq_ps(_simd_setzero_ps(), _simd_setzero_ps()));
        stencilps = _simd_blendv_ps(stencilps, stencilinvert, mask);
        break;
    }
    default:
        break;
    }
}


INLINE
simdscalar ZTest(const SWR_VIEWPORT* pViewport, const SWR_DEPTH_STENCIL_STATE* pDSState,
                 bool frontFacing, simdscalar interpZ, BYTE* pDepthBase, simdscalar mask, BYTE *pStencilBase,
                 bool testOnly)
{
    static_assert(KNOB_DEPTH_HOT_TILE_FORMAT == R32_FLOAT, "Unsupported depth hot tile format");
    static_assert(KNOB_STENCIL_HOT_TILE_FORMAT == R8_UINT, "Unsupported stencil hot tile format");

    simdscalar depthResult = _simd_set1_ps(-1.0f);
    simdscalar zbuf;

    // clamp Z to viewport [minZ..maxZ]
    simdscalar vMinZ = _simd_broadcast_ss(&pViewport->minZ);
    simdscalar vMaxZ = _simd_broadcast_ss(&pViewport->maxZ);
    interpZ = _simd_min_ps(vMaxZ, _simd_max_ps(vMinZ, interpZ));
    
    if (pDSState->depthTestEnable)
    {
        switch (pDSState->depthTestFunc)
        {
        case ZFUNC_NEVER: depthResult = _simd_setzero_ps(); break;
        case ZFUNC_ALWAYS: break;
        default:
            zbuf = _simd_load_ps((const float*)pDepthBase);
        }

        switch (pDSState->depthTestFunc)
        {
        case ZFUNC_LE: depthResult = _simd_cmple_ps(interpZ, zbuf); break;
        case ZFUNC_LT: depthResult = _simd_cmplt_ps(interpZ, zbuf); break;
        case ZFUNC_GT: depthResult = _simd_cmpgt_ps(interpZ, zbuf); break;
        case ZFUNC_GE: depthResult = _simd_cmpge_ps(interpZ, zbuf); break;
        case ZFUNC_EQ: depthResult = _simd_cmpeq_ps(interpZ, zbuf); break;
        }
    }

    simdscalar stencilMask = _simd_set1_ps(-1.0f);
    simdscalar stencilbuf;

    uint8_t stencilRefValue;
    uint32_t stencilTestFunc;
    uint32_t stencilFailOp;
    uint32_t stencilPassDepthPassOp;
    uint32_t stencilPassDepthFailOp;
    uint8_t stencilTestMask;
    uint8_t stencilWriteMask;
    if (frontFacing || !pDSState->doubleSidedStencilTestEnable)
    {
        stencilRefValue = pDSState->stencilRefValue;
        stencilTestFunc = pDSState->stencilTestFunc;
        stencilFailOp = pDSState->stencilFailOp;
        stencilPassDepthPassOp = pDSState->stencilPassDepthPassOp;
        stencilPassDepthFailOp = pDSState->stencilPassDepthFailOp;
        stencilTestMask = pDSState->stencilTestMask;
        stencilWriteMask = pDSState->stencilWriteMask;
    }
    else
    {
        stencilRefValue = pDSState->backfaceStencilRefValue;
        stencilTestFunc = pDSState->backfaceStencilTestFunc;
        stencilFailOp = pDSState->backfaceStencilFailOp;
        stencilPassDepthPassOp = pDSState->backfaceStencilPassDepthPassOp;
        stencilPassDepthFailOp = pDSState->backfaceStencilPassDepthFailOp;
        stencilTestMask = pDSState->backfaceStencilTestMask;
        stencilWriteMask = pDSState->backfaceStencilWriteMask;
    }

    if (pDSState->stencilTestEnable)
    {
        simdvector sbuf;
        LoadSOA<R8_UINT>(pStencilBase, sbuf);
        stencilbuf = sbuf.v[0];

        // apply stencil read mask
        simdscalar stencilWithMask = _simd_castsi_ps(_simd_and_si(_simd_castps_si(sbuf.v[0]), _simd_set1_epi32(stencilTestMask)));

        // do stencil compare in float to avoid simd integer emulation in AVX1
        stencilWithMask = _simd_cvtepi32_ps(_simd_castps_si(stencilWithMask));

        simdscalar stencilRef = _simd_set1_ps((float)(stencilRefValue & stencilTestMask));

        switch (stencilTestFunc)
        {
        case ZFUNC_ALWAYS: break;
        case ZFUNC_NEVER: stencilMask = _simd_setzero_ps(); break;
        case ZFUNC_LE: stencilMask = _simd_cmple_ps(stencilRef, stencilWithMask); break;
        case ZFUNC_LT: stencilMask = _simd_cmplt_ps(stencilRef, stencilWithMask); break;
        case ZFUNC_GT: stencilMask = _simd_cmpgt_ps(stencilRef, stencilWithMask); break;
        case ZFUNC_GE: stencilMask = _simd_cmpge_ps(stencilRef, stencilWithMask); break;
        case ZFUNC_EQ: stencilMask = _simd_cmpeq_ps(stencilRef, stencilWithMask); break;
        case ZFUNC_NE: stencilMask = _simd_cmpneq_ps(stencilRef, stencilWithMask); break;
        }
    }

    simdscalar depthWriteMask = _simd_and_ps(depthResult, stencilMask);
    depthWriteMask = _simd_and_ps(depthWriteMask, mask);

    if (testOnly) {
        return depthWriteMask;
    }

    if (pDSState->depthWriteEnable)
    {
        _simd_maskstore_ps((float*)pDepthBase, _simd_castps_si(depthWriteMask), interpZ);
    }

    if (pDSState->stencilWriteEnable)
    {
        simdscalar stencilps = stencilbuf;
        simdscalar stencilRefps = _simd_castsi_ps(_simd_set1_epi32(stencilRefValue));

        simdscalar stencilFailMask = _simd_andnot_ps(stencilMask, mask);
        simdscalar stencilPassDepthPassMask = _simd_and_ps(stencilMask, depthResult);
        simdscalar stencilPassDepthFailMask = _simd_and_ps(stencilMask, _simd_andnot_ps(depthResult, _simd_set1_ps(-1)));

        simdscalar origStencil = stencilps;

        StencilOp((SWR_STENCILOP)stencilFailOp, stencilFailMask, stencilRefps, stencilps);
        StencilOp((SWR_STENCILOP)stencilPassDepthFailOp, stencilPassDepthFailMask, stencilRefps, stencilps);
        StencilOp((SWR_STENCILOP)stencilPassDepthPassOp, stencilPassDepthPassMask, stencilRefps, stencilps);

        // apply stencil write mask
        simdscalari vWriteMask = _simd_set1_epi32(stencilWriteMask);
        stencilps = _simd_and_ps(stencilps, _simd_castsi_ps(vWriteMask));
        stencilps = _simd_or_ps(_simd_andnot_ps(_simd_castsi_ps(vWriteMask), origStencil), stencilps);

        simdvector stencilResult;
        stencilResult.v[0] = _simd_blendv_ps(origStencil, stencilps, mask);
        StoreSOA<R8_UINT>(stencilResult, pStencilBase);
    }

    return depthWriteMask;
}
