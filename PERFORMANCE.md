# Performance comparison
Using an Iron Chromium free energy.

# SymbolicMath `performance.C`

## with backend LLVM IR
-2.36237e+08
Elapsed time: 2.92443 s

## with backend GCCJIT
-2.36237e+08
Elapsed time: 3.60245 s

## with backend LibJIT
-2.36237e+08
Elapsed time: 7.47796 s

## with backend GNU Lightning
-2.36237e+08
Elapsed time: 12.0255 s

## with backend SLJIT
-2.36237e+08
Elapsed time: 18.4495 s

## without JIT
 -2.36237e+08
 Elapsed time: 425.593 s

# FParser

## clang JIT and Optimizer
-2.36237e+08
Elapsed time: 2.84876 s

## clang JIT
-2.36237e+08
Elapsed time: 4.34102 s

## Bytecode and Optimizer
-2.36237e+08
Elapsed time: 59.7881 s

## Bytecode
-2.36237e+08
Elapsed time: 113.151 s
