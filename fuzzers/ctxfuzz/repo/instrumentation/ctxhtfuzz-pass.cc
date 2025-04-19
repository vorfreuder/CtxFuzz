#include <vector>
#include <string>
#include <map>
#include <set>
#include "ctxhtfuzz-pass.h"
std::vector<std::string> alloccall_routines = {
    "malloc",        "valloc",      "safe_malloc", "safemalloc",
    "safexmalloc",   "realloc",     "calloc",      "memalign",
    "aligned_alloc", "safe_calloc", "safecalloc",  "safexcalloc",
    "_Znwm",         "_Znam",       "_Znaj",       "_Znwj",
};
std::vector<std::string> dealloccall_routines = {
    "free",     "cfree",     "safe_cfree", "safe_free",
    "safefree", "safexfree", "_ZdaPv",     "_ZdlPv",
};
bool is_alloccall(std::string fn_name) {
  for (std::vector<std::string>::size_type i = 0; i < alloccall_routines.size();
       i++) {
    if (fn_name.compare(0, alloccall_routines[i].size(),
                        alloccall_routines[i]) == 0)
      return true;
  }
  return false;
}

bool is_dealloccall(std::string fn_name) {
  for (std::vector<std::string>::size_type i = 0;
       i < dealloccall_routines.size(); i++) {
    if (fn_name.compare(0, dealloccall_routines[i].size(),
                        dealloccall_routines[i]) == 0)
      return true;
  }
  return false;
}
// count something, such as the number of pointer
llvm::GlobalVariable *AFLCountPtr;
// two bits for per operation
// reflect the context heap operaton sequence before entering a basic block
llvm::GlobalVariable *AFLContextHpOpSeq;
// mask for AFLContextHpOpSeq
llvm::ConstantInt *ContextHpOpSeqMask;
// the inserted function call
llvm::Function *CTXHTFuzzUpdateContextInstr;
llvm::Function *CTXHTFuzzGetContextInstr;

void init_variables(llvm::Module &M) {
  llvm::IntegerType *Int8Ty = llvm::IntegerType::getInt8Ty(M.getContext());
  llvm::IntegerType *Int32Ty = llvm::IntegerType::getInt32Ty(M.getContext());
  AFLCountPtr = new llvm::GlobalVariable(
      M, llvm::PointerType::get(Int32Ty, 0), false,
      llvm::GlobalValue::ExternalLinkage, 0, "__afl_count_ptr");

  AFLContextHpOpSeq = new llvm::GlobalVariable(
      M, Int32Ty, false, llvm::GlobalValue::ExternalLinkage, 0,
      "__afl_context_hp_op_seq", 0,
      llvm::GlobalVariable::GeneralDynamicTLSModel, 0, false);

  ContextHpOpSeqMask =
      llvm::ConstantInt::get(Int32Ty, ((1 << (OP_NUM * 2)) - 1));
  llvm::outs() << "Context Heap Operation Sequence Mask: "
               << ContextHpOpSeqMask->getZExtValue() << "\n";

  // create function
  llvm::IRBuilder<>         builder(M.getContext());
  std::vector<llvm::Type *> paramTypes;
  paramTypes.push_back(builder.getInt8PtrTy());
  paramTypes.push_back(builder.getInt8Ty());
  llvm::ArrayRef<llvm::Type *> paramTypesRef(paramTypes);
  llvm::FunctionType          *FT =
      llvm::FunctionType::get(builder.getVoidTy(), paramTypesRef,
                              /*isVarArg=*/false);
  CTXHTFuzzUpdateContextInstr = llvm::Function::Create(
      FT, llvm::GlobalValue::ExternalLinkage, "CTXHTFuzzUpdateContext", &M);
  llvm::FunctionType *FT2 =
      llvm::FunctionType::get(builder.getVoidTy(), /*isVarArg=*/false);
  CTXHTFuzzGetContextInstr = llvm::Function::Create(
      FT2, llvm::GlobalValue::ExternalLinkage, "CTXHTFuzzGetContext", &M);
}

/*
a = getelementptr b, offset
origin_ptr[a]=b
*/
std::map<llvm::Value *, llvm::Value *> origin_ptr;
std::set<llvm::Instruction *>          instr_calls;
// the last location of heap object with different heap operation
std::map<std::pair<llvm::Value *, int>, llvm::Instruction *> last_location;
// heap operation related pointer set
std::set<llvm::Value *> hp_op_ptr_set;
void                    clear_first() {
  hp_op_ptr_set.clear();
  origin_ptr.clear();
}
void clear_second() {
  instr_calls.clear();
  last_location.clear();
}
void init_hp_op_ptr_set(llvm::Function &F) {
  for (llvm::Function::arg_iterator it = F.arg_begin(); it != F.arg_end();
       it++) {
    if (it->getType()->isPointerTy()) { hp_op_ptr_set.insert(it); }
  }
  // printf("hp_op_ptr_set size: %d\n", hp_op_ptr_set.size());
}
llvm::Value *Prev_xor_Cur;
void         get_prev_xor_cur(llvm::IRBuilder<> &IRB, llvm::ConstantInt *CurLoc,
                              llvm::Value *PrevLocTrans) {
  Prev_xor_Cur = IRB.CreateXor(PrevLocTrans, CurLoc);
}

