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
* @file builder_gen.cpp
* 
* @brief auto-generated file
* 
* DO NOT EDIT
* 
******************************************************************************/

#include "builder.h"

//////////////////////////////////////////////////////////////////////////
Value *Builder::GLOBAL_STRING(StringRef Str, const Twine &Name)
{
   return IRB()->CreateGlobalString(Str, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MEMSET(Value *Ptr, Value *Val, uint64_t Size, unsigned Align, bool isVolatile, MDNode *TBAATag, MDNode *ScopeTag, MDNode *NoAliasTag)
{
   return IRB()->CreateMemSet(Ptr, Val, Size, Align, isVolatile, TBAATag, ScopeTag, NoAliasTag);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MEMSET(Value *Ptr, Value *Val, Value *Size, unsigned Align, bool isVolatile, MDNode *TBAATag, MDNode *ScopeTag, MDNode *NoAliasTag)
{
   return IRB()->CreateMemSet(Ptr, Val, Size, Align, isVolatile, TBAATag, ScopeTag, NoAliasTag);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MEMCPY(Value *Dst, Value *Src, uint64_t Size, unsigned Align, bool isVolatile, MDNode *TBAATag, MDNode *TBAAStructTag, MDNode *ScopeTag, MDNode *NoAliasTag)
{
   return IRB()->CreateMemCpy(Dst, Src, Size, Align, isVolatile, TBAATag, TBAAStructTag, ScopeTag, NoAliasTag);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MEMCPY(Value *Dst, Value *Src, Value *Size, unsigned Align, bool isVolatile, MDNode *TBAATag, MDNode *TBAAStructTag, MDNode *ScopeTag, MDNode *NoAliasTag)
{
   return IRB()->CreateMemCpy(Dst, Src, Size, Align, isVolatile, TBAATag, TBAAStructTag, ScopeTag, NoAliasTag);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MEMMOVE(Value *Dst, Value *Src, uint64_t Size, unsigned Align, bool isVolatile, MDNode *TBAATag, MDNode *ScopeTag, MDNode *NoAliasTag)
{
   return IRB()->CreateMemMove(Dst, Src, Size, Align, isVolatile, TBAATag, ScopeTag, NoAliasTag);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MEMMOVE(Value *Dst, Value *Src, Value *Size, unsigned Align, bool isVolatile, MDNode *TBAATag, MDNode *ScopeTag, MDNode *NoAliasTag)
{
   return IRB()->CreateMemMove(Dst, Src, Size, Align, isVolatile, TBAATag, ScopeTag, NoAliasTag);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::LIFETIME_START(Value *Ptr, ConstantInt *Size)
{
   return IRB()->CreateLifetimeStart(Ptr, Size);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::LIFETIME_END(Value *Ptr, ConstantInt *Size)
{
   return IRB()->CreateLifetimeEnd(Ptr, Size);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MASKED_LOAD(Value *Ptr, unsigned Align, Value *Mask, Value *PassThru, const Twine &Name)
{
   return IRB()->CreateMaskedLoad(Ptr, Align, Mask, PassThru, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::MASKED_STORE(Value *Val, Value *Ptr, unsigned Align, Value *Mask)
{
   return IRB()->CreateMaskedStore(Val, Ptr, Align, Mask);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::ASSUMPTION(Value *Cond)
{
   return IRB()->CreateAssumption(Cond);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::GC_STATEPOINT(Value *ActualCallee, ArrayRef<Value*> CallArgs, ArrayRef<Value*> DeoptArgs, ArrayRef<Value*> GCArgs, const Twine &Name)
{
   return IRB()->CreateGCStatepoint(ActualCallee, CallArgs, DeoptArgs, GCArgs, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::GC_RESULT(Instruction *Statepoint, Type *ResultType, const Twine &Name)
{
   return IRB()->CreateGCResult(Statepoint, ResultType, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::GC_RELOCATE(Instruction *Statepoint, int BaseOffset, int DerivedOffset, Type *ResultType, const Twine &Name)
{
   return IRB()->CreateGCRelocate(Statepoint, BaseOffset, DerivedOffset, ResultType, Name);
}

//////////////////////////////////////////////////////////////////////////
ReturnInst *Builder::RET_VOID()
{
   return IRB()->CreateRetVoid();
}

//////////////////////////////////////////////////////////////////////////
ReturnInst *Builder::RET(Value *V)
{
   return IRB()->CreateRet(V);
}

//////////////////////////////////////////////////////////////////////////
ReturnInst *Builder::AGGREGATE_RET(Value *const *retVals, unsigned N)
{
   return IRB()->CreateAggregateRet(retVals, N);
}

//////////////////////////////////////////////////////////////////////////
BranchInst *Builder::BR(BasicBlock *Dest)
{
   return IRB()->CreateBr(Dest);
}

//////////////////////////////////////////////////////////////////////////
BranchInst *Builder::COND_BR(Value *Cond, BasicBlock *True, BasicBlock *False, MDNode *BranchWeights)
{
   return IRB()->CreateCondBr(Cond, True, False, BranchWeights);
}

//////////////////////////////////////////////////////////////////////////
SwitchInst *Builder::SWITCH(Value *V, BasicBlock *Dest, unsigned NumCases, MDNode *BranchWeights)
{
   return IRB()->CreateSwitch(V, Dest, NumCases, BranchWeights);
}

//////////////////////////////////////////////////////////////////////////
IndirectBrInst *Builder::INDIRECT_BR(Value *Addr, unsigned NumDests)
{
   return IRB()->CreateIndirectBr(Addr, NumDests);
}

//////////////////////////////////////////////////////////////////////////
InvokeInst *Builder::INVOKE(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, const Twine &Name)
{
   return IRB()->CreateInvoke(Callee, NormalDest, UnwindDest, Name);
}

//////////////////////////////////////////////////////////////////////////
InvokeInst *Builder::INVOKE(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, Value *Arg1, const Twine &Name)
{
   return IRB()->CreateInvoke(Callee, NormalDest, UnwindDest, Arg1, Name);
}

//////////////////////////////////////////////////////////////////////////
InvokeInst *Builder::INVOKE3(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, Value *Arg1, Value *Arg2, Value *Arg3, const Twine &Name)
{
   return IRB()->CreateInvoke3(Callee, NormalDest, UnwindDest, Arg1, Arg2, Arg3, Name);
}

//////////////////////////////////////////////////////////////////////////
InvokeInst *Builder::INVOKE(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, ArrayRef<Value *> Args, const Twine &Name)
{
   return IRB()->CreateInvoke(Callee, NormalDest, UnwindDest, Args, Name);
}

//////////////////////////////////////////////////////////////////////////
ResumeInst *Builder::RESUME(Value *Exn)
{
   return IRB()->CreateResume(Exn);
}

//////////////////////////////////////////////////////////////////////////
UnreachableInst *Builder::UNREACHABLE()
{
   return IRB()->CreateUnreachable();
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ADD(Value *LHS, Value *RHS, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateAdd(LHS, RHS, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NSW_ADD(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateNSWAdd(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NUW_ADD(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateNUWAdd(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FADD(Value *LHS, Value *RHS, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateFAdd(LHS, RHS, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SUB(Value *LHS, Value *RHS, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateSub(LHS, RHS, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NSW_SUB(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateNSWSub(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NUW_SUB(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateNUWSub(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FSUB(Value *LHS, Value *RHS, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateFSub(LHS, RHS, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::MUL(Value *LHS, Value *RHS, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateMul(LHS, RHS, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NSW_MUL(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateNSWMul(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NUW_MUL(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateNUWMul(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FMUL(Value *LHS, Value *RHS, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateFMul(LHS, RHS, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::UDIV(Value *LHS, Value *RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateUDiv(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::EXACT_U_DIV(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateExactUDiv(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SDIV(Value *LHS, Value *RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateSDiv(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::EXACT_S_DIV(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateExactSDiv(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FDIV(Value *LHS, Value *RHS, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateFDiv(LHS, RHS, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::UREM(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateURem(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SREM(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateSRem(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FREM(Value *LHS, Value *RHS, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateFRem(LHS, RHS, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SHL(Value *LHS, Value *RHS, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateShl(LHS, RHS, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SHL(Value *LHS, const APInt &RHS, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateShl(LHS, RHS, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SHL(Value *LHS, uint64_t RHS, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateShl(LHS, RHS, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::LSHR(Value *LHS, Value *RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateLShr(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::LSHR(Value *LHS, const APInt &RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateLShr(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::LSHR(Value *LHS, uint64_t RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateLShr(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ASHR(Value *LHS, Value *RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateAShr(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ASHR(Value *LHS, const APInt &RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateAShr(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ASHR(Value *LHS, uint64_t RHS, const Twine &Name, bool isExact)
{
   return IRB()->CreateAShr(LHS, RHS, Name, isExact);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::AND(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateAnd(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::AND(Value *LHS, const APInt &RHS, const Twine &Name)
{
   return IRB()->CreateAnd(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::AND(Value *LHS, uint64_t RHS, const Twine &Name)
{
   return IRB()->CreateAnd(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::OR(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateOr(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::OR(Value *LHS, const APInt &RHS, const Twine &Name)
{
   return IRB()->CreateOr(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::OR(Value *LHS, uint64_t RHS, const Twine &Name)
{
   return IRB()->CreateOr(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::XOR(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateXor(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::XOR(Value *LHS, const APInt &RHS, const Twine &Name)
{
   return IRB()->CreateXor(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::XOR(Value *LHS, uint64_t RHS, const Twine &Name)
{
   return IRB()->CreateXor(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::BINOP(Instruction::BinaryOps Opc, Value *LHS, Value *RHS, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateBinOp(Opc, LHS, RHS, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NEG(Value *V, const Twine &Name, bool HasNUW, bool HasNSW)
{
   return IRB()->CreateNeg(V, Name, HasNUW, HasNSW);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NSW_NEG(Value *V, const Twine &Name)
{
   return IRB()->CreateNSWNeg(V, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NUW_NEG(Value *V, const Twine &Name)
{
   return IRB()->CreateNUWNeg(V, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FNEG(Value *V, const Twine &Name, MDNode *FPMathTag)
{
   return IRB()->CreateFNeg(V, Name, FPMathTag);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::NOT(Value *V, const Twine &Name)
{
   return IRB()->CreateNot(V, Name);
}

//////////////////////////////////////////////////////////////////////////
AllocaInst *Builder::ALLOCA(Type *Ty, Value *ArraySize, const Twine &Name)
{
   return IRB()->CreateAlloca(Ty, ArraySize, Name);
}

//////////////////////////////////////////////////////////////////////////
LoadInst *Builder::LOAD(Value *Ptr, const char *Name)
{
   return IRB()->CreateLoad(Ptr, Name);
}

//////////////////////////////////////////////////////////////////////////
LoadInst *Builder::LOAD(Value *Ptr, const Twine &Name)
{
   return IRB()->CreateLoad(Ptr, Name);
}

//////////////////////////////////////////////////////////////////////////
LoadInst *Builder::LOAD(Value *Ptr, bool isVolatile, const Twine &Name)
{
   return IRB()->CreateLoad(Ptr, isVolatile, Name);
}

//////////////////////////////////////////////////////////////////////////
StoreInst *Builder::STORE(Value *Val, Value *Ptr, bool isVolatile)
{
   return IRB()->CreateStore(Val, Ptr, isVolatile);
}

//////////////////////////////////////////////////////////////////////////
LoadInst *Builder::ALIGNED_LOAD(Value *Ptr, unsigned Align, const char *Name)
{
   return IRB()->CreateAlignedLoad(Ptr, Align, Name);
}

//////////////////////////////////////////////////////////////////////////
LoadInst *Builder::ALIGNED_LOAD(Value *Ptr, unsigned Align, const Twine &Name)
{
   return IRB()->CreateAlignedLoad(Ptr, Align, Name);
}

//////////////////////////////////////////////////////////////////////////
LoadInst *Builder::ALIGNED_LOAD(Value *Ptr, unsigned Align, bool isVolatile, const Twine &Name)
{
   return IRB()->CreateAlignedLoad(Ptr, Align, isVolatile, Name);
}

//////////////////////////////////////////////////////////////////////////
StoreInst *Builder::ALIGNED_STORE(Value *Val, Value *Ptr, unsigned Align, bool isVolatile)
{
   return IRB()->CreateAlignedStore(Val, Ptr, Align, isVolatile);
}

//////////////////////////////////////////////////////////////////////////
FenceInst *Builder::FENCE(AtomicOrdering Ordering, SynchronizationScope SynchScope, const Twine &Name)
{
   return IRB()->CreateFence(Ordering, SynchScope, Name);
}

//////////////////////////////////////////////////////////////////////////
AtomicCmpXchgInst *Builder::ATOMIC_CMP_XCHG(Value *Ptr, Value *Cmp, Value *New, AtomicOrdering SuccessOrdering, AtomicOrdering FailureOrdering, SynchronizationScope SynchScope)
{
   return IRB()->CreateAtomicCmpXchg(Ptr, Cmp, New, SuccessOrdering, FailureOrdering, SynchScope);
}

//////////////////////////////////////////////////////////////////////////
AtomicRMWInst *Builder::ATOMIC_RMW(AtomicRMWInst::BinOp Op, Value *Ptr, Value *Val, AtomicOrdering Ordering, SynchronizationScope SynchScope)
{
   return IRB()->CreateAtomicRMW(Op, Ptr, Val, Ordering, SynchScope);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::GEPA(Value *Ptr, ArrayRef<Value *> IdxList, const Twine &Name)
{
   return IRB()->CreateGEP(Ptr, IdxList, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::IN_BOUNDS_GEP(Value *Ptr, ArrayRef<Value *> IdxList, const Twine &Name)
{
   return IRB()->CreateInBoundsGEP(Ptr, IdxList, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::GEP(Value *Ptr, Value *Idx, const Twine &Name)
{
   return IRB()->CreateGEP(Ptr, Idx, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::IN_BOUNDS_GEP(Value *Ptr, Value *Idx, const Twine &Name)
{
   return IRB()->CreateInBoundsGEP(Ptr, Idx, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_GEP1_32(Value *Ptr, unsigned Idx0, const Twine &Name)
{
   return IRB()->CreateConstGEP1_32(Ptr, Idx0, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_IN_BOUNDS_GEP1_32(Value *Ptr, unsigned Idx0, const Twine &Name)
{
   return IRB()->CreateConstInBoundsGEP1_32(Ptr, Idx0, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_GEP2_32(Value *Ptr, unsigned Idx0, unsigned Idx1, const Twine &Name)
{
   return IRB()->CreateConstGEP2_32(Ptr, Idx0, Idx1, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_IN_BOUNDS_GEP2_32(Value *Ptr, unsigned Idx0, unsigned Idx1, const Twine &Name)
{
   return IRB()->CreateConstInBoundsGEP2_32(Ptr, Idx0, Idx1, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_GEP1_64(Value *Ptr, uint64_t Idx0, const Twine &Name)
{
   return IRB()->CreateConstGEP1_64(Ptr, Idx0, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_IN_BOUNDS_GEP1_64(Value *Ptr, uint64_t Idx0, const Twine &Name)
{
   return IRB()->CreateConstInBoundsGEP1_64(Ptr, Idx0, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_GEP2_64(Value *Ptr, uint64_t Idx0, uint64_t Idx1, const Twine &Name)
{
   return IRB()->CreateConstGEP2_64(Ptr, Idx0, Idx1, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CONST_IN_BOUNDS_GEP2_64(Value *Ptr, uint64_t Idx0, uint64_t Idx1, const Twine &Name)
{
   return IRB()->CreateConstInBoundsGEP2_64(Ptr, Idx0, Idx1, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::STRUCT_GEP(Value *Ptr, unsigned Idx, const Twine &Name)
{
   return IRB()->CreateStructGEP(Ptr, Idx, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::GLOBAL_STRING_PTR(StringRef Str, const Twine &Name)
{
   return IRB()->CreateGlobalStringPtr(Str, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::TRUNC(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateTrunc(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::Z_EXT(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateZExt(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::S_EXT(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateSExt(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::Z_EXT_OR_TRUNC(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateZExtOrTrunc(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::S_EXT_OR_TRUNC(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateSExtOrTrunc(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FP_TO_UI(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateFPToUI(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FP_TO_SI(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateFPToSI(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::UI_TO_FP(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateUIToFP(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SI_TO_FP(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateSIToFP(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FP_TRUNC(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateFPTrunc(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FP_EXT(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateFPExt(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::PTR_TO_INT(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreatePtrToInt(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::INT_TO_PTR(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateIntToPtr(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::BITCAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateBitCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ADDR_SPACE_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateAddrSpaceCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::Z_EXT_OR_BIT_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateZExtOrBitCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::S_EXT_OR_BIT_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateSExtOrBitCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::TRUNC_OR_BIT_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateTruncOrBitCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::CAST(Instruction::CastOps Op, Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateCast(Op, V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::POINTER_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreatePointerCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::POINTER_BIT_CAST_OR_ADDR_SPACE_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreatePointerBitCastOrAddrSpaceCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::INT_CAST(Value *V, Type *DestTy, bool isSigned, const Twine &Name)
{
   return IRB()->CreateIntCast(V, DestTy, isSigned, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::BIT_OR_POINTER_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateBitOrPointerCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FP_CAST(Value *V, Type *DestTy, const Twine &Name)
{
   return IRB()->CreateFPCast(V, DestTy, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_EQ(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpEQ(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_NE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpNE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_UGT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpUGT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_UGE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpUGE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_ULT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpULT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_ULE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpULE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_SGT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpSGT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_SGE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpSGE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_SLT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpSLT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP_SLE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmpSLE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_OEQ(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpOEQ(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_OGT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpOGT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_OGE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpOGE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_OLT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpOLT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_OLE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpOLE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_ONE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpONE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_ORD(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpORD(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_UNO(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpUNO(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_UEQ(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpUEQ(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_UGT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpUGT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_UGE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpUGE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_ULT(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpULT(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_ULE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpULE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP_UNE(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmpUNE(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::ICMP(CmpInst::Predicate P, Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateICmp(P, LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::FCMP(CmpInst::Predicate P, Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreateFCmp(P, LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
PHINode *Builder::PHI(Type *Ty, unsigned NumReservedValues, const Twine &Name)
{
   return IRB()->CreatePHI(Ty, NumReservedValues, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALL(Value *Callee, const Twine &Name)
{
   return IRB()->CreateCall(Callee, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALL(Value *Callee, Value *Arg, const Twine &Name)
{
   return IRB()->CreateCall(Callee, Arg, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALL2(Value *Callee, Value *Arg1, Value *Arg2, const Twine &Name)
{
   return IRB()->CreateCall2(Callee, Arg1, Arg2, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALL3(Value *Callee, Value *Arg1, Value *Arg2, Value *Arg3, const Twine &Name)
{
   return IRB()->CreateCall3(Callee, Arg1, Arg2, Arg3, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALL4(Value *Callee, Value *Arg1, Value *Arg2, Value *Arg3, Value *Arg4, const Twine &Name)
{
   return IRB()->CreateCall4(Callee, Arg1, Arg2, Arg3, Arg4, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALL5(Value *Callee, Value *Arg1, Value *Arg2, Value *Arg3, Value *Arg4, Value *Arg5, const Twine &Name)
{
   return IRB()->CreateCall5(Callee, Arg1, Arg2, Arg3, Arg4, Arg5, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::CALLA(Value *Callee, ArrayRef<Value *> Args, const Twine &Name)
{
   return IRB()->CreateCall(Callee, Args, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::SELECT(Value *C, Value *True, Value *False, const Twine &Name)
{
   return IRB()->CreateSelect(C, True, False, Name);
}

//////////////////////////////////////////////////////////////////////////
VAArgInst *Builder::VA_ARG(Value *List, Type *Ty, const Twine &Name)
{
   return IRB()->CreateVAArg(List, Ty, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::VEXTRACT(Value *Vec, Value *Idx, const Twine &Name)
{
   return IRB()->CreateExtractElement(Vec, Idx, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::VINSERT(Value *Vec, Value *NewElt, Value *Idx, const Twine &Name)
{
   return IRB()->CreateInsertElement(Vec, NewElt, Idx, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::VSHUFFLE(Value *V1, Value *V2, Value *Mask, const Twine &Name)
{
   return IRB()->CreateShuffleVector(V1, V2, Mask, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::EXTRACT_VALUE(Value *Agg, ArrayRef<unsigned> Idxs, const Twine &Name)
{
   return IRB()->CreateExtractValue(Agg, Idxs, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::INSERT_VALUE(Value *Agg, Value *Val, ArrayRef<unsigned> Idxs, const Twine &Name)
{
   return IRB()->CreateInsertValue(Agg, Val, Idxs, Name);
}

//////////////////////////////////////////////////////////////////////////
LandingPadInst *Builder::LANDING_PAD(Type *Ty, Value *PersFn, unsigned NumClauses, const Twine &Name)
{
   return IRB()->CreateLandingPad(Ty, PersFn, NumClauses, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::IS_NULL(Value *Arg, const Twine &Name)
{
   return IRB()->CreateIsNull(Arg, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::IS_NOT_NULL(Value *Arg, const Twine &Name)
{
   return IRB()->CreateIsNotNull(Arg, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::PTR_DIFF(Value *LHS, Value *RHS, const Twine &Name)
{
   return IRB()->CreatePtrDiff(LHS, RHS, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::VECTOR_SPLAT(unsigned NumElts, Value *V, const Twine &Name)
{
   return IRB()->CreateVectorSplat(NumElts, V, Name);
}

//////////////////////////////////////////////////////////////////////////
Value *Builder::EXTRACT_INTEGER(const DataLayout &DL, Value *From, IntegerType *ExtractedTy, uint64_t Offset, const Twine &Name)
{
   return IRB()->CreateExtractInteger(DL, From, ExtractedTy, Offset, Name);
}

//////////////////////////////////////////////////////////////////////////
CallInst *Builder::ALIGNMENT_ASSUMPTION(const DataLayout &DL, Value *PtrValue, unsigned Alignment, Value *OffsetValue)
{
   return IRB()->CreateAlignmentAssumption(DL, PtrValue, Alignment, OffsetValue);
}

