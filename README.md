# mathparse
Mathematical expression parser that builds an AST.

It consists of

- a tokenizer in `MathParseTokenize`
- an infix to prefix converter in `MathParseAST`

Work in progress

- an prefix-tree generator

## Features

Detailed error messages:

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