void last_location_insert(llvm::Instruction *inst, int code) {
  if (code == ALLOC) {
    last_location[std::make_pair(inst, code)] = inst;
  } else if (code == LOAD) {
    if (origin_ptr.count(inst->getOperand(0)))
      last_location[std::make_pair(origin_ptr[inst->getOperand(0)], code)] =
          inst;
    else
      last_location[std::make_pair(inst->getOperand(0), code)] = inst;
  } else if (code == STORE) {
    if (origin_ptr.count(inst->getOperand(1)))
      last_location[std::make_pair(origin_ptr[inst->getOperand(1)], code)] =
          inst;
    else
      last_location[std::make_pair(inst->getOperand(1), code)] = inst;
  } else if (code == DEALLOC) {
    last_location[std::make_pair(inst->getOperand(0), code)] = inst;
  }
}
void insert_update_context_function(llvm::Instruction *inst,
                                    llvm::IntegerType *Int8Ty, int code,
                                    llvm::Function *instr_fun) {
  llvm::IRBuilder<>                   CTXHTFuzzIRBuilder(inst->getNextNode());
  llvm::SmallVector<llvm::Value *, 4> args;
  llvm::Value                        *arg = nullptr;
  if (code == ALLOC)
    arg = inst;
  else if (code == LOAD) {
    if (origin_ptr.count(inst->getOperand(0)))
      arg = origin_ptr[inst->getOperand(0)];
    else
      arg = inst->getOperand(0);
  } else if (code == STORE) {
    if (origin_ptr.count(inst->getOperand(1)))
      arg = origin_ptr[inst->getOperand(1)];
    else
      arg = inst->getOperand(1);
  } else if (code == DEALLOC) {
    arg = inst->getOperand(0);
  }
  if (!arg->getType()->isPointerTy()) return;
  args.push_back(
      CTXHTFuzzIRBuilder.CreateBitCast(arg, llvm::PointerType::get(Int8Ty, 0)));
  args.push_back(llvm::ConstantInt::get(Int8Ty, code));
  llvm::CallInst *call_inst = CTXHTFuzzIRBuilder.CreateCall(instr_fun, args);
  instr_calls.insert(call_inst);
}
void context_heap_operation_capture(llvm::Module &M, llvm::BasicBlock &BB,
                                    llvm::IRBuilder<>    &IRB,
                                    llvm::GlobalVariable *MapPtr) {
  llvm::LLVMContext &C = M.getContext();
  llvm::IntegerType *Int32Ty = llvm::IntegerType::getInt32Ty(C);
  llvm::IntegerType *Int8Ty = llvm::IntegerType::getInt8Ty(C);
  for (llvm::BasicBlock::iterator it = BB.begin(); it != BB.end(); it++) {
    llvm::Instruction *inst = &*it;

    if (llvm::CallInst *call_inst = llvm::dyn_cast<llvm::CallInst>(inst)) {
      llvm::Function *called_fn = call_inst->getCalledFunction();
      if (called_fn == nullptr) {
        llvm::Value *called_value = call_inst->getCalledOperand();
        called_fn =
            llvm::dyn_cast<llvm::Function>(called_value->stripPointerCasts());
        if (called_fn == nullptr) continue;
      }
      std::string called_fn_name = called_fn->getName().str();
      if (called_fn_name.compare(0, 5, "llvm.") == 0) continue;
      if (is_alloccall(called_fn_name)) {
        hp_op_ptr_set.insert(inst);
        last_location_insert(inst, ALLOC);
      } else if (is_dealloccall(called_fn_name)) {
        last_location_insert(inst, DEALLOC);
      }
    }

    if (llvm::LoadInst *load_inst = llvm::dyn_cast<llvm::LoadInst>(inst)) {
      llvm::Value *ptr = load_inst->getOperand(0);
      // llvm::outs() << "load_inst:\t" << *load_inst << "\n";
      if (hp_op_ptr_set.count(ptr)) {
        hp_op_ptr_set.insert(inst);
        last_location_insert(inst, LOAD);
      }
    }

    if (llvm::StoreInst *store_inst = llvm::dyn_cast<llvm::StoreInst>(inst)) {
      /*
      STORE STATEMENT: store a,b
      if a is related to heap operation, then b is also related to.
      if b is related to heap operation, then the store statement should be
      record to be instrumented.
      */
      if (hp_op_ptr_set.count(inst->getOperand(0))) {
        hp_op_ptr_set.insert(inst->getOperand(1));
        last_location_insert(inst, STORE);
      } else if (hp_op_ptr_set.count(inst->getOperand(1))) {
        last_location_insert(inst, STORE);
      }
    }

    if (llvm::GetElementPtrInst *gep_inst =
            llvm::dyn_cast<llvm::GetElementPtrInst>(inst)) {
      /*
      GetElementPtr STATEMENT: a = getelementptr b, offset
      strip the offset of a, restore a to b
      */
      if (hp_op_ptr_set.count(inst->getOperand(0))) {
        hp_op_ptr_set.insert(inst);
        if (origin_ptr.count(inst->getOperand(0)))
          origin_ptr[inst] = origin_ptr[inst->getOperand(0)];
        else
          origin_ptr[inst] = inst->getOperand(0);
      }
    }

    if (llvm::BitCastInst *bitcast_inst =
            llvm::dyn_cast<llvm::BitCastInst>(inst)) {
      if (hp_op_ptr_set.count(inst->getOperand(0))) {
        hp_op_ptr_set.insert(inst);
      }
    }
  }

  for (std::map<std::pair<llvm::Value *, int>, llvm::Instruction *>::iterator
           it = last_location.begin();
       it != last_location.end(); it++) {
    insert_update_context_function(it->second, Int8Ty, it->first.second,
                                   CTXHTFuzzUpdateContextInstr);
  }

  // load share memory pointer for
  // shm[ pre ^ cur ^ ( context heap operation sequence & mask ) ]++
  llvm::LoadInst *ctx_hp_op_shm_ptr =
      IRB.CreateLoad(llvm::PointerType::get(Int8Ty, 0), MapPtr);
  ctx_hp_op_shm_ptr->setMetadata(M.getMDKindID("nosanitize"),
                                 llvm::MDNode::get(C, llvm::None));
  // insert get context function into the beginning of a basic block
  IRB.CreateCall(CTXHTFuzzGetContextInstr);

  // load context heap operation sequence
  llvm::LoadInst *ctx_hp_op_seq =
      IRB.CreateLoad(IRB.getInt32Ty(), AFLContextHpOpSeq);
  ctx_hp_op_seq->setMetadata(M.getMDKindID("nosanitize"),
                             llvm::MDNode::get(C, llvm::None));
  llvm::Value *ctx_hp_op_seq_casted =
      IRB.CreateZExt(ctx_hp_op_seq, IRB.getInt32Ty());

  // context heap operation sequence & mask
  // to get the last CTXHTFUZZ_CONTEXT_HP_OP_SEQ_MASK bits
  llvm::Value *ctx_hp_op_seq_n_bits =
      IRB.CreateAnd(ctx_hp_op_seq_casted, ContextHpOpSeqMask);

  // compute offset: pre ^ cur ^ ( context heap sequence & mask )
  llvm::Value *ctx_hp_op_shm_ptr_idx =
      IRB.CreateGEP(Int8Ty, ctx_hp_op_shm_ptr,
                    IRB.CreateXor(Prev_xor_Cur, ctx_hp_op_seq_n_bits));

  // update: shm[ pre ^ cur ^ ( context heap sequence & mask ) ]++
  llvm::LoadInst *Counter =
      IRB.CreateLoad(IRB.getInt8Ty(), ctx_hp_op_shm_ptr_idx);
  Counter->setMetadata(M.getMDKindID("nosanitize"),
                       llvm::MDNode::get(C, llvm::None));
  llvm::Value *Incr = IRB.CreateAdd(Counter, llvm::ConstantInt::get(Int8Ty, 1));
  IRB.CreateStore(Incr, ctx_hp_op_shm_ptr_idx)
      ->setMetadata(M.getMDKindID("nosanitize"),
                    llvm::MDNode::get(C, llvm::None));

  // guided by the number of context heap operation sequence update function
  // instrumented add the number of it
  if (instr_calls.size() == 0) return;
  llvm::LoadInst *CountPtr =
      IRB.CreateLoad(llvm::PointerType::get(Int32Ty, 0), AFLCountPtr);
  CountPtr->setMetadata(M.getMDKindID("nosanitize"),
                        llvm::MDNode::get(C, llvm::None));
  llvm::Value *CountPtrIdx =
      IRB.CreateGEP(Int32Ty, CountPtr, llvm::ConstantInt::get(Int32Ty, 0));
  Counter = IRB.CreateLoad(IRB.getInt32Ty(), CountPtrIdx);
  Counter->setMetadata(M.getMDKindID("nosanitize"),
                       llvm::MDNode::get(C, llvm::None));
  Incr = IRB.CreateAdd(Counter,
                       llvm::ConstantInt::get(Int32Ty, instr_calls.size()));
  IRB.CreateStore(Incr, CountPtrIdx)
      ->setMetadata(M.getMDKindID("nosanitize"),
                    llvm::MDNode::get(C, llvm::None));
}