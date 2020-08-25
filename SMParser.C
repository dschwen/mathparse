///
/// SymbolicMath toolkit
/// (c) 2017-2020 by Daniel Schwen
///

#include "SMParser.h"
#include "SMNodeData.h"

#include <iostream>
#include <string>
#include <vector>

namespace SymbolicMath
{

template <typename T>
Parser<T>::Parser() : _qp_ptr(nullptr)
{
}

template <typename T>
Function<T>
Parser<T>::parse(const std::string & expression)
{
  Tokenizer<T> tokenizer(expression);
  _expression = expression;
  _last_token = TokenPtr<T>(new InvalidToken<T>(0));

  std::stack<TokenPtr<T>> operator_stack;
  std::stack<unsigned short> argument_count_stack;

  // process tokens
  do
  {
    _token = tokenizer.getToken();
    preprocessToken();
    validateToken();

    // std::cout << formatToken() << '\n';

    //
    // Shunting yard core
    //

    if (_token->isNumber() || _token->isSymbol())
      pushToOutput(_token);
    else if (_token->isOperator())
    {
      auto precedence = _token->precedence();
      while (!operator_stack.empty() && operator_stack.top()->isOperator() &&
             ((operator_stack.top()->precedence() <= precedence &&
               operator_stack.top()->isLeftAssociative()) ||
              (operator_stack.top()->precedence() < precedence && operator_stack.top()->isUnary())))
      {
        pushToOutput(operator_stack.top());
        operator_stack.pop();
      }
      operator_stack.push(_token);
    }
    else if (_token->isFunction())
    {
      operator_stack.push(_token);
      argument_count_stack.push(1);
    }
    else if (_token->isOpeningBracket())
      operator_stack.push(_token);
    else if (_token->isClosingBracket())
    {
      if (!_last_token->isOpeningBracket())
      {
        // bracket pair containing an expression
        while (!operator_stack.empty() &&
               (!operator_stack.top()->isOpeningBracket() ||
                operator_stack.top()->bracketType() != _token->bracketType()))
        {
          pushToOutput(operator_stack.top());
          operator_stack.pop();
        }
        if (operator_stack.empty())
          fatalError(formatError("Unmatched closing bracket"));

        auto open_parens = operator_stack.top();
        operator_stack.pop();

        // check if this bracket pair was a function argument list
        if (!operator_stack.empty())
        {
          if (operator_stack.top()->isFunction())
          {
            auto function = operator_stack.top();
            auto expected_argments = function->arguments();
            auto provided_arguments = argument_count_stack.top();
            argument_count_stack.pop();

            if (expected_argments != provided_arguments)
              fatalError(formatError(function->pos(),
                                     "Expected " + std::to_string(expected_argments) +
                                         " argument(s) but found " +
                                         std::to_string(provided_arguments)));

            pushFunctionToOutput(operator_stack.top(), expected_argments);
            operator_stack.pop();
          }
          else if (operator_stack.top()->isSymbol())
            fatalError("Component operator not implemented yet\n");
        }
      }
      else
      {
        // empty bracket pair
        auto open_parens = operator_stack.top();
        if (!open_parens->isOpeningBracket())
          fatalError("Internal error\n");
        if (open_parens->bracketType() != _token->bracketType())
          fatalError(formatError(open_parens->pos(), "Mismatching empty bracket pair"));
        if (_token->bracketType() != BracketType::ROUND)
          fatalError(formatError(open_parens->pos(), "Invalid empty bracket pair"));

        argument_count_stack.pop();
        operator_stack.pop();

        if (operator_stack.empty() || !operator_stack.top()->isFunction())
          fatalError(formatError(open_parens->pos(),
                                 "Empty round bracket pairs are only allowed after functions"));

        auto function = operator_stack.top();
        auto expected_argments = function->arguments();
        if (expected_argments != 0)
          fatalError(formatError(function->pos(),
                                 "Function takes " + std::to_string(expected_argments) +
                                     " argument(s), but none were given"));

        pushFunctionToOutput(operator_stack.top(), 0);
        operator_stack.pop();
      }
    }
    else if (_token->isComma())
    {
      while (!operator_stack.empty() && !operator_stack.top()->isOpeningBracket())
      {
        pushToOutput(operator_stack.top());
        operator_stack.pop();
      }
      if (operator_stack.empty())
        fatalError(formatError("Comma outside of brackets"));

      // count the function arguments encountered for validation purposes
      if (argument_count_stack.empty())
        fatalError("argument count stack is empty");
      argument_count_stack.top()++;
    }

    // needed to discriminate unary plus and minus
    _last_token = _token;
  } while (!_last_token->isEnd());

  // unwind operator stack
  while (!operator_stack.empty())
  {
    auto & _token = operator_stack.top();
    if (_token->isOpeningBracket())
      fatalError(formatError("Unmatched opening bracket"));

    pushToOutput(_token);
    operator_stack.pop();
  }

  return Function<T>(_output_stack.top());
}

template <typename T>
void
Parser<T>::pushToOutput(TokenPtr<T> token)
{
  // std::cout << "PUSHING " << formatToken(token) << '\n';

  if (token->isNumber())
  {
    _output_stack.push(Node<T>(token->asNumber()));
    return;
  }
  else if (token->isOperator())
  {
    _output_stack.push(Node<T>(token->node(_output_stack)));
    return;
  }

  else if (token->isSymbol())
  {
    // value provider (variable etc.)
    auto vp = _value_providers.find(token->asString());
    if (vp != _value_providers.end())
    {
      _output_stack.push(Node<T>(vp->second->clone()));
      return;
    }

    // constant
    auto co = _constants.find(token->asString());
    if (co != _constants.end())
    {
      _output_stack.push(Node<T>(co->second));
      return;
    }

    // local variable
    auto lv = _local_variables.find(token->asString());
    if (lv == _local_variables.end())
    {
      auto ret = _local_variables.insert(std::make_pair(
          token->asString(), std::make_shared<LocalVariableData<T>>(_local_variables.size())));
      if (!ret.second)
        fatalError("unable to create local variable");
      lv = ret.first;
    }
    _output_stack.push(Node<T>(lv->second));
  }

  else
    fatalError("invalid_token");
}

template <typename T>
void
Parser<T>::pushFunctionToOutput(TokenPtr<T> token, unsigned int num_arguments)
{
  if (!token->isFunction())
    fatalError("invalid_token");

  _output_stack.push(Node<T>(token->node(_output_stack)));
}

template <typename T>
std::shared_ptr<ValueProvider<T>>
Parser<T>::registerValueProvider(std::string name)
{
  auto vp = std::make_shared<SymbolData<T>>(name);
  registerValueProvider(vp);
  return vp;
}

template <typename T>
void
Parser<T>::registerValueProvider(std::shared_ptr<ValueProvider<T>> vp)
{
  if (vp->_name == "")
    fatalError("Value provider has an empty name.");

  auto it = _value_providers.find(vp->_name);
  if (it != _value_providers.end())
    fatalError("Value provider '" + vp->_name + "' is already registered.");

  _value_providers[vp->_name] = vp;
}

template <typename T>
void
Parser<T>::registerConstant(const std::string & name, T value)
{
  if (name == "")
    fatalError("Constant has an empty name.");

  auto it = _constants.find(name);
  if (it != _constants.end())
    fatalError("Constant '" + name + "' is already registered.");

  _constants[name] = value;
}

template <typename T>
void
Parser<T>::preprocessToken()
{
  // operator preprocessing
  if (_token->isOperator())
  {
    // error on invalid operators
    if (_token->isInvalid())
      fatalError(formatError("Unknown operator"));

    // preprocess operators to distinguish unary plus/minus from addition/subtraction
    if (!_last_token->isNumber() && !_last_token->isSymbol() && !_last_token->isClosingBracket())
    {
      // turn addition into unary plus and subtraction into unary minus
      if (_token->is(MultinaryOperatorType::ADDITION))
        _token = UnaryOperatorToken<T>::build(UnaryOperatorType::PLUS, _token->pos());
      else if (_token->is(BinaryOperatorType::SUBTRACTION))
        _token = UnaryOperatorToken<T>::build(UnaryOperatorType::MINUS, _token->pos());
      else
        fatalError(formatError("Did not expect operator here"));
    }
  }
}

template <typename T>
void
Parser<T>::validateToken()
{
  // invalid function checking
  if (_token->isFunction() && _token->isInvalid())
    fatalError(formatError("Unknown function"));

  // disallow consecutive variables/numbers (we could insert a multiplication here...)
  if ((_token->isSymbol() || _token->isNumber() || _token->isFunction()) &&
      (_last_token->isSymbol() || _last_token->isNumber()))
    fatalError(formatError("Operator expected here"));

  // check closing bracket state
  if (_token->isClosingBracket() && (_last_token->isOperator() || _last_token->isComma()))
    fatalError(formatError("Did not expect closing bracket here"));

  // lastly check if token is completely invalid
  if (_token->isInvalid())
    fatalError(formatError("Parse error"));
}

template <typename T>
std::string
Parser<T>::formatToken()
{
  return formatToken(_token);
}

template <typename T>
std::string
Parser<T>::formatToken(TokenPtr<T> token)
{
  if (token->isOperator())
    return "OPERATOR    \t" + token->asString() + " (" + std::to_string(token->precedence()) + ')';
  else if (token->isOpeningBracket())
    return "OPENING_BRACKET \t"; // + token->asString();
  else if (token->isClosingBracket())
    return "CLOSING_BRACKET\t"; // + token->asString();
  else if (token->isFunction())
    return "FUNCTION    \t" + token->asString();
  else if (token->isSymbol())
    return "VARIABLE    \t" + token->asString();
  else if (token->isNumber())
    return "NUMBER      \t" + std::to_string(token->asNumber());
  else if (token->isComma())
    return "COMMA       \t,";
  else if (token->isEnd())
    return "[END]";
  else if (token->isInvalid())
    return "[INVALID]";
  else
    return "???";
}

template <typename T>
std::string
Parser<T>::formatError(const std::string & message, std::size_t width)
{
  return formatError(_token->pos(), message, width);
}

template <typename T>
std::string
Parser<T>::formatError(std::size_t pos, const std::string & message, std::size_t width)
{
  // pad the expression to allow displaying pos 0 error markers
  const std::string padded = "  " + _expression;
  pos += 2;

  // half the width is where we center the error
  const std::size_t half = width / 2;

  // center the error
  const std::size_t head = std::min(half, pos);

  // align the error message
  const std::size_t indent = message.size() / 2 > head ? 0 : head - message.size() / 2;

  // build the error
  std::string error = padded.substr(pos - head, width) + '\n';
  error += std::string(head - 2, ' ') + "~~^~~\n";
  error += std::string(indent, ' ') + message + '\n';

  return error;
}

template class Parser<Real>;

} // namespace SymbolicMath
