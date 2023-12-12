// pass headers
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

// analysis headers
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"

// transform headers
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

// IR headers
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"

// other headers
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#include <vector>

using namespace llvm;

namespace {

    cl::opt<unsigned int> NumPrefetchIters("prefetch-iters",
        cl::desc("Number of iterations ahead to prefetch")
    );

    struct Prefetch {
        Function *prefetch_func;
        Value *prefetch_ptr;
        Instruction *inst;

        Prefetch(Function *f, Value *ptr, Instruction *I) : 
            prefetch_func(f), prefetch_ptr(ptr), inst(I) {}
    };

    struct GPUPrefetchPass : public PassInfoMixin<GPUPrefetchPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            llvm::errs() << "Running on function " << F.getName().str() << "\n";

            llvm::LoopAnalysis::Result &LI = FAM.getResult<LoopAnalysis>(F);
            llvm::ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

            std::vector<Value*> prefetched_addr;
            std::vector<Prefetch> prefetches;
            // LoopAnalysis only returns top-level loops, use DFS to get all inner loops (if any)
            for (Loop *L : LI) {

                for (const auto BB : L->blocks()) {
                    for (auto &I : *BB) {
                        // only prefetch load instructions
                        LoadInst *loadInst = dyn_cast<LoadInst>(&I);
                        if (!loadInst) {
                            continue;
                        }
                        llvm::errs() << "At instruction " << *loadInst << "\n";

                        // Determine if memory address to load is recurrence on induction variable
                        Value *ptrOperand = loadInst->getPointerOperand();
                        const SCEV *scev = SE.getSCEV(ptrOperand);
                        const SCEVAddRecExpr *indVarRec = dyn_cast<SCEVAddRecExpr>(scev);
                        if (!indVarRec) {
                            continue;
                        }
                        llvm::errs() << "   SCEVAddRecExpr: " << *indVarRec << "\n";

                        // Compute address of memory location to prefetch after NumPrefetchIters iterations
                        // const SCEV *nextSCEV = SE.getAddExpr(indVarRec, indVarRec->getStepRecurrence(SE));
                        const SCEV *nextSCEV = SE.getAddExpr(indVarRec,
                            SE.getMulExpr(
                                SE.getConstant(indVarRec->getType(), NumPrefetchIters),
                                indVarRec->getStepRecurrence(SE)
                            )
                        );

                        // Expand address into code getelementpointer instruction, hoist expansion
                        // into loop preheader if possible
                        SCEVExpander expander(SE, I.getParent()->getModule()->getDataLayout(), "scevExpander");
                        Type *ptrType = ptrOperand->getType();
                        Value *prefetchInst = expander.expandCodeFor(nextSCEV, ptrType, &I);

                        llvm::errs() << "   Looking at address " << prefetchInst << "\n";

                        // Duplicate checking
                        auto it = std::find(prefetched_addr.begin(), prefetched_addr.end(), prefetchInst);
                        if (it != prefetched_addr.end()) {
                            llvm::errs() << "   " << prefetchInst << " was already prefetched\n";
                            continue;
                        }
                        prefetched_addr.push_back(prefetchInst);

                        // Save prefetch to avoid modifying the instructions in the loop
                        Module *mod = BB->getParent()->getParent();
                        Function *func = Intrinsic::getDeclaration(mod, Intrinsic::prefetch, ptrType);
                        prefetches.push_back(Prefetch(func, prefetchInst, loadInst));
                    }
                }
            }
            
            // Insert all the prefetches in the appropriate locations
            for (Prefetch &p : prefetches) {
                Function *prefetch_func = p.prefetch_func;
                Value *prefetchInst = p.prefetch_ptr;
                Instruction *inst = p.inst;

                IRBuilder<> builder(inst);
                llvm::errs() << "Prefetching " << *prefetchInst << "\n";

                Type *int32Type = Type::getInt32Ty(inst->getParent()->getContext());
                ArrayRef<Value*> args = {
                    prefetchInst,
                    ConstantInt::get(int32Type, 0),  // load
                    ConstantInt::get(int32Type, 3),  // max locality
                    ConstantInt::get(int32Type, 1)   // memory prefetch
                };
                builder.CreateCall(prefetch_func, args);
            }
            return PreservedAnalyses::none();
        }
    };
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "GPUPrefetchPass", "v0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "gpu-prefetch-pass") {
                        FPM.addPass(GPUPrefetchPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}