#ifndef SYMBOLICMATH_TERM_H
#define SYMBOLICMATH_TERM_H

#include <vector>
#include <string>
#include <cassert>

typedef double Real;

namespace SymbolicMath
{

// forward delcarations
class Term;
class TermNode;
class Function;
class SubstitutionRule;
typedef std::vector<Term> TermList;
typedef std::vector<TermNode *> TermNodeList;
typedef std::vector<const SubstitutionRule *> SubstitutionRuleList;

/// Base class for nodes in the expression tree
class TermNode
{
public:
  virtual ~TermNode(){};
  virtual TermNode * clone() const = 0;

  virtual std::string stringify() const = 0;
  virtual unsigned int substitute(const SubstitutionRuleList & /*rule*/) { return 0; }
  virtual int precedence() const = 0;
  friend std::ostream & operator<<(std::ostream & os, const TermNode & node)
  {
    return os << node.stringify();
  }
};

/// Template class for leaf nodes holding numbers in the expression tree
template <typename T>
class NumberNode : public TermNode
{
  T _value;

public:
  NumberNode(T value) : _value(value){};
  virtual NumberNode<T> * clone() const { return new NumberNode(_value); }

  virtual std::string stringify() const;
  virtual int precedence() const { return 0; }
};

/// Template class for leaf nodes holding symbols (i.e. variables) in the expression tree
class SymbolNode : public TermNode
{
  std::string _symbol;

public:
  SymbolNode(std::string symbol) : _symbol(symbol){};
  virtual SymbolNode * clone() const { return new SymbolNode(_symbol); }

  virtual std::string stringify() const;
  virtual int precedence() const { return 0; }
};

/**
 * Template class for leaf nodes holding anonymous IDs in the expression tree.
 * No such node must be left in the final expression that is serialized and passed to FParser
 */
class TempIDNode : public TermNode
{
  unsigned long _id;

public:
  TempIDNode(unsigned int id) : _id(id){};
  virtual TempIDNode * clone() const { return new TempIDNode(_id); }

  virtual std::string stringify() const; // returns "[idnumber]"
  virtual int precedence() const { return 0; }
};

/// Base class for nodes with a single sub node (i.e. functions or operators taking one argument)
class UnaryTermNode : public TermNode
{
public:
  UnaryTermNode(TermNode * subnode) : _subnode(subnode){};
  virtual ~UnaryTermNode() { delete _subnode; };

  virtual unsigned int substitute(const SubstitutionRuleList & rule);
  const TermNode * getSubnode() const { return _subnode; }

protected:
  TermNode * _subnode;
};

/// Node representing a function with two arguments
class UnaryFuncTermNode : public UnaryTermNode
{
public:
  enum Type
  {
    SIN,
    COS,
    TAN,
    ABS,
    LOG,
    LOG2,
    LOG10,
    EXP,
    SINH,
    COSH
  } _type;

  UnaryFuncTermNode(TermNode * subnode, Type type) : UnaryTermNode(subnode), _type(type){};
  virtual UnaryFuncTermNode * clone() const
  {
    return new UnaryFuncTermNode(_subnode->clone(), _type);
  };

  virtual std::string stringify() const;
  virtual int precedence() const { return 2; }
};

/// Node representing a unary operator
class UnaryOpTermNode : public UnaryTermNode
{
public:
  enum Type
  {
    NEG,
    LOGICNOT
  } _type;

  UnaryOpTermNode(TermNode * subnode, Type type) : UnaryTermNode(subnode), _type(type){};
  virtual UnaryOpTermNode * clone() const { return new UnaryOpTermNode(_subnode->clone(), _type); };

  virtual std::string stringify() const;
  virtual int precedence() const { return 3; }
};

/// Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
class BinaryTermNode : public TermNode
{
public:
  BinaryTermNode(TermNode * left, TermNode * right) : _left(left), _right(right){};
  virtual ~BinaryTermNode()
  {
    delete _left;
    delete _right;
  };

  virtual unsigned int substitute(const SubstitutionRuleList & rule);

protected:
  TermNode * _left;
  TermNode * _right;
};

/// Node representing a binary operator
class BinaryOpTermNode : public BinaryTermNode
{
public:
  enum Type
  {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    LESS,
    GREATER,
    LESSEQ,
    GREATEREQ,
    EQ,
    NOTEQ
  };

