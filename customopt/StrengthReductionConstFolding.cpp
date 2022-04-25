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
    struct SRCFPass : public FunctionPass {
        static char ID;
        SRCFPass() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function& function) {

            // Vector of instructions to delete at the end of the pass (void instructions)
            std::vector<Instruction *> instsToDelete;

            errs() << "Starting Strength Reduction & Constant Folding pass "
                "for function: '" << function.getName() << "':\n";

            // Iterate through each basic block of the function
            for (auto& block: function) {

                // Iterate through each instruction in the basic block
                for (auto& instruction: block) {

                    // errs() << "Current instruction: " << instruction << "\n";

                    // If operator is a binary operator
                    if (instruction.isBinaryOp()) {
                        
                        // Cast instruction to a BinaryOperator type
                        auto* op = dyn_cast<BinaryOperator>(&instruction);
                        Value* left = op->getOperand(0);
                        Value* right = op->getOperand(1);

                        switch (op->getOpcode()) {
                            
                            // If opcode is multiply
                            case Instruction::Mul:

                                // If left is a constant integer
                                if(ConstantInt* lint = dyn_cast<ConstantInt>(left)) {
                                    
                                    // Cast left into an integer
                                    auto lvalue = lint->getSExtValue();

                                    // If right is also a constant integer, constant fold
                                    if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {


                                        // Cast right into an integer
                                        auto rvalue = rint->getSExtValue();

                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all instances of the instruction with constant 
                                        Value* result = ConstantInt::get(left->getType(), lvalue * rvalue);

                                        op->replaceAllUsesWith(result);

                                        errs() << "Constant folding: ";
                                        errs() << lvalue << "*" << rvalue << " -> " << (lvalue * rvalue) << "\n";

                                    }

                                    // Else right is not a constant integer
                                    else {

                                        // If left is a power of 2 -> Strength reduction into shl
                                        if ((lvalue != 0) && ((lvalue & (lvalue - 1)) == 0)) {

                                            int count = 0;
                                            int tmp = lvalue;
                                            while(tmp != 1) {
                                                tmp >>= 1;
                                                count ++;
                                            }

                                            Value* newval = ConstantInt::get(left->getType(), count);

                                            // Inserts a new left shift instruction in the IR, instead of the multiply
                                            IRBuilder<> builder(op);
                                            Value* shl = builder.CreateShl(right, newval);

                                            errs() << "Strength reduction: ";
                                            errs() << lvalue << "*x -> x<<" << count << "\n";

                                            // Replace all uses with shl instruction
                                            for (auto& U : op->uses()) {
                                                User* user = U.getUser();
                                                user->setOperand(U.getOperandNo(), shl);
                                            }

                                            // Prepare to delete instruction
                                            instsToDelete.push_back(op);

                                        }

                                        // If left == 1, strength reduce (multiplication is useless)

                                        if(lvalue == 1) {
                                        
                                            // Prepare to delete instruction
                                            instsToDelete.push_back(op);

                                            // Replace all uses of op with left
                                            op->replaceAllUsesWith(right);

                                            errs() << "Strength reduction: ";
                                            errs() << lvalue << "*x -> x\n";

                                        }
                                    }
                            
                                }

                                // Else if only right is a constant integer
                                else if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                    // Cast right into an integer
                                    auto rvalue = rint->getSExtValue();

                                    // If right == 1, strength reduce (multiplication is useless)
                                    if(rvalue == 1) {
                                        
                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all uses of op with left
                                        op->replaceAllUsesWith(left);

                                        errs() << "Strength reduction: ";
                                        errs() << "x*" << rvalue << " -> x\n";

                                    }

                                    // If right is a power of 2 -> Strength reduction to shl
                                    if ((rvalue != 0) && ((rvalue & (rvalue - 1)) == 0)) {

                                        int count = 0;
                                        int tmp = rvalue;
                                        while(tmp != 1) {
                                            tmp >>= 1;
                                            count ++;
                                        }

                                        Value* newval = ConstantInt::get(left->getType(), count);
                                        
                                        // Inserts a new left shift instruction in the IR, instead of the multiply
                                        IRBuilder<> builder(op);
                                        Value* shl = builder.CreateShl(left, newval);

                                        errs() << "Strength reduction: ";
                                        errs() << "x*" << rvalue << " -> " << "x<<" << count << "\n";

                                        // Replace all uses with shl instruction
                                        for (auto& U : op->uses()) {
                                            User* user = U.getUser();
                                            user->setOperand(U.getOperandNo(), shl);
                                        }

                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                    }
                                }

                                break;


                            // If opcode is divide
                            // Division is not commutative so only certain instructions are possible
                            case Instruction::UDiv:
                            case Instruction::SDiv:

                                // If left is a constant integer
                                if(ConstantInt* lint = dyn_cast<ConstantInt>(left)) {
                                    
                                    // Cast left into an integer
                                    auto lvalue = lint->getSExtValue();

                                    // If right is also a constant integer, constant fold
                                    if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                        // Cast right into an integer
                                        auto rvalue = rint->getSExtValue();

                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all instances of the instruction with constant 
                                        Value* result = ConstantInt::get(left->getType(), lvalue / rvalue);

                                        op->replaceAllUsesWith(result);

                                        errs() << "Constant folding: ";
                                        errs() << lvalue << "/" << rvalue << " -> " << (lvalue / rvalue) << "\n";

                                    }
                            
                                }

                                // Else if only right is a constant integer
                                else if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                    // Cast right into an integer
                                    auto rvalue = rint->getSExtValue();

                                    // If right is a power of 2 -> Strength reduction to shr
                                    if ((rvalue != 0) && ((rvalue & (rvalue - 1)) == 0)) {

                                        int count = 0;
                                        int tmp = rvalue;
                                        while(tmp != 1) {
                                            tmp >>= 1;
                                            count ++;
                                        }

                                        Value* newval = ConstantInt::get(left->getType(), count);
                                        
                                        // Inserts a new right shift instruction in the IR, instead of the multiply
                                        IRBuilder<> builder(op);
                                        Value* shl = builder.CreateAShr(left, newval);

                                        errs() << "Strength reduction: ";
                                        errs() << "x/" << rvalue << " -> " << "x>>" << count << "\n";

                                        // Replace all uses with shl instruction
                                        for (auto& U : op->uses()) {
                                            User* user = U.getUser();
                                            user->setOperand(U.getOperandNo(), shl);
                                        }

                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                    }

                                    // If right == 1, strength reduce (division is useless)
                                    if(rvalue == 1) {
                                        
                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all uses of op with left
                                        op->replaceAllUsesWith(left);

                                        errs() << "Strength reduction: ";
                                        errs() << "x/" << rvalue << " -> x\n";

                                    }
                                }
                                break;

                            case Instruction::Add:

                                if(ConstantInt* lint = dyn_cast<ConstantInt>(left)) {

                                    // Cast left into an integer
                                    auto lvalue = lint->getSExtValue();

                                    // If left and right are constant integers, constant fold
                                    if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                        // Cast right into an integer
                                        auto rvalue = rint->getSExtValue();

                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all instances of the instruction with constant 
                                        Value* result = ConstantInt::get(left->getType(), lvalue + rvalue);

                                        op->replaceAllUsesWith(result);

                                        errs() << "Constant folding: ";
                                        errs() << lvalue << "+" << rvalue << " -> " << (lvalue + rvalue) << "\n";

                                    }

                                    // Else right is not a constant integer
                                    else {
                                        
                                        // If left == 0, convert to only right
                                        if (lvalue == 0) {
                                            
                                            // Prepare to delete instruction
                                            instsToDelete.push_back(op);

                                            // Replace all uses of op with right
                                            op->replaceAllUsesWith(right);

                                            errs() << "Strength reduction: ";
                                            errs() << lvalue << "+x -> x\n";
                                        }
                                    }

                                }

                                // Else if only right is a constant integer
                                else if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                    // Cast right into an integer
                                    auto rvalue = rint->getSExtValue();

                                    // If right == 0, convert to only left
                                    if (rvalue == 0) {
                                        
                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all uses of op with left
                                        op->replaceAllUsesWith(left);

                                        errs() << "Strength reduction: ";
                                        errs() << "x+" << rvalue << " -> x\n";
                                    }
                                    
                                }
                                break;

                            case Instruction::Sub:

                                if(ConstantInt* lint = dyn_cast<ConstantInt>(left)) {

                                    // Cast left into an integer
                                    auto lvalue = lint->getSExtValue();

                                    // If left and right are constant integers, constant fold
                                    if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                        // Cast right into an integer
                                        auto rvalue = rint->getSExtValue();

                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all instances of the instruction with constant 
                                        Value* result = ConstantInt::get(left->getType(), lvalue - rvalue);

                                        op->replaceAllUsesWith(result);

                                        errs() << "Constant folding: ";
                                        errs() << lvalue << "-" << rvalue << " -> " << (lvalue - rvalue) << "\n";

                                    }
                                }
                                
                                // Else if only right is a constant integer
                                else if(ConstantInt* rint = dyn_cast<ConstantInt>(right)) {

                                    // Cast right into an integer
                                    auto rvalue = rint->getSExtValue();

                                    // If right == 0, convert to only left
                                    if (rvalue == 0) {
                                        
                                        // Prepare to delete instruction
                                        instsToDelete.push_back(op);

                                        // Replace all uses of op with left
                                        op->replaceAllUsesWith(left);

                                        errs() << "Strength reduction: ";
                                        errs() << "x-" << rvalue << " -> x\n";
                                    }
                                    
                                }
                                break;
                            
                            // For now, we dont handle other opcodes
                            default:
                                break;

                        }
                
                    }
                }
            }

            errs() << "Strength Reduction & Constant Folding pass complete!\n\n";
                
            // Delete all the unnecessary instructions
            for(auto i: instsToDelete) {
                i->eraseFromParent();
            }

            return true;

        }
        
    };
}

char SRCFPass::ID = 0;

static RegisterPass<SRCFPass> X("srcf", "Strength Reduction & Constant Folding", false, true);
