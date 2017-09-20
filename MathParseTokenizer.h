#ifndef MATHPARSETOKENIZER_H
#define MATHPARSETOKENIZER_H

#include <string>

class MathParseTokenizer
{
public:
  MathParseTokenizer(const std::string expression);

  enum TokenType
  {
    OPERATOR,
    FUNCTION,
    COMMA,
    OPEN_PARENS,
    CLOSE_PARENS,
    NUMBER,
    VARIABLE,
    INVALID,
    END
  };

  struct Token
  {
    Token() : _type(INVALID), _data() {}
    Token(TokenType type, const std::string& data) : _type(type), _data(data) {}
    Token(TokenType type, char data) : _type(type), _data(1, data) {}
    TokenType _type;
    std::string _data;
  };

protected:
  Token getToken();

private:
  std::string _mpt_expression;
  std::string::iterator _c;

  bool isDigit();
  bool isOperator();
  bool isOpenParenthesis();
  bool isCloseParenthesis();
  bool isAlphaFirst();
  bool isAlphaCont();

  int getInteger();
  void skipWhite();
};

#endif // MATHPARSETOKENIZER_H
