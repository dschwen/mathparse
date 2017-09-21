#include "MathParseAST.h"

#include <iostream>
#include <string>
#include <vector>
#include <stack>

MathParseAST::MathParseAST(const std::string expression)
  : MathParseTokenizer(expression), _mpa_expression(expression)
{
  std::stack<Token> output_stack;
  std::stack<Token> operator_stack;

  // process tokens
  Token token;
  Token last_token;
  do
  {
    token = getToken();
    // std::cout << "got token " << formatToken(token) << '\n';

    //
    // Validation
    //

    // check if token is invalid
    if (token._type == TokenType::INVALID)
    {
      std::cerr << formatError(token._pos, "Parse error");
      throw std::domain_error("invalid");
    }

    // operator preprocessing
    if (token._type == TokenType::OPERATOR)
    {
      // error on invalid operators
      if (token._operator_type == OperatorType::INVALID)
      {
        std::cerr << formatError(token._pos, "Unknown operator");
        throw std::domain_error("operator");
      }

      // preprocess operators to distinguish unary plus/minus from addition/subtraction
      if (last_token._type != TokenType::NUMBER && last_token._type != TokenType::VARIABLE &&
          last_token._type != TokenType::CLOSE_PARENS)
      {
        // turn addition into unary plus and subtraction into unary minus
        if (token._operator_type == OperatorType::ADDITION)
          token._operator_type = OperatorType::UNARY_PLUS;
        else if (token._operator_type == OperatorType::SUBTRACTION)
          token._operator_type = OperatorType::UNARY_MINUS;
        else
        {
          std::cerr << formatError(token._pos, "Did not expect operator here");
          throw std::domain_error("operator");
        }
      }
    }

    // function checking
    if (token._type == TokenType::FUNCTION && token._function_type == FunctionType::INVALID)
    {
      std::cerr << formatError(token._pos, "Unknown function");
      throw std::domain_error("operator");
    }

    // disallow consecutive variables/numbers (we could insert a multiplication here...)
    if ((token._type == TokenType::VARIABLE || token._type == TokenType::NUMBER ||
         token._type == TokenType::FUNCTION) &&
        (last_token._type == TokenType::VARIABLE || last_token._type == TokenType::NUMBER))
    {
      std::cerr << formatError(token._pos, "Operator expected here");
      throw std::domain_error("operator");
    }

    // check closing bracket state
    if (token._type == TokenType::CLOSE_PARENS &&
        (last_token._type == TokenType::OPERATOR || last_token._type == TokenType::COMMA))
    {
      std::cerr << formatError(token._pos, "Did not expect closing bracket here");
      throw std::domain_error("operator");
    }

    // need to validate symbols

    //
    // Shunting yard core
    //

    if (token._type == TokenType::NUMBER || token._type == TokenType::VARIABLE)
      output_stack.push(token);
    else if (token._type == TokenType::OPERATOR)
    {
      auto precedence = _operators[static_cast<int>(token._operator_type)]._precedence;
      while (!operator_stack.empty() && operator_stack.top()._type == TokenType::OPERATOR &&
             operatorProperty(operator_stack.top()._operator_type)._precedence <= precedence &&
             operatorProperty(operator_stack.top()._operator_type)._left_associative)
      {
        output_stack.push(operator_stack.top());
        operator_stack.pop();
      }
      operator_stack.push(token);
    }
    else if (token._type == TokenType::FUNCTION)
      operator_stack.push(token);
    else if (token._type == TokenType::OPEN_PARENS)
      operator_stack.push(token);
    else if (token._type == TokenType::CLOSE_PARENS)
    {
      if (last_token._type != TokenType::OPEN_PARENS)
      {
        // bracket pair containing an expression
        while (!operator_stack.empty() && operator_stack.top()._type != TokenType::OPEN_PARENS)
        {
          output_stack.push(operator_stack.top());
          operator_stack.pop();
        }
        if (operator_stack.empty())
        {
          std::cerr << formatError(token._pos, "Unmatched closing bracket");
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
          output_stack.push(operator_stack.top());
          operator_stack.pop();
        }
      }
      else
      {
        // empty bracket pair
        auto open_parens = operator_stack.top();
        if (open_parens._type != TokenType::OPEN_PARENS)
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
        output_stack.push(operator_stack.top());
        operator_stack.pop();
      }
    }
    else if (token._type == TokenType::COMMA)
    {
      while (!operator_stack.empty() && operator_stack.top()._type != TokenType::OPEN_PARENS)
      {
        output_stack.push(operator_stack.top());
        operator_stack.pop();
      }
      if (operator_stack.empty())
      {
        std::cerr << formatError(token._pos, "Comma outside of brackets");
        throw std::domain_error("parenthesis");
      }

      // count the function arguments encountered for validation purposes
      // assert(operator_stack.top()._type != TokenType::OPEN_PARENS)
      operator_stack.top()._integer++;
    }

    // needed to discriminate unary plus and minus
    last_token = token;
  } while (token._type != TokenType::END);

  // unwind operator stack
  while (!operator_stack.empty())
  {
    auto token = operator_stack.top();
    if (token._type == TokenType::OPEN_PARENS)
    {
      std::cerr << formatError(token._pos, "Unmatched opening bracket");
      throw std::domain_error("parenthesis");
    }

    output_stack.push(token);
    operator_stack.pop();
  }

  // display output stack
  while (!output_stack.empty())
  {
    auto token = output_stack.top();
    output_stack.pop();
    std::cout << formatToken(token) << '\n';
  }
}

std::string
MathParseAST::formatToken(const Token & token)
{
  switch (token._type)
  {
    case TokenType::OPERATOR:
      return "OPERATOR    \t" + operatorProperty(token._operator_type)._form + " (" + std::to_string(static_cast<int>(token._operator_type)) + ')';
    case TokenType::OPEN_PARENS:
      return "OPEN_PARENS \t" + token._string;
    case TokenType::CLOSE_PARENS:
      return "CLOSE_PARENS\t" + token._string;
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
MathParseAST::formatError(std::size_t pos, const std::string & message, std::size_t width)
{
  // pad the expression to allow displaying pos 0 error markers
  const std::string padded = "  " + _mpa_expression;
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
