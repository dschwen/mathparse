#include "SymbolicMathTerm.h"

namespace SymbolicMath
{

TermList
operator, (const Term & larg, const Term & rarg)
{
  return {larg, rarg};
}

TermList
operator, (const Term & larg, const TermList & rargs)
{
  TermList list = {larg};
  list.insert(list.end(), rargs.begin(), rargs.end());
  return list;
}

TermList
operator, (const TermList & largs, const Term & rarg)
{
  TermList list = largs;
  list.push_back(rarg);
  return list;
}

std::ostream &
operator<<(std::ostream & os, const Term & term)
{
  if (term._root != NULL)
    return os << *term._root;
  else
    return os << "[NULL]";
}

std::string
SymbolNode::stringify() const
{
  return _symbol;
}

std::string
TempIDNode::stringify() const
{
  std::ostringstream s;
  s << '[' << _id << ']';
  return s.str();
}

std::string
UnaryFuncTermNode::stringify() const
{
  const char * name[] = {"sin", "cos", "tan", "abs", "log", "log2", "log10", "exp", "sinh", "cosh"};
  std::ostringstream s;
  s << name[_type] << '(' << *_subnode << ')';
  return s.str();
}

std::string
UnaryOpTermNode::stringify() const
{
  const char * name[] = {"-", "!"};
  std::ostringstream s;

  s << name[_type];

  if (_subnode->precedence() > precedence())
    s << '(' << *_subnode << ')';
  else
    s << *_subnode;

  return s.str();
}

std::string
BinaryFuncTermNode::stringify() const
{
  const char * name[] = {"min", "max", "atan2", "hypot", "plog"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_right << ')';
  return s.str();
}

std::string
BinaryOpTermNode::stringify() const
{
  const char * name[] = {"+", "-", "*", "/", "%", "^", "<", ">", "<=", ">=", "=", "!="};
  std::ostringstream s;

  if (_left->precedence() > precedence())
    s << '(' << *_left << ')';
  else
    s << *_left;

  s << name[_type];

  // these operators are left associative at equal precedence
  // (this matters for -,/,&,^ but not for + and *)
  if (_right->precedence() > precedence() ||
      (_right->precedence() == precedence() &&
       (_type == SUB || _type == DIV || _type == MOD || _type == POW)))
    s << '(' << *_right << ')';
  else
    s << *_right;

  return s.str();
}

int
BinaryOpTermNode::precedence() const
{
  switch (_type)
  {
    case ADD:
    case SUB:
      return 6;
    case MUL:
    case DIV:
    case MOD:
      return 5;
    case POW:
      return 2;
    case LESS:
    case GREATER:
    case LESSEQ:
    case GREATEREQ:
      return 8;
    case EQ:
    case NOTEQ:
      return 9;
  }

  throw std::exception("Unknown type.");
  // mooseError("Unknown type.");
}

std::string
TernaryFuncTermNode::stringify() const
{
  const char * name[] = {"if"};
  std::ostringstream s;
  s << name[_type] << '(' << *_left << ',' << *_middle << ',' << *_right << ')';
  return s.str();
}

Function &
Function::operator()(const Term & arg)
{
  this->_eval_arguments = {arg};
  return *this;
}

Function &
Function::operator()(const TermList & args)
{
  this->_eval_arguments = TermList(args);
  return *this;
}

Function &
Function::operator=(const Term & term)
{
  this->_arguments = this->_eval_arguments;
  this->_term = term;
  return *this;
}

Function &
Function::operator=(const Function & func)
{
  this->_arguments = this->_eval_arguments;
  this->_term = Term(func);
  return *this;
}

Function::operator Term() const
{
  unsigned int narg = _arguments.size();
  if (narg != _eval_arguments.size())
  {
    throw std::exception(
        "Function is used wth a different number of arguments than it was defined with");
    // mooseError("Function is used wth a different number of arguments than it was defined
    // with.");
  }
  // prepare a copy of the function term to perform the substitution on
  Term result(_term);

  // prepare a rule list for the substitutions
  SubstitutionRuleList rules;
  for (unsigned i = 0; i < narg; ++i)
    rules.push_back(new TermSubstitution(_arguments[i], _eval_arguments[i]));

  // perform substitution
  result.substitute(rules);

  // discard rule set
  for (unsigned i = 0; i < narg; ++i)
    delete rules[i];

  return result;
}

Function::operator std::string() const
{
  Term eval;
  eval = *this; // typecast Function -> Term performs a parameter substitution
  std::ostringstream s;
  s << eval;
  return s.str();
}

std::string
Function::args()
{
  unsigned int narg = _arguments.size();
  if (narg < 1)
    return "";

  std::ostringstream s;
  s << _arguments[0];
  for (unsigned int i = 1; i < narg; ++i)
    s << ',' << _arguments[i];

  return s.str();
}

unsigned int
Function::substitute(const SubstitutionRule & rule)
{
  return _term.substitute(rule);
}

unsigned int
Function::substitute(const SubstitutionRuleList & rules)
{
  return _term.substitute(rules);
}

#define UNARY_FUNC_IMPLEMENT(op, OP)                                                               \
  Term op(const Term & term)                                                                       \
  {                                                                                                \
    assert(term._root != NULL);                                                                    \
    /*mooseAssert(term._root != NULL, "Empty term provided as argument of function " #op "()");*/  \
    return Term(new UnaryFuncTermNode(term.cloneRoot(), UnaryFuncTermNode::OP));                   \
  }
UNARY_FUNC_IMPLEMENT(sin, SIN)
UNARY_FUNC_IMPLEMENT(cos, COS)
UNARY_FUNC_IMPLEMENT(tan, TAN)
UNARY_FUNC_IMPLEMENT(abs, ABS)
UNARY_FUNC_IMPLEMENT(log, LOG)
UNARY_FUNC_IMPLEMENT(log2, LOG2)
UNARY_FUNC_IMPLEMENT(log10, LOG10)
UNARY_FUNC_IMPLEMENT(exp, EXP)
UNARY_FUNC_IMPLEMENT(sinh, SINH)
UNARY_FUNC_IMPLEMENT(cosh, COSH)

#define BINARY_FUNC_IMPLEMENT(op, OP)                                                              \
  Term op(const Term & left, const Term & right)                                                   \
  {                                                                                                \
    assert(left._root != NULL);                                                                    \
    assert(right._root != NULL);                                                                   \
    /*mooseAssert(left._root != NULL, */                                                           \
    /*            "Empty term provided as first argument of function " #op "()"); */               \
    /*mooseAssert(right._root != NULL, */                                                          \
    /*            "Empty term provided as second argument of function " #op "()"); */              \
    return Term(                                                                                   \
        new BinaryFuncTermNode(left.cloneRoot(), right.cloneRoot(), BinaryFuncTermNode::OP));      \
  }
BINARY_FUNC_IMPLEMENT(min, MIN)
BINARY_FUNC_IMPLEMENT(max, MAX)
BINARY_FUNC_IMPLEMENT(atan2, ATAN2)
BINARY_FUNC_IMPLEMENT(hypot, HYPOT)
BINARY_FUNC_IMPLEMENT(plog, PLOG)

// this is a function in ExpressionBuilder (pow) but an operator in FParser (^)
Term
pow(const Term & left, const Term & right)
{
  assert(left._root != NULL);
  assert(right._root != NULL);
  // mooseAssert(left._root != NULL, "Empty term for base of pow()");
  // mooseAssert(right._root != NULL, "Empty term for exponent of pow()");
  return Term(new BinaryOpTermNode(left.cloneRoot(), right.cloneRoot(), BinaryOpTermNode::POW));
}

#define TERNARY_FUNC_IMPLEMENT(op, OP)                                                             \
  Term op(const Term & left, const Term & middle, const Term & right)                              \
  {                                                                                                \
    assert(left._root != NULL);                                                                    \
    assert(middle._root != NULL);                                                                  \
    assert(right._root != NULL);                                                                   \
    /*mooseAssert(left._root != NULL, */                                                           \
    /*            "Empty term provided as first argument of the ternary function " #op "()"); */   \
    /*mooseAssert(middle._root != NULL, */                                                         \
    /*            "Empty term provided as second argument of the ternary function " #op "()"); */  \
    /*mooseAssert(right._root != NULL, */                                                          \
    /*            "Empty term provided as third argument of the ternary function " #op "()"); */   \
    return Term(new TernaryFuncTermNode(                                                           \
        left.cloneRoot(), middle.cloneRoot(), right.cloneRoot(), TernaryFuncTermNode::OP));        \
  }
TERNARY_FUNC_IMPLEMENT(conditional, CONDITIONAL)

unsigned int
UnaryTermNode::substitute(const SubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();

  for (unsigned int i = 0; i < nrule; ++i)
  {
    TermNode * replace = rules[i]->apply(_subnode);
    if (replace != NULL)
    {
      delete _subnode;
      _subnode = replace;
      return 1;
    }
  }

  return _subnode->substitute(rules);
}

unsigned int
BinaryTermNode::substitute(const SubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();
  unsigned int success = 0;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    TermNode * replace = rules[i]->apply(_left);
    if (replace != NULL)
    {
      delete _left;
      _left = replace;
      success = 1;
      break;
    }
  }

  if (success == 0)
    success += _left->substitute(rules);

  for (unsigned int i = 0; i < nrule; ++i)
  {
    TermNode * replace = rules[i]->apply(_right);
    if (replace != NULL)
    {
      delete _right;
      _right = replace;
      return success + 1;
    }
  }

  return success + _right->substitute(rules);
}

unsigned int
TernaryTermNode::substitute(const SubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();
  bool left_success = false, middle_success = false, right_success = false;
  TermNode * replace;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(_left);
    if (replace)
    {
      delete _left;
      _left = replace;
      left_success = true;
      break;
    }
  }

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(_middle);
    if (replace)
    {
      delete _middle;
      _middle = replace;
      middle_success = true;
      break;
    }
  }

  for (unsigned int i = 0; i < nrule; ++i)
  {
    replace = rules[i]->apply(_right);
    if (replace)
    {
      delete _right;
      _right = replace;
      right_success = true;
      break;
    }
  }

  if (!left_success)
    left_success = _left->substitute(rules);
  if (!middle_success)
    middle_success = _middle->substitute(rules);
  if (!right_success)
    right_success = _right->substitute(rules);

  return left_success + middle_success + right_success;
}

