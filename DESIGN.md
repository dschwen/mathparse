# Design document

## Requirements
The symbolic math module has the following requirements

### Basic requirements

- Register _value providers_ to a symbolic math object
    - user settable variables
    - MOOSE non-linear variables (requires connecting Assemby/FEProblem to a symbolic math object)
    - Gradients of MOOSE variables
    - MOOSE scalar variables
    - MOOSE Postprocessors
    - MOOSE material properties
- Build an abstract syntax tree (AST) from a parsed expression
- Build an AST using operator overloading (ExpressionBuilder)
- Provide automatic differentiation
- Fast evaluation (JIT?)

### Advanced requirements

- Differential operator support (nabla)
- Derivatives w.r.t. variable gradients
- Functional derivatives
- Derivatives w.r.t. shape function coefficients (phi)
- Tensor support
    - shape validation
    - Matrix algebra
- Sub expressions (local variables)

## Structure

The parser uses a tokenizer, a prefix stack builder, and an AST builder. Both
the tokenizer as well as the prefix stack only need to live while the expression
is parsed form plain text. All further manipulation and evaluation is performed
on the AST (i.e. no round-trip conversions like in FParser).

### Tokenizer

The tokenizer splits a character string in to semantic units such as operators,
numbers and function/variable names. It assigns tags to operators, variables, and
functions. If an operator, variable, or function is not recognized it is marked
with an _invalid_ tag. Thi requires the tokenizer to know valid operators (static
list), valid function names (ststic list), and valid variables (dynamic list).

Variables (or more generally value providers) need to be registered after the
construction of a symbolic math object, but before parsing (this registration is
not necessary when using operator overloading to construct the object).

Tokens store their position in the original input expression string to facilitate
the formatting of error messages.

### Prefix stack

The prefix notation is built using the shunting yard algorithm. The shunting yard
loop has the current and last token avialable. In a preprcessing step using the
last token we discriminate between binary and unary plus and minus. Then validation
is performed. We check

- Bracket balancing
- Invalid tags (unknown funxtions, variables, or operators)
- Consecutive symbols or operators (well-formedness)
- Correct number of arguments for functions

### Abstract Syntax Tree (AST) builder

The AST is built by popping operators off the prefix stack and recursively popping
their argument subtrees.

## References

- https://en.wikipedia.org/wiki/Shunting-yard_algorithm
- http://wcipeg.com/wiki/Shunting_yard_algorithm
