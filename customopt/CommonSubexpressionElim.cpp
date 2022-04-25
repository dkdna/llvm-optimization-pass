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
    struct CSEPass : public FunctionPass {
        static char ID;
        CSEPass() : FunctionPass(ID) {}

        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {

            // AU.setPreservesCFG();
            AU.addRequired<DominatorTreeWrapperPass>();
            return;

        }

        virtual bool runOnFunction(Function& function) override {

            // Vector of instructions to delete at the end of the pass (void instructions)
            std::vector<Instruction *> instsToDelete;

            // Vector of previously seen instructions (globally)
            std::vector<Instruction *> globalSeenInstructions;

            // Get dominator tree for the function
            DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

            errs() << "Starting Common Subexpression Elimination pass "
                "for function: '" << function.getName() << "':\n";

            // Iterate through each basic block of the function
            for (auto& block: function) {

                // Vector of previously seen instructions (locally)
                std::vector<Instruction *> localSeenInstructions;

                // Iterate through each instruction in the basic block
                for (auto& instruction: block) {

                    // errs() << "Current instruction: " << instruction << "\n";

                    Instruction* ins = &instruction;
                    Instruction* identical;

                    // Common subexpression elimination
                    bool localSeen = false;
                    bool globalSeen = false;

                    // We cannot replace a terminating instruction
                    if(ins->isTerminator()) {
                        continue;
                    }

                    // We cannot replace a landing pad instruction either
                    LandingPadInst *lp = dyn_cast<LandingPadInst>(ins);
                    if (lp) {
                        continue;
                    }

                    // As it is SSA we dont need to worry about instructions being modified
                    for (auto& i: localSeenInstructions) {

                        if (ins->isIdenticalTo(i)) {
                            
                            // errs() << "Found local instruction: " << instruction << "!\n";

                            // Mark locally seen as true, and save the identical instruction
                            identical = i;
                            localSeen = true;
                            break;
                            
                        }

                    }

                    if (localSeen) {

                        // Replace all uses of the instruction with the identical one
                        ins->replaceAllUsesWith(identical);

                        // Delete instruction
                        instsToDelete.push_back(ins);
                        errs() << "Found Local Common Subexpression: " << instruction << ", Deleting\n";

                        continue;

                    }

                    for (auto& i: globalSeenInstructions) {

                        if (ins->isIdenticalTo(i)) {
                            
                            // errs() << "Found global instruction: " << instruction << "!\n";

                            // Mark globally seen as true, and save the identical instruction
                            identical = i;
                            globalSeen = true;
                            break;
                            
                        }

                    }

                    if (globalSeen) {
                        
                        // Set if we can delete the variable (able to replace all uses)
                        bool canClear = true;

                        for (auto& U : ins->uses()) {
                            
                            // If identical dominates U, 
                            // i.e if identical can be substituted at all possible uses of U
                            if (DT->dominates(identical, U)) {
        
                                errs() << "Found Global Common Subexpression: " << instruction << ", Replacing\n";

                            }

                            // We hit a non-dominant case, cannot delete the instruction altogether
                            else {
                                canClear = false;
                            }
                        }

                        if (canClear) {
                            
                            // Replace all uses of the instruction with the identical one
                            ins->replaceAllUsesWith(identical);
                            instsToDelete.push_back(ins);
                        }

                        continue;

                    }

                    else {

                        // Add instruction to both locally and globally seen
                        localSeenInstructions.push_back(ins);
                        globalSeenInstructions.push_back(ins);

                    }
                }
            }

            errs() << "Common Subexpression Elimination pass complete!\n\n";

            // Delete all the unnecessary instructions
            for(auto i: instsToDelete) {
                i->eraseFromParent();
            }

            return true;
        
        }
    };
}

char CSEPass::ID = 0;

static RegisterPass<CSEPass> X("cse", "Common Subexpression Elimination", false, true);
