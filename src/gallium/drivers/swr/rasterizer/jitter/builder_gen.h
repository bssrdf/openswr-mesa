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
* @file builder_gen.h
* 
* @brief auto-generated file
* 
* DO NOT EDIT
* 
******************************************************************************/

#pragma once

//////////////////////////////////////////////////////////////////////////
/// Auto-generated Builder IR declarations
//////////////////////////////////////////////////////////////////////////
Value *GLOBAL_STRING(StringRef Str, const Twine &Name = "");
CallInst *MEMSET(Value *Ptr, Value *Val, uint64_t Size, unsigned Align, bool isVolatile = false, MDNode *TBAATag = nullptr, MDNode *ScopeTag = nullptr, MDNode *NoAliasTag = nullptr);
CallInst *MEMSET(Value *Ptr, Value *Val, Value *Size, unsigned Align, bool isVolatile = false, MDNode *TBAATag = nullptr, MDNode *ScopeTag = nullptr, MDNode *NoAliasTag = nullptr);
CallInst *MEMCPY(Value *Dst, Value *Src, uint64_t Size, unsigned Align, bool isVolatile = false, MDNode *TBAATag = nullptr, MDNode *TBAAStructTag = nullptr, MDNode *ScopeTag = nullptr, MDNode *NoAliasTag = nullptr);
CallInst *MEMCPY(Value *Dst, Value *Src, Value *Size, unsigned Align, bool isVolatile = false, MDNode *TBAATag = nullptr, MDNode *TBAAStructTag = nullptr, MDNode *ScopeTag = nullptr, MDNode *NoAliasTag = nullptr);
CallInst *MEMMOVE(Value *Dst, Value *Src, uint64_t Size, unsigned Align, bool isVolatile = false, MDNode *TBAATag = nullptr, MDNode *ScopeTag = nullptr, MDNode *NoAliasTag = nullptr);
CallInst *MEMMOVE(Value *Dst, Value *Src, Value *Size, unsigned Align, bool isVolatile = false, MDNode *TBAATag = nullptr, MDNode *ScopeTag = nullptr, MDNode *NoAliasTag = nullptr);
CallInst *LIFETIME_START(Value *Ptr, ConstantInt *Size = nullptr);
CallInst *LIFETIME_END(Value *Ptr, ConstantInt *Size = nullptr);
CallInst *MASKED_LOAD(Value *Ptr, unsigned Align, Value *Mask, Value *PassThru = 0, const Twine &Name = "");
CallInst *MASKED_STORE(Value *Val, Value *Ptr, unsigned Align, Value *Mask);
CallInst *ASSUMPTION(Value *Cond);
CallInst *GC_STATEPOINT(Value *ActualCallee, ArrayRef<Value*> CallArgs, ArrayRef<Value*> DeoptArgs, ArrayRef<Value*> GCArgs, const Twine &Name = "");
CallInst *GC_RESULT(Instruction *Statepoint, Type *ResultType, const Twine &Name = "");
CallInst *GC_RELOCATE(Instruction *Statepoint, int BaseOffset, int DerivedOffset, Type *ResultType, const Twine &Name = "");
ReturnInst *RET_VOID();
ReturnInst *RET(Value *V);
ReturnInst *AGGREGATE_RET(Value *const *retVals, unsigned N);
BranchInst *BR(BasicBlock *Dest);
BranchInst *COND_BR(Value *Cond, BasicBlock *True, BasicBlock *False, MDNode *BranchWeights = nullptr);
SwitchInst *SWITCH(Value *V, BasicBlock *Dest, unsigned NumCases = 10, MDNode *BranchWeights = nullptr);
IndirectBrInst *INDIRECT_BR(Value *Addr, unsigned NumDests = 10);
InvokeInst *INVOKE(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, const Twine &Name = "");
InvokeInst *INVOKE(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, Value *Arg1, const Twine &Name = "");
InvokeInst *INVOKE3(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, Value *Arg1, Value *Arg2, Value *Arg3, const Twine &Name = "");
InvokeInst *INVOKE(Value *Callee, BasicBlock *NormalDest, BasicBlock *UnwindDest, ArrayRef<Value *> Args, const Twine &Name = "");
ResumeInst *RESUME(Value *Exn);
UnreachableInst *UNREACHABLE();
Value *ADD(Value *LHS, Value *RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *NSW_ADD(Value *LHS, Value *RHS, const Twine &Name = "");
Value *NUW_ADD(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FADD(Value *LHS, Value *RHS, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *SUB(Value *LHS, Value *RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *NSW_SUB(Value *LHS, Value *RHS, const Twine &Name = "");
Value *NUW_SUB(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FSUB(Value *LHS, Value *RHS, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *MUL(Value *LHS, Value *RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *NSW_MUL(Value *LHS, Value *RHS, const Twine &Name = "");
Value *NUW_MUL(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FMUL(Value *LHS, Value *RHS, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *UDIV(Value *LHS, Value *RHS, const Twine &Name = "", bool isExact = false);
Value *EXACT_U_DIV(Value *LHS, Value *RHS, const Twine &Name = "");
Value *SDIV(Value *LHS, Value *RHS, const Twine &Name = "", bool isExact = false);
Value *EXACT_S_DIV(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FDIV(Value *LHS, Value *RHS, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *UREM(Value *LHS, Value *RHS, const Twine &Name = "");
Value *SREM(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FREM(Value *LHS, Value *RHS, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *SHL(Value *LHS, Value *RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *SHL(Value *LHS, const APInt &RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *SHL(Value *LHS, uint64_t RHS, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *LSHR(Value *LHS, Value *RHS, const Twine &Name = "", bool isExact = false);
Value *LSHR(Value *LHS, const APInt &RHS, const Twine &Name = "", bool isExact = false);
Value *LSHR(Value *LHS, uint64_t RHS, const Twine &Name = "", bool isExact = false);
Value *ASHR(Value *LHS, Value *RHS, const Twine &Name = "", bool isExact = false);
Value *ASHR(Value *LHS, const APInt &RHS, const Twine &Name = "", bool isExact = false);
Value *ASHR(Value *LHS, uint64_t RHS, const Twine &Name = "", bool isExact = false);
Value *AND(Value *LHS, Value *RHS, const Twine &Name = "");
Value *AND(Value *LHS, const APInt &RHS, const Twine &Name = "");
Value *AND(Value *LHS, uint64_t RHS, const Twine &Name = "");
Value *OR(Value *LHS, Value *RHS, const Twine &Name = "");
Value *OR(Value *LHS, const APInt &RHS, const Twine &Name = "");
Value *OR(Value *LHS, uint64_t RHS, const Twine &Name = "");
Value *XOR(Value *LHS, Value *RHS, const Twine &Name = "");
Value *XOR(Value *LHS, const APInt &RHS, const Twine &Name = "");
Value *XOR(Value *LHS, uint64_t RHS, const Twine &Name = "");
Value *BINOP(Instruction::BinaryOps Opc, Value *LHS, Value *RHS, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *NEG(Value *V, const Twine &Name = "", bool HasNUW = false, bool HasNSW = false);
Value *NSW_NEG(Value *V, const Twine &Name = "");
Value *NUW_NEG(Value *V, const Twine &Name = "");
Value *FNEG(Value *V, const Twine &Name = "", MDNode *FPMathTag = nullptr);
Value *NOT(Value *V, const Twine &Name = "");
AllocaInst *ALLOCA(Type *Ty, Value *ArraySize = nullptr, const Twine &Name = "");
LoadInst *LOAD(Value *Ptr, const char *Name);
LoadInst *LOAD(Value *Ptr, const Twine &Name = "");
LoadInst *LOAD(Value *Ptr, bool isVolatile, const Twine &Name = "");
StoreInst *STORE(Value *Val, Value *Ptr, bool isVolatile = false);
LoadInst *ALIGNED_LOAD(Value *Ptr, unsigned Align, const char *Name);
LoadInst *ALIGNED_LOAD(Value *Ptr, unsigned Align, const Twine &Name = "");
LoadInst *ALIGNED_LOAD(Value *Ptr, unsigned Align, bool isVolatile, const Twine &Name = "");
StoreInst *ALIGNED_STORE(Value *Val, Value *Ptr, unsigned Align, bool isVolatile = false);
FenceInst *FENCE(AtomicOrdering Ordering, SynchronizationScope SynchScope = CrossThread, const Twine &Name = "");
AtomicCmpXchgInst *ATOMIC_CMP_XCHG(Value *Ptr, Value *Cmp, Value *New, AtomicOrdering SuccessOrdering, AtomicOrdering FailureOrdering, SynchronizationScope SynchScope = CrossThread);
AtomicRMWInst *ATOMIC_RMW(AtomicRMWInst::BinOp Op, Value *Ptr, Value *Val, AtomicOrdering Ordering, SynchronizationScope SynchScope = CrossThread);
Value *GEPA(Value *Ptr, ArrayRef<Value *> IdxList, const Twine &Name = "");
Value *IN_BOUNDS_GEP(Value *Ptr, ArrayRef<Value *> IdxList, const Twine &Name = "");
Value *GEP(Value *Ptr, Value *Idx, const Twine &Name = "");
Value *IN_BOUNDS_GEP(Value *Ptr, Value *Idx, const Twine &Name = "");
Value *CONST_GEP1_32(Value *Ptr, unsigned Idx0, const Twine &Name = "");
Value *CONST_IN_BOUNDS_GEP1_32(Value *Ptr, unsigned Idx0, const Twine &Name = "");
Value *CONST_GEP2_32(Value *Ptr, unsigned Idx0, unsigned Idx1, const Twine &Name = "");
Value *CONST_IN_BOUNDS_GEP2_32(Value *Ptr, unsigned Idx0, unsigned Idx1, const Twine &Name = "");
Value *CONST_GEP1_64(Value *Ptr, uint64_t Idx0, const Twine &Name = "");
Value *CONST_IN_BOUNDS_GEP1_64(Value *Ptr, uint64_t Idx0, const Twine &Name = "");
Value *CONST_GEP2_64(Value *Ptr, uint64_t Idx0, uint64_t Idx1, const Twine &Name = "");
Value *CONST_IN_BOUNDS_GEP2_64(Value *Ptr, uint64_t Idx0, uint64_t Idx1, const Twine &Name = "");
Value *STRUCT_GEP(Value *Ptr, unsigned Idx, const Twine &Name = "");
Value *GLOBAL_STRING_PTR(StringRef Str, const Twine &Name = "");
Value *TRUNC(Value *V, Type *DestTy, const Twine &Name = "");
Value *Z_EXT(Value *V, Type *DestTy, const Twine &Name = "");
Value *S_EXT(Value *V, Type *DestTy, const Twine &Name = "");
Value *Z_EXT_OR_TRUNC(Value *V, Type *DestTy, const Twine &Name = "");
Value *S_EXT_OR_TRUNC(Value *V, Type *DestTy, const Twine &Name = "");
Value *FP_TO_UI(Value *V, Type *DestTy, const Twine &Name = "");
Value *FP_TO_SI(Value *V, Type *DestTy, const Twine &Name = "");
Value *UI_TO_FP(Value *V, Type *DestTy, const Twine &Name = "");
Value *SI_TO_FP(Value *V, Type *DestTy, const Twine &Name = "");
Value *FP_TRUNC(Value *V, Type *DestTy, const Twine &Name = "");
Value *FP_EXT(Value *V, Type *DestTy, const Twine &Name = "");
Value *PTR_TO_INT(Value *V, Type *DestTy, const Twine &Name = "");
Value *INT_TO_PTR(Value *V, Type *DestTy, const Twine &Name = "");
Value *BITCAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *ADDR_SPACE_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *Z_EXT_OR_BIT_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *S_EXT_OR_BIT_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *TRUNC_OR_BIT_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *CAST(Instruction::CastOps Op, Value *V, Type *DestTy, const Twine &Name = "");
Value *POINTER_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *POINTER_BIT_CAST_OR_ADDR_SPACE_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *INT_CAST(Value *V, Type *DestTy, bool isSigned, const Twine &Name = "");
Value *BIT_OR_POINTER_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *FP_CAST(Value *V, Type *DestTy, const Twine &Name = "");
Value *ICMP_EQ(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_NE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_UGT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_UGE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_ULT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_ULE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_SGT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_SGE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_SLT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP_SLE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_OEQ(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_OGT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_OGE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_OLT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_OLE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_ONE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_ORD(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_UNO(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_UEQ(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_UGT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_UGE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_ULT(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_ULE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP_UNE(Value *LHS, Value *RHS, const Twine &Name = "");
Value *ICMP(CmpInst::Predicate P, Value *LHS, Value *RHS, const Twine &Name = "");
Value *FCMP(CmpInst::Predicate P, Value *LHS, Value *RHS, const Twine &Name = "");
PHINode *PHI(Type *Ty, unsigned NumReservedValues, const Twine &Name = "");
CallInst *CALL(Value *Callee, const Twine &Name = "");
CallInst *CALL(Value *Callee, Value *Arg, const Twine &Name = "");
CallInst *CALL2(Value *Callee, Value *Arg1, Value *Arg2, const Twine &Name = "");
CallInst *CALL3(Value *Callee, Value *Arg1, Value *Arg2, Value *Arg3, const Twine &Name = "");
CallInst *CALL4(Value *Callee, Value *Arg1, Value *Arg2, Value *Arg3, Value *Arg4, const Twine &Name = "");
CallInst *CALL5(Value *Callee, Value *Arg1, Value *Arg2, Value *Arg3, Value *Arg4, Value *Arg5, const Twine &Name = "");
CallInst *CALLA(Value *Callee, ArrayRef<Value *> Args, const Twine &Name = "");
Value *SELECT(Value *C, Value *True, Value *False, const Twine &Name = "");
VAArgInst *VA_ARG(Value *List, Type *Ty, const Twine &Name = "");
Value *VEXTRACT(Value *Vec, Value *Idx, const Twine &Name = "");
Value *VINSERT(Value *Vec, Value *NewElt, Value *Idx, const Twine &Name = "");
Value *VSHUFFLE(Value *V1, Value *V2, Value *Mask, const Twine &Name = "");
Value *EXTRACT_VALUE(Value *Agg, ArrayRef<unsigned> Idxs, const Twine &Name = "");
Value *INSERT_VALUE(Value *Agg, Value *Val, ArrayRef<unsigned> Idxs, const Twine &Name = "");
LandingPadInst *LANDING_PAD(Type *Ty, Value *PersFn, unsigned NumClauses, const Twine &Name = "");
Value *IS_NULL(Value *Arg, const Twine &Name = "");
Value *IS_NOT_NULL(Value *Arg, const Twine &Name = "");
Value *PTR_DIFF(Value *LHS, Value *RHS, const Twine &Name = "");
Value *VECTOR_SPLAT(unsigned NumElts, Value *V, const Twine &Name = "");
Value *EXTRACT_INTEGER(const DataLayout &DL, Value *From, IntegerType *ExtractedTy, uint64_t Offset, const Twine &Name);
CallInst *ALIGNMENT_ASSUMPTION(const DataLayout &DL, Value *PtrValue, unsigned Alignment, Value *OffsetValue = nullptr);
