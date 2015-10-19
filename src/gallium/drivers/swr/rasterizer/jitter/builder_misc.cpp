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
* @file builder_misc.cpp
* 
* @brief Implementation for miscellaneous builder functions
* 
* Notes:
* 
******************************************************************************/
#include "builder.h"
#include "llvm/Support/DynamicLibrary.h"

void __cdecl CallPrint(const char* fmt, ...);

Constant *Builder::C(bool i)
{
    return ConstantInt::get(IRB()->getInt1Ty(), (i ? 1 : 0));
}

Constant *Builder::C(char i)
{
    return ConstantInt::get(IRB()->getInt8Ty(), i);
}

Constant *Builder::C(uint8_t i)
{
    return ConstantInt::get(IRB()->getInt8Ty(), i);
}

Constant *Builder::C(int i)
{
    return ConstantInt::get(IRB()->getInt32Ty(), i);
}

Constant *Builder::C(int64_t i)
{
    return ConstantInt::get(IRB()->getInt64Ty(), i);
}

Constant *Builder::C(UINT16 i)
{
    return ConstantInt::get(mInt16Ty,i);
}

Constant *Builder::C(uint32_t i)
{
    return ConstantInt::get(IRB()->getInt32Ty(), i);
}

Constant *Builder::C(float i)
{
    return ConstantFP::get(IRB()->getFloatTy(), i);
}

Constant *Builder::PRED(bool pred)
{
    return ConstantInt::get(IRB()->getInt1Ty(), (pred ? 1 : 0));
}

Value *Builder::VIMMED1(int i)
{
    return ConstantVector::getSplat(JM()->mVWidth, cast<ConstantInt>(C(i)));
}

Value *Builder::VIMMED1(uint32_t i)
{
    return ConstantVector::getSplat(JM()->mVWidth, cast<ConstantInt>(C(i)));
}

Value *Builder::VIMMED1(float i)
{
    return ConstantVector::getSplat(JM()->mVWidth, cast<ConstantFP>(C(i)));
}

Value *Builder::VIMMED1(bool i)
{
    return ConstantVector::getSplat(JM()->mVWidth, cast<ConstantInt>(C(i)));
}

Value *Builder::VUNDEF_IPTR()
{
    return UndefValue::get(VectorType::get(PointerType::get(mInt32Ty, 0),JM()->mVWidth));
}

Value *Builder::VUNDEF_I()
{
    return UndefValue::get(VectorType::get(mInt32Ty, JM()->mVWidth));
}

Value *Builder::VUNDEF(Type *ty, uint32_t size)
{
    return UndefValue::get(VectorType::get(ty, size));
}

Value *Builder::VUNDEF_F()
{
    return UndefValue::get(VectorType::get(mFP32Ty, JM()->mVWidth));
}

Value *Builder::VUNDEF(Type* t)
{
    return UndefValue::get(VectorType::get(t, JM()->mVWidth));
}

Value *Builder::VINSERT(Value *vec, Value *val, int index)
{
    return VINSERT(vec, val, C(index));
}

Value *Builder::VBROADCAST(Value *src)
{
    // check if src is already a vector
    if (src->getType()->isVectorTy())
    {
        return src;
    }

    Value *vecRet = VUNDEF(src->getType());
    vecRet = VINSERT(vecRet, src, 0);
    vecRet = VSHUFFLE(vecRet, vecRet, VIMMED1(0));

    return vecRet;
}

uint32_t Builder::IMMED(Value* v)
{
    SWR_ASSERT(isa<ConstantInt>(v));
    ConstantInt *pValConst = cast<ConstantInt>(v);
    return pValConst->getZExtValue();
}

Value *Builder::GEP(Value* ptr, const std::initializer_list<Value*> &indexList)
{
    std::vector<Value*> indices;
    for (auto i : indexList)
        indices.push_back(i);
    return GEPA(ptr, indices);
}

Value *Builder::GEP(Value* ptr, const std::initializer_list<uint32_t> &indexList)
{
    std::vector<Value*> indices;
    for (auto i : indexList)
        indices.push_back(C(i));
    return GEPA(ptr, indices);
}

LoadInst *Builder::LOAD(Value *basePtr, const std::initializer_list<uint32_t> &indices, const llvm::Twine& name)
{
    std::vector<Value*> valIndices;
    for (auto i : indices)
        valIndices.push_back(C(i));
    return LOAD(GEPA(basePtr, valIndices), name);
}

LoadInst *Builder::LOADV(Value *basePtr, const std::initializer_list<Value*> &indices, const llvm::Twine& name)
{
    std::vector<Value*> valIndices;
    for (auto i : indices)
        valIndices.push_back(i);
    return LOAD(GEPA(basePtr, valIndices), name);
}

StoreInst *Builder::STORE(Value *val, Value *basePtr, const std::initializer_list<uint32_t> &indices)
{
    std::vector<Value*> valIndices;
    for (auto i : indices)
        valIndices.push_back(C(i));
    return STORE(val, GEPA(basePtr, valIndices));
}

StoreInst *Builder::STOREV(Value *val, Value *basePtr, const std::initializer_list<Value*> &indices)
{
    std::vector<Value*> valIndices;
    for (auto i : indices)
        valIndices.push_back(i);
    return STORE(val, GEPA(basePtr, valIndices));
}

CallInst *Builder::CALL(Value *Callee, const std::initializer_list<Value*> &argsList)
{
    std::vector<Value*> args;
    for (auto arg : argsList)
        args.push_back(arg);
    return CALLA(Callee, args);
}

Value *Builder::VRCP(Value *va)
{
    return FDIV(VIMMED1(1.0f), va);  // 1 / a
}

