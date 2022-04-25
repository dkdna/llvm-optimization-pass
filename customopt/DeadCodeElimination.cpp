#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {
    struct DCEPass : public FunctionPass {
        static char ID;
        DCEPass() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function& function) {

            // Vector of instructions to delete at the end of the pass (void instructions)
            std::vector<Instruction *> instsToDelete;

            errs() << "Starting Dead Code Elimination pass "
                "for function: '" << function.getName() << "':\n";

            // Iterate through each basic block of the function
            for (auto& block: function) {

                // Iterate through each instruction in the basic block
                for (auto& instruction: block) {
                    
                    Instruction* ins = &instruction;

                    // errs() << "Current instruction: " << instruction << "\n";

                    // If instruction is in use, 
                    // or has side effects, 
                    // or is a terminator instruction, continue
                    if (!ins->use_empty() || 
                        ins->mayHaveSideEffects() ||
                        ins->isTerminator() ) {
                        continue;
                    }

                    // If instruction is a landing pad instruction (jump target), continue
                    LandingPadInst *lp = dyn_cast<LandingPadInst>(ins);
                    if (lp) {
                        continue;
                    }

                    // Delete instruction
                    instsToDelete.push_back(ins);

                    errs() << "Deleting instruction: " << instruction << "\n";
                }
            }

            errs() << "Dead Code Elimination pass complete!\n\n";
                
            // Delete all the unnecessary instructions
            for(auto i: instsToDelete) {
                i->eraseFromParent();
            }

            return true;
        
        }
    };
}

char DCEPass::ID = 0;

static RegisterPass<DCEPass> X("dcelim", "Dead Code Elimination", false, true);
