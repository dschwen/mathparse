#include "SymbolicMathParser.h"
#include "SymbolicMathNodeData.h"

#include <iostream>
#include <string>
#include <vector>

namespace SymbolicMath
{

Parser::Parser() : _qp_ptr(nullptr) {}

Function
Parser::parse(const std::string & expression)
{
  Tokenizer tokenizer(expression);
  _expression = expression;
  _last_token.reset(new InvalidToken(0));

  std::stack<TokenPtr> operator_stack;
  std::stack<unsigned short> argument_count_stack;

  // process tokens
  do
  {
    _token.reset(tokenizer.getToken());
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
  } while (!_token->isEnd());

  // unwind operator stack
  while (!operator_stack.empty())
  {
    auto _token = operator_stack.top();
    if (_token->isOpeningBracket())
      fatalError(formatError("Unmatched opening bracket"));

    pushToOutput(_token);
    operator_stack.pop();
  }

  return Function(_output_stack.top());
}

void
Parser::pushToOutput(TokenPtr token)
{
  // std::cout << "PUSHING " << formatToken(token) << '\n';

  if (token->isNumber())
  {
    _output_stack.push(Node(token->asNumber()));
    return;
  }
  else if (token->isOperator())
  {
    _output_stack.push(Node(token->node(_output_stack)));
    return;
  }

  else if (token->isSymbol())
  {
    // value provider (variable etc.)
    auto vp = _value_providers.find(token->asString());
    if (vp != _value_providers.end())
    {
      _output_stack.push(Node(vp->second->clone()));
      return;
    }

    // constant
    auto co = _constants.find(token->asString());
    if (co != _constants.end())
    {
      _output_stack.push(Node(co->second));
      return;
    }

    fatalError(formatError(token->pos(), "Unknown value provider or constant name"));
  }

  else
    fatalError("invalid_token");
}

void
Parser::pushFunctionToOutput(TokenPtr token, unsigned int num_arguments)
{
  if (!token->isFunction())
    fatalError("invalid_token");

  _output_stack.push(Node(token->node(_output_stack)));
}

std::shared_ptr<ValueProvider>
Parser::registerValueProvider(std::string name)
{
  auto vp = std::make_shared<SymbolData>(name);
  registerValueProvider(vp);
  return vp;
}

void
Parser::registerValueProvider(std::shared_ptr<ValueProvider> vp)
{
  if (vp->_name == "")
    fatalError("Value provider has an empty name.");

  auto it = _value_providers.find(vp->_name);
  if (it != _value_providers.end())
    fatalError("Value provider '" + vp->_name + "' is already registered.");

  _value_providers[vp->_name] = vp;
}

void
Parser::registerConstant(const std::string & name, Real value)
{
  if (name == "")
    fatalError("Constant has an empty name.");

  auto it = _constants.find(name);
  if (it != _constants.end())
    fatalError("Constant '" + name + "' is already registered.");

  _constants[name] = value;
}

void
Parser::preprocessToken()
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
        _token.reset(UnaryOperatorToken::build(UnaryOperatorType::PLUS, _token->pos()));
      else if (_token->is(BinaryOperatorType::SUBTRACTION))
        _token.reset(UnaryOperatorToken::build(UnaryOperatorType::MINUS, _token->pos()));
      else
        fatalError(formatError("Did not expect operator here"));
    }
  }
}

void
Parser::validateToken()
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

std::string
Parser::formatToken()
{
  return formatToken(_token);
}

std::string
Parser::formatToken(TokenPtr token)
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

std::string
Parser::formatError(const std::string & message, std::size_t width)
{
  return formatError(_token->pos(), message, width);
}

std::string
Parser::formatError(std::size_t pos, const std::string & message, std::size_t width)
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

// end namespace SymbolicMath
}
