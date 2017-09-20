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

    // shunting yard core
    if (token._type == TokenType::NUMBER || token._type == TokenType::VARIABLE)
      output_stack.push(token);
    else if (token._type == TokenType::OPERATOR)
    {
      // operator precedence
      operator_stack.push(token);
    }
    if (token._type == TokenType::OPEN_PARENS)
      operator_stack.push(token);
    if (token._type == TokenType::CLOSE_PARENS)
    {
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
      operator_stack.pop();
    }

    // needed to discriminate unary plus and minus
    last_token = token;
  } while (token._type != TokenType::END);

  // unwind stacks
  // std::cout << type[token._type] << "\t'" << token._data << "'\n";
  // if (token._data == "ERR")
  //   std::cerr << formatError(token._pos, "Invalid token BLAH");
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
