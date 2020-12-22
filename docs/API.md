# API

All methods of the symbolic math library are scoped under the namespace
`SymbolicMath`. All relevant classes are templated on the value type of the
expressions. An alias for the double precision floating point type is
`SymbolicMath::Real`, which will be used in the examples below.

# Parsing a string expression

Create a parser object for expressions with double precision values

```
SymbolicMath::Parser<SymbolicMath::Real> parser;
```

Create variables to be used in the parsed expression.

```
SymbolicMath::Real c, T;
```

Register the C++ variables in the parser under the selected names "c" and "y".

```
auto c_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(c, "c");
auto T_var = std::make_shared<SymbolicMath::RealReferenceData<SymbolicMath::Real>>(T, "y");
parser.registerValueProvider(c_var);
parser.registerValueProvider(T_var);
```

Register some numerical constants

```
parser.registerConstant("kB", 8.6173324e-5);
parser.registerConstant("T0", 410.0);
```

Generate a function object `func` using the parser and a string containing the
mathematical expression

```
auto func = parser.parse("log10(c)*exp((y-T0)*kB)+0*y");
```

## Transforms

Simplify the parsed function

```
SymbolicMath::Simplify<SymbolicMath::Real> simplify(func);
```

Evaluate `func` for chosen values of `c` ("c") and `T` ("y"). The C++ variables
`c` and `T` are bound to the function `func` and their current values will be
used when evaluating the function.

```
c = 0.5;
T = 300.0;
std::cout << "Value = " << func() << '\n';
```

## Derivatives

Build the symbolic derivative of `func` with respect to `c`

```
auto diff = func.D(c_var);
```

## Compilation

A variety of Just-in-Time compilation backends are available. The backends are
registered in `SymbolicMath::CompilerFactory<T>`, where `T` is the value type of
the expression.

A compiler backend can be built by calling

```
auto best_comp = SymbolicMath::CompilerFactory<SymbolicMath::Real>::buildBestCompiler(func);
```

This will instantiate the best available compiler backend on the current
platform and for the selected value type `T`.

The compiled expression can be evaluated as follows

```
std::cout << "Value = " << (*best_comp)() << '\n';
```

Bindings to the variables used in `func` are carried over to the compiled
instances. In this example changing the C++ variables `c` and `T` will affect
the result returned by `(*best_comp)()`.

### Debugging

A list of available compiler backends can be obtained through

```
std::vector<std::string>> compilers = SymbolicMath::CompilerFactory<SymbolicMath::Real>::listCompilers();
```

and the best available compiler backend (highest priority score) can be queried
through

```
std::string best_compiler = SymbolicMath::CompilerFactory<SymbolicMath::Real>::bestCompiler();
```
