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
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/Support/CommandLine.h"

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

    cl::opt<unsigned> NumPrefetchIters("prefetch-iters", cl::desc("Number of iterations ahead to prefetch"), cl::Hidden);

    struct GPUPrefetchPass : public PassInfoMixin<GPUPrefetchPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            llvm::errs() << "Running on function " << F.getName().str() << "\n";

            llvm::LoopAnalysis::Result &LI = FAM.getResult<LoopAnalysis>(F);
            llvm::ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
            llvm::TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);

            for (Loop *L : LI) {
                
                llvm::errs() << "Running on loop " << L << "\n";
                // also need to iterate over Loop *LL : depth_first(L) to get nested loops

                // step 1: identify prefetches
                if (!L->isInnermost()) {
                    llvm::errs() << "Not innermost loop - skipping\n";
                    continue;
                }

                for (const auto BB : L->blocks()) {
                    for (auto &I : *BB) {
                        // only prefetch load instructions
                        LoadInst *loadInst = dyn_cast<LoadInst>(&I);
                        if (!loadInst) {
                            continue;
                        }
                        llvm::errs() << "At instruction " << *loadInst << "\n";

                        Value *ptrOperand = loadInst->getPointerOperand();

                        const SCEV *LSCEV = SE.getSCEV(ptrOperand);

                        llvm::errs() << *LSCEV << "\n";

                        const SCEVAddRecExpr *LSCEVAddRec = dyn_cast<SCEVAddRecExpr>(LSCEV);
                        if (!LSCEVAddRec) {
                            continue;
                        }
                        llvm::errs() << *LSCEVAddRec << "\n";

                        // Insert Prefetch
                        IRBuilder<> Builder(loadInst);
                        Module *M = BB->getParent()->getParent();
                        Type *I32 = Type::getInt32Ty(BB->getContext());

                        const SCEV *NextLSCEV = SE.getAddExpr(
                            LSCEVAddRec,
                            SE.getMulExpr(
                                SE.getConstant(LSCEVAddRec->getType(), NumPrefetchIters),
                                LSCEVAddRec->getStepRecurrence(SE)
                            )
                        );

                        SCEVExpander SCEVE(SE, I.getParent()->getModule()->getDataLayout(), "prefaddr");
                        unsigned PtrAddrSpace = NextLSCEV->getType()->getPointerAddressSpace();
                        Type *I8Ptr = PointerType::get(I.getParent()->getContext(), PtrAddrSpace);
                        Value *PrefPtrValue = SCEVE.expandCodeFor(NextLSCEV, I8Ptr, &I);

                        llvm::errs() << "Prefetching " << *PrefPtrValue << "\n";

                        Function *PrefetchFunc = Intrinsic::getDeclaration(
                            M, Intrinsic::prefetch, ptrOperand->getType());
                        Builder.CreateCall(
                            PrefetchFunc,
                            {PrefPtrValue,  // pointer value + num_iters_ahead * memsize
                            ConstantInt::get(I32, 0),
                            ConstantInt::get(I32, 3), ConstantInt::get(I32, 1)});

                    }
                }
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