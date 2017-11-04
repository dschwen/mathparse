# SymbolicMath [![Build Status](https://travis-ci.org/dschwen/mathparse.svg?branch=master)](https://travis-ci.org/dschwen/mathparse)
Abstract syntax tree (AST) based symbolic math system with an expression parser.

It consists of

- a tokenizer in `SymbolicMathTokenize`
- an infix to prefix tree converter in `SymbolicMathParser`
- an automatic differentiation component
- Just-in-time compilation
## Building SymbolicMath

Build using
```
make JIT=backend
```

Where `backend` is one of the options in the table below.

## Features

### Just-in-time compilation

SymbolicMath features just-in-time (JIT) compilation of expression trees. Available JIT backends are:

`JIT=` parameter|Project | Description
---------|-------------|------------
`llvmir` |[LLVM](http://llvm.org) | Builds LLVM intermediate representation, optimizes it, and compiles it with LLVM Orc JIT
`gccjit`|[libgccjit](https://gcc.gnu.org/onlinedocs/jit/)| Uses _libggcjit_ from the GNU Compiler Collection (gcc) to build _-O3_ optimized code in memory
`ccode` | -| Launches an external compiler to compile generated C code and links in the generated object using _dlopen_
`libjit`| [GNU LibJIT](https://www.gnu.org/software/libjit/) | JIT library originally developed for the Mono project. Limited architecture support
`lightning`| [GNU Lightning](https://www.gnu.org/software/lightning/) | Low level assembly generator. Good architecture support
`sljit`| [SLJIT](http://sljit.sourceforge.net/) | Low level assembly generator. Broad architecture support, no external dependencies

Research is ongoing as to which backend is best used. See [performance comparison](PERFORMANCE.md) for details.

### Detailed error messages:

```
> ./mathparse 'a + b * (c - d'
  a + b * (c - d
        ~~^~~
Unmatched opening bracket
```

```
./mathparse 'a + b * c) - d'
 a + b * c) - d
        ~~^~~
Unmatched closing bracket
```

```
> ./mathparse 'a + b c - d'
  a + b c - d
      ~~^~~
Operator expected here
```

```
> ./mathparse 'a + b - * d'
  a + b - * d
        ~~^~~
Did not expect operator here
```

```
> ./mathparse 'a + b * c -= d'
  a + b * c -= d
          ~~^~~
    Unknown operator
```

```
> ./mathparse 'a + b * $ - d'
  a + b * $ - d
        ~~^~~
     Parse error
```

```
> ./mathparse 'a + sin()'
  a + sin()
    ~~^~~
Function takes 1 argument(s), but none were given
```

```
> ./mathparse 'a + sin(1,3)'
  a + sin(1,3)
    ~~^~~
Expected 1 argument(s) but found 2
```

```
> ./mathparse 'a - schwing(1,3,3)'
  a - schwing(1,3,3)
    ~~^~~
Unknown function
```