  BinaryOpTermNode(TermNode * left, TermNode * right, Type type)
    : BinaryTermNode(left, right), _type(type){};
  virtual BinaryOpTermNode * clone() const
  {
    return new BinaryOpTermNode(_left->clone(), _right->clone(), _type);
  };

  virtual std::string stringify() const;
  virtual int precedence() const;

protected:
  Type _type;
};

/// Node representing a function with two arguments
class BinaryFuncTermNode : public BinaryTermNode
{
public:
  enum Type
  {
    MIN,
    MAX,
    ATAN2,
    HYPOT,
    PLOG
  } _type;

  BinaryFuncTermNode(TermNode * left, TermNode * right, Type type)
    : BinaryTermNode(left, right), _type(type){};
  virtual BinaryFuncTermNode * clone() const
  {
    return new BinaryFuncTermNode(_left->clone(), _right->clone(), _type);
  };

  virtual std::string stringify() const;
  virtual int precedence() const { return 2; }
};

/// Base class for nodes with two sub nodes (i.e. functions or operators taking two arguments)
class TernaryTermNode : public BinaryTermNode
{
public:
  TernaryTermNode(TermNode * left, TermNode * middle, TermNode * right)
    : BinaryTermNode(left, right), _middle(middle){};
  virtual ~TernaryTermNode() { delete _middle; };

  virtual unsigned int substitute(const SubstitutionRuleList & rule);

protected:
  TermNode * _middle;
};

/// Node representing a function with three arguments
class TernaryFuncTermNode : public TernaryTermNode
{
public:
  enum Type
  {
    CONDITIONAL
  } _type;

  TernaryFuncTermNode(TermNode * left, TermNode * middle, TermNode * right, Type type)
    : TernaryTermNode(left, middle, right), _type(type){};
  virtual TernaryFuncTermNode * clone() const
  {
    return new TernaryFuncTermNode(_left->clone(), _middle->clone(), _right->clone(), _type);
  };

  virtual std::string stringify() const;
  virtual int precedence() const { return 2; }
};

/**
 * User facing host object for an expression tree. Each Term contains a _root
 * node pointer to an TermNode object. The _root pointer should never be NULL,
 * but it should be safe if it ever is. The default constructor assigns a
 * TempIDNode to _root with a unique ID.
 */
class Term
{
public:
  // the default constructor assigns a temporary id node to root we use the address of the
  // current Term object as the ID. This could be problematic if we create and destroy terms,
  // but then we should not expect the substitution to do sane things anyways.
  Term() : _root(new TempIDNode(reinterpret_cast<unsigned long>(this))){};

  Term(const Term & term) : _root(term.cloneRoot()){};
  ~Term() { delete _root; };

private:
  // construct a term from a node
  Term(TermNode * root) : _root(root){};

public:
  // construct from number or string
  Term(int number) : _root(new NumberNode<int>(number)) {}
  Term(Real number) : _root(new NumberNode<Real>(number)) {}
  Term(const char * symbol) : _root(new SymbolNode(symbol)) {}

  // concatenate terms to form a parameter list with (()) syntax (those need to be out-of-class!)
  friend TermList operator,(const Term & larg, const Term & rarg);
  friend TermList operator,(const Term & larg, const TermList & rargs);
  friend TermList operator,(const TermList & largs, const Term & rarg);

  // dump term as FParser expression
  friend std::ostream & operator<<(std::ostream & os, const Term & term);
  // cast into a string
  operator std::string() const { return _root->stringify(); }

  // assign a term
  Term & operator=(const Term & term)
  {
    delete _root;
    _root = term.cloneRoot();
    return *this;
  }

  // perform a substitution (returns substituton count)
  unsigned int substitute(const SubstitutionRule & rule);
  unsigned int substitute(const SubstitutionRuleList & rules);

