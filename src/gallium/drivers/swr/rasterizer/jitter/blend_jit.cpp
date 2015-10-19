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
* @file blend_jit.cpp
*
* @brief Implementation of the blend jitter
*
* Notes:
*
******************************************************************************/
#include "jit_api.h"
#include "blend_jit.h"
#include "builder.h"
#include "state_llvm.h"
#include "common/containers.hpp"
#include "llvm/IR/DataLayout.h"

#include <sstream>

// components with bit-widths <= the QUANTIZE_THRESHOLD will be quantized
#define QUANTIZE_THRESHOLD 2

//////////////////////////////////////////////////////////////////////////
/// Interface to Jitting a blend shader
//////////////////////////////////////////////////////////////////////////
struct BlendJit : public Builder
{
    BlendJit(JitManager* pJitMgr) : Builder(pJitMgr){};

    template<bool Color, bool Alpha>
    void GenerateBlendFactor(SWR_BLEND_FACTOR factor, Value* constColor[4], Value* src[4], Value* src1[4], Value* dst[4], Value* result[4])
    {
        Value* out[4];

        switch (factor)
        {
        case BLENDFACTOR_ONE:
            out[0] = out[1] = out[2] = out[3] = VIMMED1(1.0f);
            break;
        case BLENDFACTOR_SRC_COLOR:
            out[0] = src[0];
            out[1] = src[1];
            out[2] = src[2];
            out[3] = src[3];
            break;
        case BLENDFACTOR_SRC_ALPHA:
            out[0] = out[1] = out[2] = out[3] = src[3];
            break;
        case BLENDFACTOR_DST_ALPHA:
            out[0] = out[1] = out[2] = out[3] = dst[3];
            break;
        case BLENDFACTOR_DST_COLOR:
            out[0] = dst[0];
            out[1] = dst[1];
            out[2] = dst[2];
            out[3] = dst[3];
            break;
        case BLENDFACTOR_SRC_ALPHA_SATURATE:
            out[0] = out[1] = out[2] = VMINPS(src[3], FSUB(VIMMED1(1.0f), dst[3]));
            out[3] = VIMMED1(1.0f);
            break;
        case BLENDFACTOR_CONST_COLOR:
            out[0] = constColor[0];
            out[1] = constColor[1];
            out[2] = constColor[2];
            out[3] = constColor[3];
            break;
        case BLENDFACTOR_CONST_ALPHA:
            out[0] = out[1] = out[2] = out[3] = constColor[3];
            break;
        case BLENDFACTOR_SRC1_COLOR:
            out[0] = src1[0];
            out[1] = src1[1];
            out[2] = src1[2];
            out[3] = src1[3];
            break;
        case BLENDFACTOR_SRC1_ALPHA:
            out[0] = out[1] = out[2] = out[3] = src1[3];
            break;
        case BLENDFACTOR_ZERO:
            out[0] = out[1] = out[2] = out[3] = VIMMED1(0.0f);
            break;
        case BLENDFACTOR_INV_SRC_COLOR:
            out[0] = FSUB(VIMMED1(1.0f), src[0]);
            out[1] = FSUB(VIMMED1(1.0f), src[1]);
            out[2] = FSUB(VIMMED1(1.0f), src[2]);
            out[3] = FSUB(VIMMED1(1.0f), src[3]);
            break;
        case BLENDFACTOR_INV_SRC_ALPHA:
            out[0] = out[1] = out[2] = out[3] = FSUB(VIMMED1(1.0f), src[3]);
            break;
        case BLENDFACTOR_INV_DST_ALPHA:
            out[0] = out[1] = out[2] = out[3] = FSUB(VIMMED1(1.0f), dst[3]);
            break;
        case BLENDFACTOR_INV_DST_COLOR:
            out[0] = FSUB(VIMMED1(1.0f), dst[0]);
            out[1] = FSUB(VIMMED1(1.0f), dst[1]);
            out[2] = FSUB(VIMMED1(1.0f), dst[2]);
            out[3] = FSUB(VIMMED1(1.0f), dst[3]);
            break;
        case BLENDFACTOR_INV_CONST_COLOR:
            out[0] = FSUB(VIMMED1(1.0f), constColor[0]);
            out[1] = FSUB(VIMMED1(1.0f), constColor[1]);
            out[2] = FSUB(VIMMED1(1.0f), constColor[2]);
            out[3] = FSUB(VIMMED1(1.0f), constColor[3]);
            break;
        case BLENDFACTOR_INV_CONST_ALPHA:
            out[0] = out[1] = out[2] = out[3] = FSUB(VIMMED1(1.0f), constColor[3]);
            break;
        case BLENDFACTOR_INV_SRC1_COLOR:
            out[0] = FSUB(VIMMED1(1.0f), src1[0]);
            out[1] = FSUB(VIMMED1(1.0f), src1[1]);
            out[2] = FSUB(VIMMED1(1.0f), src1[2]);
            out[3] = FSUB(VIMMED1(1.0f), src1[3]);
            break;
        case BLENDFACTOR_INV_SRC1_ALPHA:
            out[0] = out[1] = out[2] = out[3] = FSUB(VIMMED1(1.0f), src1[3]);
            break;
        default:
            SWR_ASSERT(false, "Unsupported blend factor: %d", factor);
            out[0] = out[1] = out[2] = out[3] = VIMMED1(0.0f);
            break;
        }

        if (Color)
        {
            result[0] = out[0];
            result[1] = out[1];
            result[2] = out[2];
        }

        if (Alpha)
        {
            result[3] = out[3];
        }
    }

