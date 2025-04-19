#ifndef CTXHTFUZZ
#define CTXHTFUZZ

#include "config.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/ADT/None.h"
#include "llvm/IR/Metadata.h"

void init_variables(llvm::Module &M);
void clear_first();
void clear_second();
void init_hp_op_ptr_set(llvm::Function &F);
void get_prev_xor_cur(llvm::IRBuilder<> &IRB, llvm::ConstantInt *CurLoc,
                      llvm::Value *PrevLocTrans);
void context_heap_operation_capture(llvm::Module &M, llvm::BasicBlock &BB,
                                    llvm::IRBuilder<>    &IRB,
                                    llvm::GlobalVariable *MapPtr);
#endif