Value *Builder::VPLANEPS(Value* vA, Value* vB, Value* vC, Value* &vX, Value* &vY)
{
    Value* vOut = FMADDPS(vA, vX, vC);
    vOut = FMADDPS(vB, vY, vOut);
    return vOut;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate an i32 masked load operation in LLVM IR.  If not  
/// supported on the underlying platform, emulate it with float masked load
/// @param src - base address pointer for the load
/// @param vMask - SIMD wide mask that controls whether to access memory load 0
Value *Builder::MASKLOADD(Value* src,Value* mask)
{
    Value* vResult;
    // use avx2 gather instruction is available
    if(JM()->mArch.AVX2())
    {
        Function *func = Intrinsic::getDeclaration(JM()->mpCurrentModule, Intrinsic::x86_avx2_maskload_d_256);
        vResult = CALL2(func,src,mask);   
    }
    else
    {
        Function *func = Intrinsic::getDeclaration(JM()->mpCurrentModule,Intrinsic::x86_avx_maskload_ps_256);
        Value* fMask = BITCAST(mask,VectorType::get(mFP32Ty,JM()->mVWidth));
        vResult = BITCAST(CALL2(func,src,fMask), VectorType::get(mInt32Ty,JM()->mVWidth));
    }
    return vResult;
}

//////////////////////////////////////////////////////////////////////////
/// @brief insert a JIT call to CallPrint
/// - outputs formatted string to both stdout and VS output window
/// - DEBUG builds only
/// Usage example:
///   PRINT("index %d = 0x%p\n",{C(lane), pIndex});
///   where C(lane) creates a constant value to print, and pIndex is the Value*
///   result from a GEP, printing out the pointer to memory
/// @param printStr - constant string to print, which includes format specifiers
/// @param printArgs - initializer list of Value*'s to print to std out
CallInst *Builder::PRINT(const std::string &printStr,const std::initializer_list<Value*> &printArgs)
{
#if defined( DEBUG ) || defined( _DEBUG )
    // push the arguments to CallPrint into a vector
    std::vector<Value*> printCallArgs;
    // save room for the format string.  we still need to modify it for vectors
    printCallArgs.resize(1);

    // search through the format string for special processing
    size_t pos = 0;
    std::string tempStr(printStr);
    pos = tempStr.find('%', pos);
    auto v = printArgs.begin();
    // printing is slow.  now it's slower...
    while((pos != std::string::npos) && (v != printArgs.end()))
    {
        // for %f we need to cast float Values to doubles so that they print out correctly
        if((tempStr[pos+1]=='f') && ((*v)->getType()->isFloatTy()))
        {
            printCallArgs.push_back(FP_EXT(*v, Type::getDoubleTy(JM()->mContext)));
            pos++;
        }
        // add special handling for %f and %d format specifiers to make printing llvm vector types easier
        else if((*v)->getType()->isVectorTy())
        {
            if((tempStr[pos+1]=='f') && ((*v)->getType()->getContainedType(0)->isFloatTy()))
            {
                uint32_t i = 0;
                for( ; i < ((*v)->getType()->getVectorNumElements())-1; i++)
                {
                    tempStr.insert(pos, std::string("%f "));
                    pos+=3;
                    printCallArgs.push_back(FP_EXT(VEXTRACT(*v, C(i)), Type::getDoubleTy(JM()->mContext)));
                }
                printCallArgs.push_back(FP_EXT(VEXTRACT(*v,C(i)),Type::getDoubleTy(JM()->mContext)));
            }
            else if((tempStr[pos+1]=='d') && ((*v)->getType()->getContainedType(0)->isIntegerTy()))
            {
                uint32_t i = 0;
                for( ; i < ((*v)->getType()->getVectorNumElements())-1; i++)
                {
                    tempStr.insert(pos,std::string("%d "));
                    pos += 3;
                    printCallArgs.push_back(VEXTRACT(*v,C(i)));
                }
                printCallArgs.push_back(VEXTRACT(*v,C(i)));
            }
            else
            {
                /// not a supported vector to print
                /// @todo pointer types too
                SWR_ASSERT(0);
            }
        }
        else
        {
            printCallArgs.push_back(*v);
        }
 
        // advance to the next arguement
        v++;
        pos = tempStr.find('%', ++pos);
    }

    // create global variable constant string
    Constant *constString = ConstantDataArray::getString(JM()->mContext,tempStr,true);
    GlobalVariable *gvPtr = new GlobalVariable(constString->getType(),true,GlobalValue::InternalLinkage,constString,"printStr");
    JM()->mpCurrentModule->getGlobalList().push_back(gvPtr);

    // get a pointer to the first character in the constant string array
    std::vector<Constant*> geplist{C(0),C(0)};
    Constant *strGEP = ConstantExpr::getGetElementPtr(gvPtr,geplist,false);

    // insert the pointer to the format string in the argument vector
    printCallArgs[0] = strGEP;

    // get pointer to CallPrint function and insert decl into the module if needed
    std::vector<Type*> args;
    args.push_back(PointerType::get(mInt8Ty,0));
    FunctionType* callPrintTy = FunctionType::get(Type::getVoidTy(JM()->mContext),args,true);
    Function *callPrintFn = cast<Function>(JM()->mpCurrentModule->getOrInsertFunction("CallPrint", callPrintTy));

    // if we haven't yet added the symbol to the symbol table
    if((sys::DynamicLibrary::SearchForAddressOfSymbol("CallPrint")) == nullptr)
    {
        sys::DynamicLibrary::AddSymbol("CallPrint", (void *)&CallPrint);
    }

    // insert a call to CallPrint
    return CALLA(callPrintFn,printCallArgs);
#else // #if defined( DEBUG ) || defined( _DEBUG )
    return nullptr;
#endif
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a masked gather operation in LLVM IR.  If not  
/// supported on the underlying platform, emulate it with loads
/// @param vSrc - SIMD wide value that will be loaded if mask is invalid
/// @param pBase - Int8* base VB address pointer value
/// @param vIndices - SIMD wide value of VB byte offsets
/// @param vMask - SIMD wide mask that controls whether to access memory or the src values
/// @param scale - value to scale indices by
Value *Builder::GATHERPS(Value* vSrc, Value* pBase, Value* vIndices, Value* vMask, Value* scale)
{
    Value* vGather;

    // use avx2 gather instruction if available
    if(JM()->mArch.AVX2())
    {
        // force mask to <N x float>, required by vgather
        vMask = BITCAST(vMask, mSimdFP32Ty);
        vGather = VGATHERPS(vSrc,pBase,vIndices,vMask,scale);
    }
    else
    {
        Value* pStack = STACKSAVE();

        // store vSrc on the stack.  this way we can select between a valid load address and the vSrc address
        Value* vSrcPtr = ALLOCA(vSrc->getType());
        STORE(vSrc, vSrcPtr);

        vGather = VUNDEF_F();
        Value *vScaleVec = VBROADCAST(Z_EXT(scale,mInt32Ty));
        Value *vOffsets = MUL(vIndices,vScaleVec);
        Value *mask = MASK(vMask);
        for(uint32_t i = 0; i < JM()->mVWidth; ++i)
        {
            // single component byte index
            Value *offset = VEXTRACT(vOffsets,C(i));
            // byte pointer to component
            Value *loadAddress = GEP(pBase,offset);
            loadAddress = BITCAST(loadAddress,PointerType::get(mFP32Ty,0));
            // pointer to the value to load if we're masking off a component
            Value *maskLoadAddress = GEP(vSrcPtr,{C(0), C(i)});
            Value *selMask = VEXTRACT(mask,C(i));
            // switch in a safe address to load if we're trying to access a vertex 
            Value *validAddress = SELECT(selMask, loadAddress, maskLoadAddress);
            Value *val = LOAD(validAddress);
            vGather = VINSERT(vGather,val,C(i));
        }
        STACKRESTORE(pStack);
    }

    return vGather;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a masked gather operation in LLVM IR.  If not  
/// supported on the underlying platform, emulate it with loads
/// @param vSrc - SIMD wide value that will be loaded if mask is invalid
/// @param pBase - Int8* base VB address pointer value
/// @param vIndices - SIMD wide value of VB byte offsets
/// @param vMask - SIMD wide mask that controls whether to access memory or the src values
/// @param scale - value to scale indices by
Value *Builder::GATHERDD(Value* vSrc, Value* pBase, Value* vIndices, Value* vMask, Value* scale)
{
    Value* vGather;

    // use avx2 gather instruction if available
    if(JM()->mArch.AVX2())
    {
        vGather = VGATHERDD(vSrc, pBase, vIndices, vMask, scale);
    }
    else
    {
        Value* pStack = STACKSAVE();

        // store vSrc on the stack.  this way we can select between a valid load address and the vSrc address
        Value* vSrcPtr = ALLOCA(vSrc->getType());
        STORE(vSrc, vSrcPtr);

        vGather = VUNDEF_I();
        Value *vScaleVec = VBROADCAST(Z_EXT(scale, mInt32Ty));
        Value *vOffsets = MUL(vIndices, vScaleVec);
        Value *mask = MASK(vMask);
        for(uint32_t i = 0; i < JM()->mVWidth; ++i)
        {
            // single component byte index
            Value *offset = VEXTRACT(vOffsets, C(i));
            // byte pointer to component
            Value *loadAddress = GEP(pBase, offset);
            loadAddress = BITCAST(loadAddress, PointerType::get(mInt32Ty, 0));
            // pointer to the value to load if we're masking off a component
            Value *maskLoadAddress = GEP(vSrcPtr, {C(0), C(i)});
            Value *selMask = VEXTRACT(mask, C(i));
            // switch in a safe address to load if we're trying to access a vertex 
            Value *validAddress = SELECT(selMask, loadAddress, maskLoadAddress);
            Value *val = LOAD(validAddress, C(0));
            vGather = VINSERT(vGather, val, C(i));
        }

        STACKRESTORE(pStack);
    }
    return vGather;
}

//////////////////////////////////////////////////////////////////////////
/// @brief convert x86 <N x float> mask to llvm <N x i1> mask
Value* Builder::MASK(Value* vmask)
{
    Value* src = BITCAST(vmask, mSimdInt32Ty);
    return ICMP_SLT(src, VIMMED1(0));
}

//////////////////////////////////////////////////////////////////////////
/// @brief convert llvm <N x i1> mask to x86 <N x i32> mask
Value* Builder::VMASK(Value* mask)
{
    return S_EXT(mask, mSimdInt32Ty);
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a VPSHUFB operation in LLVM IR.  If not  
/// supported on the underlying platform, emulate it
/// @param a - 256bit SIMD(32x8bit) of 8bit integer values
/// @param b - 256bit SIMD(32x8bit) of 8bit integer mask values
/// Byte masks in lower 128 lane of b selects 8 bit values from lower 
/// 128bits of a, and vice versa for the upper lanes.  If the mask 
/// value is negative, '0' is inserted.
Value *Builder::PSHUFB(Value* a, Value* b)
{
    Value* res;
    // use avx2 pshufb instruction if available
    if(JM()->mArch.AVX2())
    {
        res = VPSHUFB(a, b);
    }
    else
    {
        Constant* cB = dyn_cast<Constant>(b);
        // number of 8 bit elements in b
        uint32_t numElms = cast<VectorType>(cB->getType())->getNumElements();
        // output vector
        Value* vShuf = UndefValue::get(VectorType::get(mInt8Ty, numElms));

        // insert an 8 bit value from the high and low lanes of a per loop iteration
        numElms /= 2;
        for(uint32_t i = 0; i < numElms; i++)
        {
            ConstantInt* cLow128b = cast<ConstantInt>(cB->getAggregateElement(i));
            ConstantInt* cHigh128b = cast<ConstantInt>(cB->getAggregateElement(i + numElms));

            // extract values from constant mask
            char valLow128bLane =  (char)(cLow128b->getSExtValue());
            char valHigh128bLane = (char)(cHigh128b->getSExtValue());

            Value* insertValLow128b;
            Value* insertValHigh128b;

            // if the mask value is negative, insert a '0' in the respective output position
            // otherwise, lookup the value at mask position (bits 3..0 of the respective mask byte) in a and insert in output vector
            insertValLow128b = (valLow128bLane < 0) ? C((char)0) : VEXTRACT(a, C((valLow128bLane & 0xF)));
            insertValHigh128b = (valHigh128bLane < 0) ? C((char)0) : VEXTRACT(a, C((valHigh128bLane & 0xF) + numElms));

            vShuf = VINSERT(vShuf, insertValLow128b, i);
            vShuf = VINSERT(vShuf, insertValHigh128b, (i + numElms));
        }
        res = vShuf;
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a VPSHUFB operation (sign extend 8 8bit values to 32 
/// bits)in LLVM IR.  If not supported on the underlying platform, emulate it
/// @param a - 128bit SIMD lane(16x8bit) of 8bit integer values.  Only 
/// lower 8 values are used.
Value *Builder::PMOVSXBD(Value* a)
{
    Value* res;
    // use avx2 byte sign extend instruction if available
    if(JM()->mArch.AVX2())
    {
        res = VPMOVSXBD(a);
    }
    else
    {
        // VPMOVSXBD output type
        Type* v8x32Ty = VectorType::get(mInt32Ty, 8);
        // Extract 8 values from 128bit lane and sign extend
        res = S_EXT(VSHUFFLE(a, a, C<int>({0, 1, 2, 3, 4, 5, 6, 7})), v8x32Ty);
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a VPSHUFB operation (sign extend 8 16bit values to 32 
/// bits)in LLVM IR.  If not supported on the underlying platform, emulate it
/// @param a - 128bit SIMD lane(8x16bit) of 16bit integer values.
Value *Builder::PMOVSXWD(Value* a)
{
    Value* res;
    // use avx2 word sign extend if available
    if(JM()->mArch.AVX2())
    {
        res = VPMOVSXWD(a);
    }
    else
    {
        // VPMOVSXWD output type
        Type* v8x32Ty = VectorType::get(mInt32Ty, 8);
        // Extract 8 values from 128bit lane and sign extend
        res = S_EXT(VSHUFFLE(a, a, C<int>({0, 1, 2, 3, 4, 5, 6, 7})), v8x32Ty);
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a VPERMD operation (shuffle 32 bit integer values 
/// across 128 bit lanes) in LLVM IR.  If not supported on the underlying 
/// platform, emulate it
/// @param a - 256bit SIMD lane(8x32bit) of integer values.
/// @param idx - 256bit SIMD lane(8x32bit) of 3 bit lane index values
Value *Builder::PERMD(Value* a, Value* idx)
{
    Value* res;
    // use avx2 permute instruction if available
    if(JM()->mArch.AVX2())
    {
        // llvm 3.6.0 swapped the order of the args to vpermd
        res = VPERMD(idx, a);
    }
    else
    {
        res = VSHUFFLE(a, a, idx);
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a VCVTPH2PS operation (float16->float32 conversion)
/// in LLVM IR.  If not supported on the underlying platform, emulate it
/// @param a - 128bit SIMD lane(8x16bit) of float16 in int16 format.
Value *Builder::CVTPH2PS(Value* a)
{
    if (JM()->mArch.F16C())
    {
        return VCVTPH2PS(a);
    }
    else
    {
        Value* vExt = S_EXT(a, mSimdInt32Ty);
        Value* sign = AND(vExt,0x80000000);

        // normal case
        Value* mantissa = SHL(AND(vExt, 0x03ff), 13);
        Value* exponent = AND(vExt, 0x7c00);
        exponent = ADD(exponent, VIMMED1(0x1c000));
        exponent = SHL(exponent, 13);

        Value* result = OR(OR(sign, mantissa), exponent);

        // handle 0
        Value* zeroMask = ICMP_EQ(AND(vExt, 0x7fff), VIMMED1(0));
        result = SELECT(zeroMask, sign, result);

        // handle infinity
        Value* infMask = ICMP_EQ(AND(vExt, 0x7c00), VIMMED1(0x7c00));
        Value* signedInf = OR(VIMMED1(0x7f800000), sign);
        result = SELECT(infMask, signedInf, result);

        // @todo handle subnormal

        // cast to f32
        result = BITCAST(result, mSimdFP32Ty);
        return result;
    }
}

//////////////////////////////////////////////////////////////////////////
/// @brief Generate a VCVTPS2PH operation (float32->float16 conversion)
/// in LLVM IR.  If not supported on the underlying platform, emulate it
/// @param a - 128bit SIMD lane(8x16bit) of float16 in int16 format.
Value *Builder::CVTPS2PH(Value* a, Value* rounding)
{
    if (JM()->mArch.F16C())
    {
        return VCVTPS2PH(a, rounding);
    }
    else
    {
        SWR_ASSERT(false, "Emulation of VCVTPH2PS unimplemented.");
        return nullptr;
    }
}

Value *Builder::PMAXSD(Value* a, Value* b)
{
    if (JM()->mArch.AVX2())
    {
        return VPMAXSD(a, b);
    }
    else
    {
        // use 4-wide sse max intrinsic on lower/upper halves of 8-wide sources
        Function* pmaxsd = Intrinsic::getDeclaration(JM()->mpCurrentModule, Intrinsic::x86_sse41_pmaxsd);

        // low 128
        Value* aLo = VEXTRACTI128(a, C((uint8_t)0));
        Value* bLo = VEXTRACTI128(b, C((uint8_t)0));
        Value* resLo = CALL2(pmaxsd, aLo, bLo);

        // high 128
        Value* aHi = VEXTRACTI128(a, C((uint8_t)1));
        Value* bHi = VEXTRACTI128(b, C((uint8_t)1));
        Value* resHi = CALL2(pmaxsd, aHi, bHi);

        // combine 
        Value* result = VINSERTI128(VUNDEF_I(), resLo, C((uint8_t)0));
        result = VINSERTI128(result, resHi, C((uint8_t)1));

        return result;
    }
}

Value *Builder::PMINSD(Value* a, Value* b)
{
    if (JM()->mArch.AVX2())
    {
        return VPMINSD(a, b);
    }
    else
    {
        // use 4-wide sse max intrinsic on lower/upper halves of 8-wide sources
        Function* pminsd = Intrinsic::getDeclaration(JM()->mpCurrentModule, Intrinsic::x86_sse41_pminsd);

        // low 128
        Value* aLo = VEXTRACTI128(a, C((uint8_t)0));
        Value* bLo = VEXTRACTI128(b, C((uint8_t)0));
        Value* resLo = CALL2(pminsd, aLo, bLo);

        // high 128
        Value* aHi = VEXTRACTI128(a, C((uint8_t)1));
        Value* bHi = VEXTRACTI128(b, C((uint8_t)1));
        Value* resHi = CALL2(pminsd, aHi, bHi);

        // combine 
        Value* result = VINSERTI128(VUNDEF_I(), resLo, C((uint8_t)0));
        result = VINSERTI128(result, resHi, C((uint8_t)1));

        return result;
    }
}

void Builder::Gather4(const SWR_FORMAT format, Value* pSrcBase, Value* byteOffsets, 
                      Value* mask, Value* vGatherComponents[], bool bPackedOutput)
{
    const SWR_FORMAT_INFO &info = GetFormatInfo(format);
    if(info.type[0] == SWR_TYPE_FLOAT && info.bpc[0] == 32)
    {
        // ensure our mask is the correct type
        mask = BITCAST(mask, mSimdFP32Ty);
        GATHER4PS(info, pSrcBase, byteOffsets, mask, vGatherComponents, bPackedOutput);
    }
    else
    {
        // ensure our mask is the correct type
        mask = BITCAST(mask, mSimdInt32Ty);
        GATHER4DD(info, pSrcBase, byteOffsets, mask, vGatherComponents, bPackedOutput);
    }
}

void Builder::GATHER4PS(const SWR_FORMAT_INFO &info, Value* pSrcBase, Value* byteOffsets, 
                        Value* mask, Value* vGatherComponents[], bool bPackedOutput)
{
    switch(info.bpp / info.numComps)
    {
        case 16: 
        {
                Value* vGatherResult[2];
                Value *vMask;

                // TODO: vGatherMaskedVal
                Value* vGatherMaskedVal = VIMMED1((float)0);

                // always have at least one component out of x or y to fetch

                // save mask as it is zero'd out after each gather
                vMask = mask;

                vGatherResult[0] = GATHERPS(vGatherMaskedVal, pSrcBase, byteOffsets, vMask, C((char)1));
                // e.g. result of first 8x32bit integer gather for 16bit components
                // 256i - 0    1    2    3    4    5    6    7
                //        xyxy xyxy xyxy xyxy xyxy xyxy xyxy xyxy
                //

                // if we have at least one component out of x or y to fetch
                if(info.numComps > 2)
                {
                    // offset base to the next components(zw) in the vertex to gather
                    pSrcBase = GEP(pSrcBase, C((char)4));
                    vMask = mask;

                    vGatherResult[1] =  GATHERPS(vGatherMaskedVal, pSrcBase, byteOffsets, vMask, C((char)1));
                    // e.g. result of second 8x32bit integer gather for 16bit components
                    // 256i - 0    1    2    3    4    5    6    7
                    //        zwzw zwzw zwzw zwzw zwzw zwzw zwzw zwzw 
                    //
                }
                else
                {
                    vGatherResult[1] =  vGatherMaskedVal;
                }

                // Shuffle gathered components into place, each row is a component
                Shuffle16bpcGather4(info, vGatherResult, vGatherComponents, bPackedOutput);  
        }
            break;
        case 32: 
        { 
            // apply defaults
            for (uint32_t i = 0; i < 4; ++i)
            {
                vGatherComponents[i] = VIMMED1(*(float*)&info.defaults[i]);
            }

            for(uint32_t i = 0; i < info.numComps; i++)
            {
                uint32_t swizzleIndex = info.swizzle[i];

                // save mask as it is zero'd out after each gather
                Value *vMask = mask;

                // Gather a SIMD of components
                vGatherComponents[swizzleIndex] = GATHERPS(vGatherComponents[swizzleIndex], pSrcBase, byteOffsets, vMask, C((char)1));

                // offset base to the next component to gather
                pSrcBase = GEP(pSrcBase, C((char)4));
            }
        }
            break;
        default:
            SWR_ASSERT(0, "Invalid float format");
            break;
    }
}

void Builder::GATHER4DD(const SWR_FORMAT_INFO &info, Value* pSrcBase, Value* byteOffsets,
                        Value* mask, Value* vGatherComponents[], bool bPackedOutput)
{
    switch (info.bpp / info.numComps)
    {
        case 8:
        {
            Value* vGatherMaskedVal = VIMMED1((int32_t)0);
            Value* vGatherResult = GATHERDD(vGatherMaskedVal, pSrcBase, byteOffsets, mask, C((char)1));
            // e.g. result of an 8x32bit integer gather for 8bit components
            // 256i - 0    1    2    3    4    5    6    7
            //        xyzw xyzw xyzw xyzw xyzw xyzw xyzw xyzw 

            Shuffle8bpcGather4(info, vGatherResult, vGatherComponents, bPackedOutput);  
        }
            break;
        case 16:
        {
            Value* vGatherResult[2];
            Value *vMask;

            // TODO: vGatherMaskedVal
            Value* vGatherMaskedVal = VIMMED1((int32_t)0);

            // always have at least one component out of x or y to fetch

            // save mask as it is zero'd out after each gather
            vMask = mask;

            vGatherResult[0] = GATHERDD(vGatherMaskedVal, pSrcBase, byteOffsets, vMask, C((char)1));
            // e.g. result of first 8x32bit integer gather for 16bit components
            // 256i - 0    1    2    3    4    5    6    7
            //        xyxy xyxy xyxy xyxy xyxy xyxy xyxy xyxy
            //

            // if we have at least one component out of x or y to fetch
            if(info.numComps > 2)
            {
                // offset base to the next components(zw) in the vertex to gather
                pSrcBase = GEP(pSrcBase, C((char)4));
                vMask = mask;

                vGatherResult[1] = GATHERDD(vGatherMaskedVal, pSrcBase, byteOffsets, vMask, C((char)1));
                // e.g. result of second 8x32bit integer gather for 16bit components
                // 256i - 0    1    2    3    4    5    6    7
                //        zwzw zwzw zwzw zwzw zwzw zwzw zwzw zwzw 
                //
            }
            else
            {
                vGatherResult[1] = vGatherMaskedVal;
            }

            // Shuffle gathered components into place, each row is a component
            Shuffle16bpcGather4(info, vGatherResult, vGatherComponents, bPackedOutput);

        }
            break;
        case 32:
        {
            // apply defaults
            for (uint32_t i = 0; i < 4; ++i)
            {
                vGatherComponents[i] = VIMMED1((int)info.defaults[i]);
            }

            for(uint32_t i = 0; i < info.numComps; i++)
            {
                uint32_t swizzleIndex = info.swizzle[i];

                // save mask as it is zero'd out after each gather
                Value *vMask = mask;

                // Gather a SIMD of components
                vGatherComponents[swizzleIndex] = GATHERDD(vGatherComponents[swizzleIndex], pSrcBase, byteOffsets, vMask, C((char)1));

                // offset base to the next component to gather
                pSrcBase = GEP(pSrcBase, C((char)4));
            }
        }
            break;
        default:
            SWR_ASSERT(0, "unsupported format");
        break;
    }
}

void Builder::Shuffle16bpcGather4(const SWR_FORMAT_INFO &info, Value* vGatherInput[2], Value* vGatherOutput[4], bool bPackedOutput)
{
    // cast types
    Type* vGatherTy = VectorType::get(IntegerType::getInt32Ty(JM()->mContext), JM()->mVWidth);
    Type* v32x8Ty = VectorType::get(mInt8Ty, JM()->mVWidth * 4); // vwidth is units of 32 bits

    // input could either be float or int vector; do shuffle work in int
    vGatherInput[0] = BITCAST(vGatherInput[0], mSimdInt32Ty);
    vGatherInput[1] = BITCAST(vGatherInput[1], mSimdInt32Ty);

    if(bPackedOutput) 
    {
        Type* v128bitTy = VectorType::get(IntegerType::getIntNTy(JM()->mContext, 128), JM()->mVWidth / 4); // vwidth is units of 32 bits

        // shuffle mask
        Value* vConstMask = C<char>({0, 1, 4, 5, 8, 9, 12, 13, 2, 3, 6, 7, 10, 11, 14, 15,
                                     0, 1, 4, 5, 8, 9, 12, 13, 2, 3, 6, 7, 10, 11, 14, 15});
        Value* vShufResult = BITCAST(PSHUFB(BITCAST(vGatherInput[0], v32x8Ty), vConstMask), vGatherTy);
        // after pshufb: group components together in each 128bit lane
        // 256i - 0    1    2    3    4    5    6    7
        //        xxxx xxxx yyyy yyyy xxxx xxxx yyyy yyyy

        Value* vi128XY = BITCAST(PERMD(vShufResult, C<int32_t>({0, 1, 4, 5, 2, 3, 6, 7})), v128bitTy);
        // after PERMD: move and pack xy components into each 128bit lane
        // 256i - 0    1    2    3    4    5    6    7
        //        xxxx xxxx xxxx xxxx yyyy yyyy yyyy yyyy

        // do the same for zw components
        Value* vi128ZW = nullptr;
        if(info.numComps > 2) 
        {
            Value* vShufResult = BITCAST(PSHUFB(BITCAST(vGatherInput[1], v32x8Ty), vConstMask), vGatherTy);
            vi128ZW = BITCAST(PERMD(vShufResult, C<int32_t>({0, 1, 4, 5, 2, 3, 6, 7})), v128bitTy);
        }

        for(uint32_t i = 0; i < 4; i++)
        {
            uint32_t swizzleIndex = info.swizzle[i];
            // todo: fixed for packed
            Value* vGatherMaskedVal = VIMMED1((int32_t)(info.defaults[i]));
            if(i >= info.numComps)
            {
                // set the default component val
                vGatherOutput[swizzleIndex] = vGatherMaskedVal;
                continue;
            }

            // if x or z, extract 128bits from lane 0, else for y or w, extract from lane 1
            uint32_t lane = ((i == 0) || (i == 2)) ? 0 : 1;
            // if x or y, use vi128XY permute result, else use vi128ZW
            Value* selectedPermute = (i < 2) ? vi128XY : vi128ZW;

            // extract packed component 128 bit lanes 
            vGatherOutput[swizzleIndex] = VEXTRACT(selectedPermute, C(lane));
        }

    }
    else 
    {
        // pshufb masks for each component
        Value* vConstMask[2];
        // x/z shuffle mask
        vConstMask[0] = C<char>({0, 1, -1, -1, 4, 5, -1, -1, 8, 9, -1, -1, 12, 13, -1, -1,
                                 0, 1, -1, -1, 4, 5, -1, -1, 8, 9, -1, -1, 12, 13, -1, -1, });

        // y/w shuffle mask
        vConstMask[1] = C<char>({2, 3, -1, -1, 6, 7, -1, -1, 10, 11, -1, -1, 14, 15, -1, -1,
                                 2, 3, -1, -1, 6, 7, -1, -1, 10, 11, -1, -1, 14, 15, -1, -1});


        // shuffle enabled components into lower word of each 32bit lane, 0 extending to 32 bits
        // apply defaults
        for (uint32_t i = 0; i < 4; ++i)
        {
            vGatherOutput[i] = VIMMED1((int32_t)info.defaults[i]);
        }

        for(uint32_t i = 0; i < info.numComps; i++)
        {
            uint32_t swizzleIndex = info.swizzle[i];

            // select correct constMask for x/z or y/w pshufb
            uint32_t selectedMask = ((i == 0) || (i == 2)) ? 0 : 1;
            // if x or y, use vi128XY permute result, else use vi128ZW
            uint32_t selectedGather = (i < 2) ? 0 : 1;

            vGatherOutput[swizzleIndex] = BITCAST(PSHUFB(BITCAST(vGatherInput[selectedGather], v32x8Ty), vConstMask[selectedMask]), vGatherTy);
            // after pshufb mask for x channel; z uses the same shuffle from the second gather
            // 256i - 0    1    2    3    4    5    6    7
            //        xx00 xx00 xx00 xx00 xx00 xx00 xx00 xx00 
        }
    }
}

void Builder::Shuffle8bpcGather4(const SWR_FORMAT_INFO &info, Value* vGatherInput, Value* vGatherOutput[], bool bPackedOutput)
{
    // cast types
    Type* vGatherTy = VectorType::get(IntegerType::getInt32Ty(JM()->mContext), JM()->mVWidth);
    Type* v32x8Ty =  VectorType::get(mInt8Ty, JM()->mVWidth * 4 ); // vwidth is units of 32 bits

    if(bPackedOutput)
    {
        Type* v128Ty = VectorType::get(IntegerType::getIntNTy(JM()->mContext, 128), JM()->mVWidth / 4); // vwidth is units of 32 bits
        // shuffle mask
        Value* vConstMask = C<char>({0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
                                     0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15});
        Value* vShufResult = BITCAST(PSHUFB(BITCAST(vGatherInput, v32x8Ty), vConstMask), vGatherTy);
        // after pshufb: group components together in each 128bit lane
        // 256i - 0    1    2    3    4    5    6    7
        //        xxxx yyyy zzzz wwww xxxx yyyy zzzz wwww

        Value* vi128XY = BITCAST(PERMD(vShufResult, C<int32_t>({0, 4, 0, 0, 1, 5, 0, 0})), v128Ty);
        // after PERMD: move and pack xy and zw components in low 64 bits of each 128bit lane
        // 256i - 0    1    2    3    4    5    6    7
        //        xxxx xxxx dcdc dcdc yyyy yyyy dcdc dcdc (dc - don't care)

        // do the same for zw components
        Value* vi128ZW = nullptr;
        if(info.numComps > 2) 
        {
            vi128ZW = BITCAST(PERMD(vShufResult, C<int32_t>({2, 6, 0, 0, 3, 7, 0, 0})), v128Ty);
        }

        // sign extend all enabled components. If we have a fill vVertexElements, output to current simdvertex
        for(uint32_t i = 0; i < 4; i++)
        {
            uint32_t swizzleIndex = info.swizzle[i];
            // todo: fix for packed
            Value* vGatherMaskedVal = VIMMED1((int32_t)(info.defaults[i]));
            if(i >= info.numComps)
            {
                // set the default component val
                vGatherOutput[swizzleIndex] = vGatherMaskedVal;
                continue;
            }

            // if x or z, extract 128bits from lane 0, else for y or w, extract from lane 1
            uint32_t lane = ((i == 0) || (i == 2)) ? 0 : 1; 
            // if x or y, use vi128XY permute result, else use vi128ZW
            Value* selectedPermute = (i < 2) ? vi128XY : vi128ZW;
            
            // sign extend
            vGatherOutput[swizzleIndex] = VEXTRACT(selectedPermute, C(lane));
        }
    }
    // else zero extend
    else{
        // shuffle enabled components into lower byte of each 32bit lane, 0 extending to 32 bits
        // apply defaults
        for (uint32_t i = 0; i < 4; ++i)
        {
            vGatherOutput[i] = VIMMED1((int32_t)info.defaults[i]);
        }

        for(uint32_t i = 0; i < info.numComps; i++){
            uint32_t swizzleIndex = info.swizzle[i];

            // pshufb masks for each component
            Value* vConstMask;
            switch(i)
            {
                case 0:
                    // x shuffle mask
                    vConstMask = C<char>({0, -1, -1, -1, 4, -1, -1, -1, 8, -1, -1, -1, 12, -1, -1, -1,
                                          0, -1, -1, -1, 4, -1, -1, -1, 8, -1, -1, -1, 12, -1, -1, -1});
                    break;
                case 1:
                    // y shuffle mask
                    vConstMask = C<char>({1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
                                          1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1});
                    break;
                case 2:
                    // z shuffle mask
                    vConstMask = C<char>({2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
                                          2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1});
                    break;
                case 3:
                    // w shuffle mask
                    vConstMask = C<char>({3, -1, -1, -1, 7, -1, -1, -1, 11, -1, -1, -1, 15, -1, -1, -1,
                                          3, -1, -1, -1, 7, -1, -1, -1, 11, -1, -1, -1, 15, -1, -1, -1});
                    break;
                default:
                    vConstMask = nullptr;
                    break;
            }

                vGatherOutput[swizzleIndex] = BITCAST(PSHUFB(BITCAST(vGatherInput, v32x8Ty), vConstMask), vGatherTy);
                // after pshufb for x channel
                // 256i - 0    1    2    3    4    5    6    7
                //        x000 x000 x000 x000 x000 x000 x000 x000 
        }
    }
}

//////////////////////////////////////////////////////////////////////////
/// @brief emulates a scatter operation.
/// @param pDst - pointer to destination 
/// @param vSrc - vector of src data to scatter
/// @param vOffsets - vector of byte offsets from pDst
/// @param vMask - mask of valid lanes
void Builder::SCATTERPS(Value* pDst, Value* vSrc, Value* vOffsets, Value* vMask)
{
    Value* pStack = STACKSAVE();

    // allocate tmp stack for masked off lanes
    Value* vTmpPtr = ALLOCA(vSrc->getType()->getVectorElementType());

    Value *mask = MASK(vMask);
    for (uint32_t i = 0; i < JM()->mVWidth; ++i)
    {
        Value *offset = VEXTRACT(vOffsets, C(i));
        // byte pointer to component
        Value *storeAddress = GEP(pDst, offset);
        storeAddress = BITCAST(storeAddress, PointerType::get(mFP32Ty, 0));
        Value *selMask = VEXTRACT(mask, C(i));
        Value *srcElem = VEXTRACT(vSrc, C(i));
        // switch in a safe address to load if we're trying to access a vertex 
        Value *validAddress = SELECT(selMask, storeAddress, vTmpPtr);
        STORE(srcElem, validAddress);
    }

    STACKRESTORE(pStack);
}

Value* Builder::VABSPS(Value* a)
{
    Value* asInt = BITCAST(a, mSimdInt32Ty);
    Value* result = BITCAST(AND(asInt, VIMMED1(0x7fffffff)), mSimdFP32Ty);
    return result;
}

Value *Builder::ICLAMP(Value* src, Value* low, Value* high)
{
    Value *lowCmp = ICMP_SLT(src, low);
    Value *ret = SELECT(lowCmp, low, src);

    Value *highCmp = ICMP_SGT(ret, high);
    ret = SELECT(highCmp, high, ret);

    return ret;
}

Value *Builder::FCLAMP(Value* src, Value* low, Value* high)
{
    Value *lowCmp = FCMP_OLT(src, low);
    Value *ret = SELECT(lowCmp, low, src);

    Value *highCmp = FCMP_OGT(ret, high);
    ret = SELECT(highCmp, high, ret);

    return ret;
}

Value *Builder::FCLAMP(Value* src, float low, float high)
{
    Value* result = VMAXPS(src, VIMMED1(low));
    result = VMINPS(result, VIMMED1(high));

    return result;
}

//////////////////////////////////////////////////////////////////////////
/// @brief save/restore stack, providing ability to push/pop the stack and 
///        reduce overall stack requirements for temporary stack use
Value* Builder::STACKSAVE()
{
    Function* pfnStackSave = Intrinsic::getDeclaration(JM()->mpCurrentModule, Intrinsic::stacksave);
    return CALL(pfnStackSave);
}

void Builder::STACKRESTORE(Value* pSaved)
{
    Function* pfnStackRestore = Intrinsic::getDeclaration(JM()->mpCurrentModule, Intrinsic::stackrestore);
    CALL(pfnStackRestore, pSaved);
}

Value *Builder::FMADDPS(Value* a, Value* b, Value* c)
{
    Value* vOut;
    // use FMADs if available
    if(JM()->mArch.AVX2())
    {
        vOut = VFMADDPS(a, b, c);
    }
    else
    {
        vOut = FADD(FMUL(a, b), c);
    }
    return vOut;
}

Value* Builder::POPCNT(Value* a)
{
    Function* pCtPop = Intrinsic::getDeclaration(JM()->mpCurrentModule, Intrinsic::ctpop, { a->getType() });
    return CALL(pCtPop, a);
}

//////////////////////////////////////////////////////////////////////////
/// @brief C functions called by LLVM IR
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// @brief called in JIT code, inserted by PRINT
/// output to both stdout and visual studio debug console
void __cdecl CallPrint(const char* fmt, ...)
{
#if defined( DEBUG ) || defined( _DEBUG )
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);

#if defined( _WIN32 )
    char strBuf[1024];
    vsnprintf_s(strBuf, _TRUNCATE, fmt, args);
    OutputDebugString(strBuf);
#endif
#endif // #if defined( DEBUG ) || defined( _DEBUG )
}
