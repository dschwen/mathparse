#include "SymbolicMathParser.h"

#include <iostream>
#include <string>
#include <vector>

namespace SymbolicMath
{

Parser::Parser() {}

std::unique_ptr<Tree>
Parser::parse(const std::string & expression)
{
  Tokenizer tokenizer(expression);
  _expression = expression;

  std::stack<Token> operator_stack;

  // process tokens
  do
  {
    _token = tokenizer.getToken();
    preprocessToken();
    validateToken();

    //
    // Shunting yard core
    //

    if (_token._type == TokenType::NUMBER || _token._type == TokenType::VARIABLE)
      pushToOutput(_token);
    else if (_token._type == TokenType::OPERATOR)
    {
      auto precedence = _operators[static_cast<int>(_token._operator_type)]._precedence;
      while (!operator_stack.empty() && operator_stack.top()._type == TokenType::OPERATOR &&
             ((operatorProperty(operator_stack.top()._operator_type)._precedence <= precedence &&
               operatorProperty(operator_stack.top()._operator_type)._left_associative) ||
              (operatorProperty(operator_stack.top()._operator_type)._precedence < precedence &&
               operatorProperty(operator_stack.top()._operator_type)._unary)))
      {
        pushToOutput(operator_stack.top());
        operator_stack.pop();
      }
      operator_stack.push(_token);
    }
    else if (_token._type == TokenType::FUNCTION)
      operator_stack.push(_token);
    else if (_token._type == TokenType::OPENING_BRACKET)
      operator_stack.push(_token);
    else if (_token._type == TokenType::CLOSING_BRACKET)
    {
      if (_last_token._type != TokenType::OPENING_BRACKET)
      {
        // bracket pair containing an expression
        while (!operator_stack.empty() && operator_stack.top()._type != TokenType::OPENING_BRACKET)
        {
          pushToOutput(operator_stack.top());
          operator_stack.pop();
        }
        if (operator_stack.empty())
        {
          std::cerr << formatError("Unmatched closing bracket");
          throw std::domain_error("parenthesis");
        }
        auto open_parens = operator_stack.top();
        operator_stack.pop();

        // check if this bracket pair was a function argument list
        if (!operator_stack.empty() && operator_stack.top()._type == TokenType::FUNCTION)
        {
          auto function = operator_stack.top();
          auto expected_argments = functionProperty(function._function_type)._arguments;
          if (expected_argments != open_parens._integer + 1)
          {
            {
              std::cerr << formatError(function._pos,
                                       "Expected " + std::to_string(expected_argments) +
                                           " argument(s) but found " +
                                           std::to_string(open_parens._integer + 1));
              throw std::domain_error("parenthesis");
            }
          }
          pushFunctionToOutput(operator_stack.top(), expected_argments);
          operator_stack.pop();
        }
      }
      else
      {
        // empty bracket pair
        auto open_parens = operator_stack.top();
        if (open_parens._type != TokenType::OPENING_BRACKET)
        {
          std::cerr << "Internal error\n";
          throw std::domain_error("parenthesis");
        }
        operator_stack.pop();

        if (operator_stack.empty() || operator_stack.top()._type != TokenType::FUNCTION)
        {
          std::cerr << formatError(open_parens._pos,
                                   "Empty bracket pairs are only allowed after functions");
          throw std::domain_error("parenthesis");
        }

        auto function = operator_stack.top();
        auto expected_argments = functionProperty(function._function_type)._arguments;
        if (expected_argments != 0)
        {
          std::cerr << formatError(function._pos,
                                   "Function takes " + std::to_string(expected_argments) +
                                       " argument(s), but none were given");
          throw std::domain_error("parenthesis");
        }
        pushFunctionToOutput(operator_stack.top(), 0);
        operator_stack.pop();
      }
    }
    else if (_token._type == TokenType::COMMA)
    {
      while (!operator_stack.empty() && operator_stack.top()._type != TokenType::OPENING_BRACKET)
      {
        pushToOutput(operator_stack.top());
        operator_stack.pop();
      }
      if (operator_stack.empty())
      {
        std::cerr << formatError("Comma outside of brackets");
        throw std::domain_error("parenthesis");
      }

      // count the function arguments encountered for validation purposes
      // assert(operator_stack.top()._type != TokenType::OPENING_BRACKET)
      operator_stack.top()._integer++;
    }

    // needed to discriminate unary plus and minus
    _last_token = _token;
  } while (_token._type != TokenType::END);

  // unwind operator stack
  while (!operator_stack.empty())
  {
    auto _token = operator_stack.top();
    if (_token._type == TokenType::OPENING_BRACKET)
    {
      std::cerr << formatError("Unmatched opening bracket");
      throw std::domain_error("parenthesis");
    }

    pushToOutput(_token);
    operator_stack.pop();
  }

  return std::move(_output_stack.top());
}

void
Parser::pushToOutput(const Token & token)
{
  switch (token._type)
  {
    case TokenType::NUMBER:
      _output_stack.push(std::move(std::unique_ptr<Tree>(new Tree(token._real))));
      return;

    case TokenType::OPERATOR:
    {
      auto right = std::move(_output_stack.top());
      _output_stack.pop();

      if (operatorProperty(token._operator_type)._unary)
      {
        _output_stack.push(
            std::move(std::unique_ptr<Tree>(new Tree(token._operator_type, {right.release()}))));
      }
      else
      {
        auto left = std::move(_output_stack.top());
        _output_stack.pop();

        _output_stack.push(std::move(std::unique_ptr<Tree>(
            new Tree(token._operator_type, {left.release(), right.release()}))));
      }
      return;
    }

    default:
      throw std::domain_error("invalid_token");
  }
}

void
Parser::pushFunctionToOutput(const Token & token, unsigned int num_arguments)
{
  if (token._type != TokenType::FUNCTION)
    throw std::domain_error("invalid_token");

  std::vector<Tree *> arguments;
  for (unsigned int i = 0; i < num_arguments; ++i)
  {
    arguments.push_back(_output_stack.top().release());
    _output_stack.pop();
  }
  _output_stack.push(std::move(std::unique_ptr<Tree>(new Tree(token._function_type, arguments))));
}

void
Parser::preprocessToken()
{
  // operator preprocessing
  if (_token._type == TokenType::OPERATOR)
  {
    // error on invalid operators
    if (_token._operator_type == OperatorType::INVALID)
    {
      std::cerr << formatError("Unknown operator");
      throw std::domain_error("operator");
    }

    // preprocess operators to distinguish unary plus/minus from addition/subtraction
    if (_last_token._type != TokenType::NUMBER && _last_token._type != TokenType::VARIABLE &&
        _last_token._type != TokenType::CLOSING_BRACKET)
    {
      // turn addition into unary plus and subtraction into unary minus
      if (_token._operator_type == OperatorType::ADDITION)
        _token._operator_type = OperatorType::UNARY_PLUS;
      else if (_token._operator_type == OperatorType::SUBTRACTION)
        _token._operator_type = OperatorType::UNARY_MINUS;
      else
      {
        std::cerr << formatError(_token._pos, "Did not expect operator here");
        throw std::domain_error("operator");
      }
    }
  }
}

void
Parser::validateToken()
{
  // check if token is invalid
  if (_token._type == TokenType::INVALID)
  {
    std::cerr << formatError("Parse error");
    throw std::domain_error("invalid");
  }

  // function checking
  if (_token._type == TokenType::FUNCTION && _token._function_type == FunctionType::INVALID)
  {
    std::cerr << formatError("Unknown function");
    throw std::domain_error("operator");
  }

  // disallow consecutive variables/numbers (we could insert a multiplication here...)
  if ((_token._type == TokenType::VARIABLE || _token._type == TokenType::NUMBER ||
       _token._type == TokenType::FUNCTION) &&
      (_last_token._type == TokenType::VARIABLE || _last_token._type == TokenType::NUMBER))
  {
    std::cerr << formatError("Operator expected here");
    throw std::domain_error("operator");
  }

  // check closing bracket state
  if (_token._type == TokenType::CLOSING_BRACKET &&
      (_last_token._type == TokenType::OPERATOR || _last_token._type == TokenType::COMMA))
  {
    std::cerr << formatError("Did not expect closing bracket here");
    throw std::domain_error("operator");
  }
}

std::string
Parser::formatToken()
{
  return formatToken(_token);
}

std::string
Parser::formatToken(const Token & token)
{
  switch (token._type)
  {
    case TokenType::OPERATOR:
      return "OPERATOR    \t" + operatorProperty(token._operator_type)._form + " (" +
             std::to_string(operatorProperty(token._operator_type)._precedence) + ')';
    case TokenType::OPENING_BRACKET:
      return "OPENING_BRACKET \t" + token._string;
    case TokenType::CLOSING_BRACKET:
      return "CLOSING_BRACKET\t" + token._string;
    case TokenType::FUNCTION:
      return "FUNCTION    \t" + functionProperty(token._function_type)._form;
    case TokenType::VARIABLE:
      return "VARIABLE    \t" + token._string;
    case TokenType::NUMBER:
      return "NUMBER      \t" + std::to_string(token._real);
    case TokenType::COMMA:
      return "COMMA       \t,";
    default:
      return "???";
  }
}

std::string
Parser::formatError(const std::string & message, std::size_t width)
{
  return formatError(_token._pos, message, width);
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
