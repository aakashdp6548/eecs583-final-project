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
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include <iostream>
#include <vector>

using namespace llvm;

namespace {
    struct SCVETestPass : public PassInfoMixin<SCVETestPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            llvm::errs() << "Running on function " << F.getName().str() << "\n";

            llvm::LoopAnalysis::Result &LI = FAM.getResult<LoopAnalysis>(F);
            llvm::ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
            llvm::TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);

            for (Loop *L : LI) {
                for (const auto BB : L->blocks()) {
                    for (auto &I : *BB) {
                        // only prefetch load instructions
                        LoadInst *LMemI = dyn_cast<LoadInst>(&I);
                        if (!LMemI) {
                            continue;
                        }
                        llvm::errs() << "Instruction " << *LMemI << "\n";

                        Instruction *MemI = LMemI;
                        Value *PtrValue = LMemI->getPointerOperand();

                        const SCEV *LSCEV = SE.getSCEV(PtrValue);
                        llvm::errs() << "   SCEV: " << *LSCEV << "\n";

                        bool isAddRecExpr = SE.containsAddRecurrence(LSCEV);
                        llvm::errs() << "   Does " << (isAddRecExpr ? "" : "NOT ") << "contain add recurrence\n";

                        const SCEVAddRecExpr *LSCEVAddRec = dyn_cast<SCEVAddRecExpr>(LSCEV);
                        if (!LSCEVAddRec) {
                            continue;
                        }
                        llvm::errs() << "   SCEVAddRecExpr: " << *LSCEVAddRec << "\n";
                    }
                }
            }
            return PreservedAnalyses::all();
        }
    };
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "SCVETestPass", "v0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "scev-test-pass") {
                        FPM.addPass(SCVETestPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}