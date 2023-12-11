// pass headers
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
// loop headers
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
// block headers
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
// other headers
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#include <vector>

// useful for GDB
#ifdef DEBUG
#define LOG(X) llvm::errs() << X << "\n";

// this function is for gdb
void printValue(const llvm::Value *V)
{
    LOG(*V);
}
#else
#define LOG(X)
#endif
//

using namespace llvm;

namespace {
    /// A record for a potential prefetch made during the initial scan of the
    /// loop. This is used to let a single prefetch target multiple memory accesses.
    struct Prefetch {
        /// The address formula for this prefetch as returned by ScalarEvolution.
        const SCEVAddRecExpr *LSCEVAddRec;
        /// The point of insertion for the prefetch instruction.
        Instruction *InsertPt = nullptr;
        /// True if targeting a write memory access.
        bool Writes = false;
        /// The (first seen) prefetched instruction.
        Instruction *MemI = nullptr;

        /// Constructor to create a new Prefetch for \p I.
        Prefetch(const SCEVAddRecExpr *L, Instruction *I) : LSCEVAddRec(L)
        {
            MemI = I;
            InsertPt = I;
            Writes = isa<StoreInst>(I);
        };
    };

    struct GPUPrefetchPass : public PassInfoMixin<GPUPrefetchPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            llvm::errs() << "Running on function " << F.getName().str() << "\n";

            llvm::LoopAnalysis::Result &LI = FAM.getResult<LoopAnalysis>(F);
            // DominatorTree *DT = &AM.getResult<DominatorTreeAnalysis>(F);
            ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
            // AssumptionCache *AC = &AM.getResult<AssumptionAnalysis>(F);

            for (Loop *L : LI) {
                
                // llvm::errs() << "Running on loop " << L << "\n";
                // also need to iterate over Loop *LL : depth_first(L) to get nested loops

                // step 1: identify prefetches
                if (!L->isInnermost()) {
                    // llvm::errs() << "Not innermost loop - skipping\n";
                    continue;
                }

                std::vector<Prefetch> Prefetches;
                for (const auto BB : L->blocks()) {
                    for (auto &I : *BB) {
                        // only prefetch load instructions
                        LoadInst *LMemI = dyn_cast<LoadInst>(&I);
                        if (!LMemI) {
                            continue;
                        }
                        llvm::errs() << "At instruction " << *LMemI << "\n";

                        Instruction *MemI = LMemI;
                        Value *PtrValue = LMemI->getPointerOperand();

                        // unsigned PtrAddrSpace = PtrValue->getType()->getPointerAddressSpace();
                        // if (!TTI->shouldPrefetchAddressSpace(PtrAddrSpace)) {
                        //     continue;
                        // }
                        // if (L->isLoopInvariant(PtrValue)) {
                        //     continue;
                        // }

                        const SCEV *LSCEV = SE.getSCEV(PtrValue);

                        llvm::errs() << *LSCEV << "\n";

                        const SCEVAddExpr *LSCEVAddExpr = dyn_cast<SCEVAddExpr>(LSCEV);
                        if (!LSCEVAddExpr) {
                            llvm::errs() << " :( \n";
                            continue;
                        }

                        const SCEVAddRecExpr *LSCEVAddRec = dyn_cast<SCEVAddRecExpr>(LSCEVAddExpr->getOperand(0));//
                        if (!LSCEVAddRec) {
                            llvm::errs() << " :( \n";
                            continue;
                        }
                        llvm::errs() << *LSCEVAddRec << "\n";

                        Prefetches.push_back(Prefetch(LSCEVAddRec, MemI));
                        llvm::errs() << "Prefetch at instruction " << MemI << " possible\n";
                    }
                }
                // step 2: prefetch prefetches
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