unsigned int
Term::substitute(const SubstitutionRule & rule)
{
  SubstitutionRuleList rules(1);
  rules[0] = &rule;
  return substitute(rules);
}

unsigned int
Term::substitute(const SubstitutionRuleList & rules)
{
  unsigned int nrule = rules.size();

  if (_root == NULL)
    return 0;

  for (unsigned int i = 0; i < nrule; ++i)
  {
    TermNode * replace = rules[i]->apply(_root);
    if (replace != NULL)
    {
      delete _root;
      _root = replace;
      return 1;
    }
  }

  return _root->substitute(rules);
}

TermSubstitution::TermSubstitution(const Term & find, const Term & replace)
{
  // the expression we want to substitute (has to be a symbol node)
  const SymbolNode * find_root = dynamic_cast<const SymbolNode *>(find.getRoot());
  if (find_root == NULL)
  {
    throw std::exeption("Function arguments must be pure symbols");
    // mooseError("Function arguments must be pure symbols.");
  }
  _find = find_root->stringify();

  // the term we want to substitute with
  if (replace.getRoot() != NULL)
    _replace = replace.cloneRoot();
  else
  {
    throw std::exeption("Trying to substitute in an empty term");
    // mooseError("Trying to substitute in an empty term for ", _find);
  }
}

TermNode *
TermSubstitution::substitute(const SymbolNode & node) const
{
  if (node.stringify() == _find)
    return _replace->clone();
  else
    return NULL;
}

TermNode *
LogPlogSubstitution::substitute(const UnaryFuncTermNode & node) const
{
  if (node._type == UnaryFuncTermNode::LOG)
    return new BinaryFuncTermNode(
        node.getSubnode()->clone(), _epsilon->clone(), BinaryFuncTermNode::PLOG);
  else
    return NULL;
}

// end namespace SymbolicMath
}