  const TermNode * getRoot() const { return _root; }
  TermNode * cloneRoot() const { return _root == NULL ? NULL : _root->clone(); }

protected:
  TermNode * _root;

public:
/**
* Unary operators
*/
#define UNARY_OP_IMPLEMENT(op, OP)                                                                 \
  Term operator op() const                                                                         \
  {                                                                                                \
    assert(_root != NULL);                                                                         \
    /* mooseAssert(_root != NULL, "Empty term provided for unary operator " #op); */               \
    return Term(new UnaryOpTermNode(cloneRoot(), UnaryOpTermNode::OP));                            \
  }
  UNARY_OP_IMPLEMENT(-, NEG)
  UNARY_OP_IMPLEMENT(!, LOGICNOT)

  /**
   * Unary functions
   */
  friend Term sin(const Term &);
  friend Term cos(const Term &);
  friend Term tan(const Term &);
  friend Term abs(const Term &);
  friend Term log(const Term &);
  friend Term log2(const Term &);
  friend Term log10(const Term &);
  friend Term exp(const Term &);
  friend Term sinh(const Term &);
  friend Term cosh(const Term &);

/*
* Binary operators (including number,term operations)
*/
#define BINARY_OP_IMPLEMENT(op, OP)                                                                \
  Term operator op(const Term & term) const                                                        \
  {                                                                                                \
    /*mooseAssert(_root != NULL, "Empty term provided on left side of operator " #op); */          \
    /*mooseAssert(term._root != NULL, "Empty term provided on right side of operator " #op); */    \
    return Term(new BinaryOpTermNode(cloneRoot(), term.cloneRoot(), BinaryOpTermNode::OP));        \
  }                                                                                                \
  friend Term operator op(int left, const Term & right)                                            \
  {                                                                                                \
    /*mooseAssert(right._root != NULL, "Empty term provided on right side of operator " #op); */   \
    return Term(                                                                                   \
        new BinaryOpTermNode(new NumberNode<int>(left), right.cloneRoot(), BinaryOpTermNode::OP)); \
  }                                                                                                \
  friend Term operator op(Real left, const Term & right)                                           \
  {                                                                                                \
    /*mooseAssert(right._root != NULL, "Empty term provided on right side of operator " #op); */   \
    return Term(new BinaryOpTermNode(                                                              \
        new NumberNode<Real>(left), right.cloneRoot(), BinaryOpTermNode::OP));                     \
  }                                                                                                \
  friend Term operator op(const Function & left, const Term & right)                               \
  {                                                                                                \
    /*mooseAssert(Term(left)._root != NULL,"Empty term provided on left side of operator " #op);*/ \
    /*mooseAssert(right._root != NULL, "Empty term provided on right side of operator " #op); */   \
    return Term(                                                                                   \
        new BinaryOpTermNode(Term(left).cloneRoot(), right.cloneRoot(), BinaryOpTermNode::OP));    \
  }                                                                                                \
  friend Term operator op(const Function & left, const Function & right);                          \
  friend Term operator op(int left, const Function & right);                                       \
  friend Term operator op(Real left, const Function & right);
  BINARY_OP_IMPLEMENT(+, ADD)
  BINARY_OP_IMPLEMENT(-, SUB)
  BINARY_OP_IMPLEMENT(*, MUL)
  BINARY_OP_IMPLEMENT(/, DIV)
  BINARY_OP_IMPLEMENT(%, MOD)
  BINARY_OP_IMPLEMENT(<, LESS)
  BINARY_OP_IMPLEMENT(>, GREATER)
  BINARY_OP_IMPLEMENT(<=, LESSEQ)
  BINARY_OP_IMPLEMENT(>=, GREATEREQ)
  BINARY_OP_IMPLEMENT(==, EQ)
  BINARY_OP_IMPLEMENT(!=, NOTEQ)

/*
* Compound assignment operators
*/
#define BINARYCOMP_OP_IMPLEMENT(op, OP)                                                            \
  Term & operator op(const Term & term)                                                            \
  {                                                                                                \
    /*mooseAssert(_root != NULL, "Empty term provided on left side of operator " #op); */          \
    /*mooseAssert(term._root != NULL, "Empty term provided on right side of operator " #op); */    \
    if (dynamic_cast<TempIDNode *>(_root))                                                         \
      mooseError("Using compound assignment operator on anonymous term. Set it to 0 first!");      \
    _root = new BinaryOpTermNode(_root, term.cloneRoot(), BinaryOpTermNode::OP);                   \
    return *this;                                                                                  \
  }
  BINARYCOMP_OP_IMPLEMENT(+=, ADD)
  BINARYCOMP_OP_IMPLEMENT(-=, SUB)
  BINARYCOMP_OP_IMPLEMENT(*=, MUL)
  BINARYCOMP_OP_IMPLEMENT(/=, DIV)
  BINARYCOMP_OP_IMPLEMENT(%=, MOD)

