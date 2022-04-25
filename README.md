## llvm-optimization-pass

A few simple optimization passes implemented for LLVM IR.

The optimization passes implemented are:
+ Strength Reduction
+ Constant Folding
+ Dead Code Elimination
+ Common Subexpression Elimination

A few example input files are in the [examples](examples/) directory.

### Build/Run Instructions

Run `build.sh` to build the LLVM IR optimization passes (make sure to replace `CC` and `LLVM_DIR` with your paths).

Run `run.sh` with the first argument as your target `.c` file to see the optimization passes in action.
