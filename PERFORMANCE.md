# Performance comparison
Using an Iron Chromium free energy derivative. All times for single core execution on an AMD Ryzen unless otherwise stated.

# SymbolicMath

## SymbolicMath::Function...
1.85968e+07
Elapsed time: 445.184 s

## SymbolicMath::CompiledByteCode...
1.85968e+07
Elapsed time: 381.925 s

## SymbolicMath::CompiledCCode...
4.1095e+07
Elapsed time: 46.5132 s

## SymbolicMath::CompiledSLJIT...
1.85968e+07
Elapsed time: 25.7384 s

## SymbolicMath::CompiledLibJIT...
1.85968e+07
Elapsed time: 9.52671 s

## SymbolicMath::CompiledLightning...
1.85968e+07
Elapsed time: 27.9611 s

## SymbolicMath::CompiledLLVM...
1.85968e+07

# FParser

## Bytecode
1.85968e+07
Elapsed time: 66.9428 s

## Bytecode with Optimizer
1.85968e+07
Elapsed time: 54.5693 s

## JIT with Optimizer
1.85968e+07
Elapsed time: 1.73766 s

## JIT
1.85968e+07
Elapsed time: 2.08427 s