  /**
  * @{
   * Binary functions
   */
  friend Term min(const Term &, const Term &);
  friend Term max(const Term &, const Term &);
  friend Term pow(const Term &, const Term &);
  template <typename T>
  friend Term pow(const Term &, T exponent);
  friend Term atan2(const Term &, const Term &);
  friend Term hypot(const Term &, const Term &);
  friend Term plog(const Term &, const Term &);
  ///@}

  /**
   * Ternary functions
   */
  friend Term conditional(const Term &, const Term &, const Term &);
};

/// User facing host object for a function. This combines a term with an argument list.
class Function
{
public:
  Function(){};

  /// @{
  /// set the temporary argument list which is either used for evaluation
  /// or committed to the argument list upon function definition (assignment)
  Function & operator()(const Term & arg);
  Function & operator()(const TermList & args);
  /// @}

  /// @{
  /// convenience operators to allow single bracket syntax
  Function & operator()(const Term & a1, const Term & a2) { return (*this)((a1, a2)); }
  Function & operator()(const Term & a1, const Term & a2, const Term & a3)
  {
    return (*this)((a1, a2, a3));
  }
  Function & operator()(const Term & a1, const Term & a2, const Term & a3, const Term & a4)
  {
    return (*this)((a1, a2, a3, a4));
  }
  Function &
  operator()(const Term & a1, const Term & a2, const Term & a3, const Term & a4, const Term & a5)
  {
    return (*this)((a1, a2, a3, a4, a5));
  }
  Function & operator()(const Term & a1,
                        const Term & a2,
                        const Term & a3,
                        const Term & a4,
                        const Term & a5,
                        const Term & a6)
  {
    return (*this)((a1, a2, a3, a4, a5, a6));
  }
  Function & operator()(const Term & a1,
                        const Term & a2,
                        const Term & a3,
                        const Term & a4,
                        const Term & a5,
                        const Term & a6,
                        const Term & a7)
  {
    return (*this)((a1, a2, a3, a4, a5, a6, a7));
  }
  Function & operator()(const Term & a1,
                        const Term & a2,
                        const Term & a3,
                        const Term & a4,
                        const Term & a5,
                        const Term & a6,
                        const Term & a7,
                        const Term & a8)
  {
    return (*this)((a1, a2, a3, a4, a5, a6, a7, a8));
  }
  Function & operator()(const Term & a1,
                        const Term & a2,
                        const Term & a3,
                        const Term & a4,
                        const Term & a5,
                        const Term & a6,
                        const Term & a7,
                        const Term & a8,
                        const Term & a9)
  {
    return (*this)((a1, a2, a3, a4, a5, a6, a7, a8, a9));
  }
  /// @}

  /// cast an Function into an Term
  operator Term() const;

  /// cast into a string (via the cast into a term above)
  operator std::string() const;

  /// @{
  /// function definition (assignment)
  Function & operator=(const Term &);
  Function & operator=(const Function &);
  /// @}

  /// get the list of arguments and check if they are all symbols
  std::string args();

  /// @{
  /// Unary operators on functions
  Term operator-() { return -Term(*this); }
  Term operator!() { return !Term(*this); }
  /// @}

  // perform a substitution (returns substituton count)
  unsigned int substitute(const SubstitutionRule & rule);
  unsigned int substitute(const SubstitutionRuleList & rules);

protected:
  /// argument list the function is declared with
  TermList _arguments;
  /// argument list passed in when evaluating the function
  TermList _eval_arguments;