    void Clamp(SWR_FORMAT format, Value* src[4])
    {
        const SWR_FORMAT_INFO& info = GetFormatInfo(format);
        SWR_TYPE type = info.type[0];

        switch (type)
        {
        case SWR_TYPE_FLOAT:
            break;

        case SWR_TYPE_UNORM:
            src[0] = VMINPS(VMAXPS(src[0], VIMMED1(0.0f)), VIMMED1(1.0f));
            src[1] = VMINPS(VMAXPS(src[1], VIMMED1(0.0f)), VIMMED1(1.0f));
            src[2] = VMINPS(VMAXPS(src[2], VIMMED1(0.0f)), VIMMED1(1.0f));
            src[3] = VMINPS(VMAXPS(src[3], VIMMED1(0.0f)), VIMMED1(1.0f));
            break;

        case SWR_TYPE_SNORM:
            src[0] = VMINPS(VMAXPS(src[0], VIMMED1(-1.0f)), VIMMED1(1.0f));
            src[1] = VMINPS(VMAXPS(src[1], VIMMED1(-1.0f)), VIMMED1(1.0f));
            src[2] = VMINPS(VMAXPS(src[2], VIMMED1(-1.0f)), VIMMED1(1.0f));
            src[3] = VMINPS(VMAXPS(src[3], VIMMED1(-1.0f)), VIMMED1(1.0f));
            break;

        default: SWR_ASSERT(false, "Unsupport format type: %d", type);
        }
    }

    void ApplyDefaults(SWR_FORMAT format, Value* src[4])
    {
        const SWR_FORMAT_INFO& info = GetFormatInfo(format);

        bool valid[] = { false, false, false, false };
        for (uint32_t c = 0; c < info.numComps; ++c)
        {
            valid[info.swizzle[c]] = true;
        }

        for (uint32_t c = 0; c < 4; ++c)
        {
            if (!valid[c])
            {
                src[c] = BITCAST(VIMMED1((int)info.defaults[c]), mSimdFP32Ty);
            }
        }
    }

    void ApplyUnusedDefaults(SWR_FORMAT format, Value* src[4])
    {
        const SWR_FORMAT_INFO& info = GetFormatInfo(format);

        for (uint32_t c = 0; c < info.numComps; ++c)
        {
            if (info.type[c] == SWR_TYPE_UNUSED)
            {
                src[info.swizzle[c]] = BITCAST(VIMMED1((int)info.defaults[info.swizzle[c]]), mSimdFP32Ty);
            }
        }
    }

    void Quantize(SWR_FORMAT format, Value* src[4])
    {
        const SWR_FORMAT_INFO& info = GetFormatInfo(format);
        for (uint32_t c = 0; c < info.numComps; ++c)
        {
            if (info.bpc[c] <= QUANTIZE_THRESHOLD)
            {
                uint32_t swizComp = info.swizzle[c];
                float factor = (float)((1 << info.bpc[c]) - 1);
                switch (info.type[c])
                {
                case SWR_TYPE_UNORM:
                    src[swizComp] = FADD(FMUL(src[swizComp], VIMMED1(factor)), VIMMED1(0.5f));
                    src[swizComp] = VROUND(src[swizComp], C(_MM_FROUND_TO_ZERO));
                    src[swizComp] = FMUL(src[swizComp], VIMMED1(1.0f /factor));
                    break;
                default: SWR_ASSERT(false, "Unsupported format type: %d", info.type[c]);
                }
            }
        }
    }

