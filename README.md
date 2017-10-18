# SymbolicMath [![Build Status](https://travis-ci.org/dschwen/mathparse.svg?branch=master)](https://travis-ci.org/dschwen/mathparse)
Abstract syntax tree (AST) based symbolic math system with an expression parser.

It consists of

- a tokenizer in `SymbolicMathTokenize`
- an infix to prefix tree converter in `SymbolicMathParser`

## Features

### Just-in-time compilation

SymbolicMath features just-in-time (JIT) compilation of expression trees.
Available JIT backends are

* [GNU LibJIT](https://www.gnu.org/software/libjit/) - Fastest option, limited architecture support
* [GNU Lightning](https://www.gnu.org/software/lightning/) - Good architecture support, medium speed
* [SLJIT](http://sljit.sourceforge.net/) - Broad architecture support, no external dependencies, slowest option

Research is ongoing as to which backend is best used.

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