  // underlying term that the _eval_arguments are substituted in
  Term _term;
};

/*
* Binary operators
*/
#define BINARYFUNC_OP_IMPLEMENT(op, OP)                                                            \
  friend Term operator op(const Function & left, const Function & right)                           \
  {                                                                                                \
    mooseAssert(Term(left)._root != NULL, "Empty term provided on left side of operator " #op);    \
    mooseAssert(Term(right)._root != NULL, "Empty term provided on right side of operator " #op);  \
    return Term(new BinaryOpTermNode(                                                              \
        Term(left).cloneRoot(), Term(right).cloneRoot(), BinaryOpTermNode::OP));                   \
  }                                                                                                \
  friend Term operator op(int left, const Function & right)                                        \
  {                                                                                                \
    mooseAssert(Term(right)._root != NULL, "Empty term provided on right side of operator " #op);  \
    return Term(new BinaryOpTermNode(                                                              \
        new NumberNode<int>(left), Term(right).cloneRoot(), BinaryOpTermNode::OP));                \
  }                                                                                                \
  friend Term operator op(Real left, const Function & right)                                       \
  {                                                                                                \
    mooseAssert(Term(right)._root != NULL, "Empty term provided on right side of operator " #op);  \
    return Term(new BinaryOpTermNode(                                                              \
        new NumberNode<Real>(left), Term(right).cloneRoot(), BinaryOpTermNode::OP));               \
  }
BINARYFUNC_OP_IMPLEMENT(+, ADD)
BINARYFUNC_OP_IMPLEMENT(-, SUB)
BINARYFUNC_OP_IMPLEMENT(*, MUL)
BINARYFUNC_OP_IMPLEMENT(/, DIV)
BINARYFUNC_OP_IMPLEMENT(%, MOD)
BINARYFUNC_OP_IMPLEMENT(<, LESS)
BINARYFUNC_OP_IMPLEMENT(>, GREATER)
BINARYFUNC_OP_IMPLEMENT(<=, LESSEQ)
BINARYFUNC_OP_IMPLEMENT(>=, GREATEREQ)
BINARYFUNC_OP_IMPLEMENT(==, EQ)
BINARYFUNC_OP_IMPLEMENT(!=, NOTEQ)
};

// convenience function for numeric exponent
template <typename T>
Term
pow(const Term & left, T exponent)
{
  return Term(
      new BinaryOpTermNode(left.cloneRoot(), new NumberNode<T>(exponent), BinaryOpTermNode::POW));
}

// convert a number node into a string
template <typename T>
std::string
NumberNode<T>::stringify() const
{
  std::ostringstream s;
  s << std::setprecision(12) << _value;
  return s.str();
}

/**
 * Substitution rule functor base class to perform flexible term substitutions
 */
class SubstitutionRule
{
public:
  virtual TermNode * apply(const TermNode *) const = 0;
  virtual ~SubstitutionRule() {}
};

/**
 * Substitution rule base class that applies to nodes of type Node_T
 */
template <class Node_T>
class SubstitutionRuleTyped : public SubstitutionRule
{
public:
  virtual TermNode * apply(const TermNode *) const;

protected:
  // on successful substitution this returns a new node to replace the old one, otherwise it
  // returns NULL
  virtual TermNode * substitute(const Node_T &) const = 0;
};

/**
 * Generic Substitution rule to replace all occurences of a given symbol node
 * term with a user defined term. This is used by Function.
 */
class TermSubstitution : public SubstitutionRuleTyped<SymbolNode>
{
public:
  TermSubstitution(const Term & find, const Term & replace);
  virtual ~TermSubstitution() { delete _replace; }
protected:
  virtual TermNode * substitute(const SymbolNode &) const;
  std::string _find;
  TermNode * _replace;
};

/**
 * Substitution rule to replace all occurences of log(x) with plog(x, epsilon)
 * with a user defined term for epsilon.
 */
class LogPlogSubstitution : public SubstitutionRuleTyped<UnaryFuncTermNode>
{
public:
  LogPlogSubstitution(const Term & epsilon) : _epsilon(epsilon.cloneRoot())
  {
    mooseAssert(_epsilon != NULL, "Epsilon must not be an empty term in LogPlogSubstitution");
  }
  virtual ~LogPlogSubstitution() { delete _epsilon; }
protected:
  virtual TermNode * substitute(const UnaryFuncTermNode &) const;
  TermNode * _epsilon;
};

template <class Node_T>
TermNode *
SubstitutionRuleTyped<Node_T>::apply(const TermNode * node) const
{
  const Node_T * match_node = dynamic_cast<const Node_T *>(node);
  if (match_node == NULL)
    return NULL;
  else
    return substitute(*match_node);
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_TERM_H