    template<bool Color, bool Alpha>
    void BlendFunc(SWR_BLEND_OP blendOp, Value* src[4], Value* srcFactor[4], Value* dst[4], Value* dstFactor[4], Value* result[4])
    {
        Value* out[4];
        Value* srcBlend[4];
        Value* dstBlend[4];
        for (uint32_t i = 0; i < 4; ++i)
        {
            srcBlend[i] = FMUL(src[i], srcFactor[i]);
            dstBlend[i] = FMUL(dst[i], dstFactor[i]);
        }

        switch (blendOp)
        {
        case BLENDOP_ADD:
            out[0] = FADD(srcBlend[0], dstBlend[0]);
            out[1] = FADD(srcBlend[1], dstBlend[1]);
            out[2] = FADD(srcBlend[2], dstBlend[2]);
            out[3] = FADD(srcBlend[3], dstBlend[3]);
            break;

        case BLENDOP_SUBTRACT:
            out[0] = FSUB(srcBlend[0], dstBlend[0]);
            out[1] = FSUB(srcBlend[1], dstBlend[1]);
            out[2] = FSUB(srcBlend[2], dstBlend[2]);
            out[3] = FSUB(srcBlend[3], dstBlend[3]);
            break;

        case BLENDOP_REVSUBTRACT:
            out[0] = FSUB(dstBlend[0], srcBlend[0]);
            out[1] = FSUB(dstBlend[1], srcBlend[1]);
            out[2] = FSUB(dstBlend[2], srcBlend[2]);
            out[3] = FSUB(dstBlend[3], srcBlend[3]);
            break;

        case BLENDOP_MIN:
            out[0] = VMINPS(src[0], dst[0]);
            out[1] = VMINPS(src[1], dst[1]);
            out[2] = VMINPS(src[2], dst[2]);
            out[3] = VMINPS(src[3], dst[3]);
            break;

        case BLENDOP_MAX:
            out[0] = VMAXPS(src[0], dst[0]);
            out[1] = VMAXPS(src[1], dst[1]);
            out[2] = VMAXPS(src[2], dst[2]);
            out[3] = VMAXPS(src[3], dst[3]);
            break;

        default:
            SWR_ASSERT(false, "Unsupported blend operation: %d", blendOp);
            out[0] = out[1] = out[2] = out[3] = VIMMED1(0.0f);
            break;
        }

        if (Color)
        {
            result[0] = out[0];
            result[1] = out[1];
            result[2] = out[2];
        }

        if (Alpha)
        {
            result[3] = out[3];
        }
    }

    Function* Create(const BLEND_COMPILE_STATE& state)
    {
        static std::size_t jitNum = 0;

        std::stringstream fnName("BlendShader", std::ios_base::in | std::ios_base::out | std::ios_base::ate);
        fnName << jitNum++;

        // blend function signature
        // typedef void(*PFN_BLEND_JIT_FUNC)(SWR_BLEND_STATE*, simdvector&, simdvector&, uint8_t*, simdvector&);

        std::vector<Type*> args{
            PointerType::get(Gen_SWR_BLEND_STATE(JM()), 0), // SWR_BLEND_STATE*
            PointerType::get(mSimdFP32Ty, 0),               // simdvector& src
            PointerType::get(mSimdFP32Ty, 0),               // simdvector& src1
            PointerType::get(mSimdFP32Ty, 0),               // uint8_t* pDst
            PointerType::get(mSimdFP32Ty, 0),               // simdvector& result
        };

        FunctionType* fTy = FunctionType::get(IRB()->getVoidTy(), args, false);
        Function* blendFunc = Function::Create(fTy, GlobalValue::ExternalLinkage, fnName.str(), JM()->mpCurrentModule);

        BasicBlock* entry = BasicBlock::Create(JM()->mContext, "entry", blendFunc);

        IRB()->SetInsertPoint(entry);

        // arguments
        auto argitr = blendFunc->getArgumentList().begin();
        Value* pBlendState = argitr++;
        pBlendState->setName("pBlendState");
        Value* pSrc = argitr++;
        pSrc->setName("src");
        Value* pSrc1 = argitr++;
        pSrc1->setName("src1");
        Value* pDst = argitr++;
        pDst->setName("pDst");
        Value* pResult = argitr++;
        pResult->setName("result");

        static_assert(KNOB_COLOR_HOT_TILE_FORMAT == R32G32B32A32_FLOAT, "Unsupported hot tile format");
        Value* dst[4];
        Value* constantColor[4];
        Value* src[4];
        Value* src1[4];
        Value* result[4];
        for (uint32_t i = 0; i < 4; ++i)
        {
            // load hot tile
            dst[i] = LOAD(pDst, { i });

            // load constant color
            constantColor[i] = VBROADCAST(LOAD(pBlendState, { 0, SWR_BLEND_STATE_constantColor, i }));

            // load src
            src[i] = LOAD(pSrc, { i });

            // load src1
            src1[i] = LOAD(pSrc1, { i });
        }

        // clamp sources
        Clamp(state.format, src);
        Clamp(state.format, src1);
        Clamp(state.format, dst);
        Clamp(state.format, constantColor);

        // apply defaults to hottile contents to take into account missing components
        ApplyDefaults(state.format, dst);

        // Force defaults for unused 'X' components
        ApplyUnusedDefaults(state.format, dst);

        // Quantize low precision components
        Quantize(state.format, dst);

        // special case clamping for R11G11B10_float which has no sign bit
        if (state.format == R11G11B10_FLOAT)
        {
            dst[0] = VMAXPS(dst[0], VIMMED1(0.0f));
            dst[1] = VMAXPS(dst[1], VIMMED1(0.0f));
            dst[2] = VMAXPS(dst[2], VIMMED1(0.0f));
            dst[3] = VMAXPS(dst[3], VIMMED1(0.0f));
        }

        Value* srcFactor[4];
        Value* dstFactor[4];
        if (state.independentAlphaBlendEnable)
        {
            GenerateBlendFactor<true, false>((SWR_BLEND_FACTOR)state.blendState.sourceBlendFactor, constantColor, src, src1, dst, srcFactor);
            GenerateBlendFactor<false, true>((SWR_BLEND_FACTOR)state.blendState.sourceAlphaBlendFactor, constantColor, src, src1, dst, srcFactor);

            GenerateBlendFactor<true, false>((SWR_BLEND_FACTOR)state.blendState.destBlendFactor, constantColor, src, src1, dst, dstFactor);
            GenerateBlendFactor<false, true>((SWR_BLEND_FACTOR)state.blendState.destAlphaBlendFactor, constantColor, src, src1, dst, dstFactor);

            BlendFunc<true, false>((SWR_BLEND_OP)state.blendState.colorBlendFunc, src, srcFactor, dst, dstFactor, result);
            BlendFunc<false, true>((SWR_BLEND_OP)state.blendState.alphaBlendFunc, src, srcFactor, dst, dstFactor, result);
        }
        else
        {
            GenerateBlendFactor<true, true>((SWR_BLEND_FACTOR)state.blendState.sourceBlendFactor, constantColor, src, src1, dst, srcFactor);
            GenerateBlendFactor<true, true>((SWR_BLEND_FACTOR)state.blendState.destBlendFactor, constantColor, src, src1, dst, dstFactor);

            BlendFunc<true, true>((SWR_BLEND_OP)state.blendState.colorBlendFunc, src, srcFactor, dst, dstFactor, result);
        }

        // store results out
        for (uint32_t i = 0; i < 4; ++i)
        {
            STORE(result[i], pResult, { i });
        }

        RET_VOID();

        JitManager::DumpToFile(blendFunc, "");

        FunctionPassManager passes(JM()->mpCurrentModule);
        passes.add(createBreakCriticalEdgesPass());
        passes.add(createCFGSimplificationPass());
        passes.add(createEarlyCSEPass());
        passes.add(createPromoteMemoryToRegisterPass());
        passes.add(createCFGSimplificationPass());
        passes.add(createEarlyCSEPass());
        passes.add(createInstructionCombiningPass());
        passes.add(createInstructionSimplifierPass());
        passes.add(createConstantPropagationPass());
        passes.add(createSCCPPass());
        passes.add(createAggressiveDCEPass());

        passes.run(*blendFunc);

        JitManager::DumpToFile(blendFunc, "optimized");

        return blendFunc;
    }
};

//////////////////////////////////////////////////////////////////////////
/// @brief JITs from fetch shader IR
/// @param hJitMgr - JitManager handle
/// @param func   - LLVM function IR
/// @return PFN_FETCH_FUNC - pointer to fetch code
PFN_BLEND_JIT_FUNC JitBlendFunc(HANDLE hJitMgr, const HANDLE hFunc)
{
    const llvm::Function *func = (const llvm::Function*)hFunc;
    JitManager* pJitMgr = reinterpret_cast<JitManager*>(hJitMgr);
    PFN_BLEND_JIT_FUNC pfnBlend;
    pfnBlend = (PFN_BLEND_JIT_FUNC)(pJitMgr->mpExec->getFunctionAddress(func->getName().str()));
    // MCJIT finalizes modules the first time you JIT code from them. After finalized, you cannot add new IR to the module
    pJitMgr->mIsModuleFinalized = true;

    return pfnBlend;
}

//////////////////////////////////////////////////////////////////////////
/// @brief JIT compiles blend shader
/// @param hJitMgr - JitManager handle
/// @param state   - blend state to build function from
extern "C" PFN_BLEND_JIT_FUNC JITCALL JitCompileBlend(HANDLE hJitMgr, const BLEND_COMPILE_STATE& state)
{
    JitManager* pJitMgr = reinterpret_cast<JitManager*>(hJitMgr);

    pJitMgr->SetupNewModule();

    BlendJit theJit(pJitMgr);
    HANDLE hFunc = theJit.Create(state);

    return JitBlendFunc(hJitMgr, hFunc);
}
