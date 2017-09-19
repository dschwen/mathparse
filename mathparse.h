#include <string>

class MathParse
{
public:
  MathParse(const std::string expression);

protected:
  std::string _expression;
  std::string::iterator _c;

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

  bool isDigit();
  bool isOperator();
  bool isOpenParenthesis();
  bool isCloseParenthesis();
  bool isAlphaFirst();
  bool isAlphaCont();

  int getInteger();
  void skipWhite();

  struct Token
  {
    Token() : _type(INVALID), _data() {}
    Token(TokenType type, const std::string& data) : _type(type), _data(data) {}
    Token(TokenType type, char data) : _type(type), _data(1, data) {}
    TokenType _type;
    std::string _data;
  };

  Token getToken();

  bool inSet(const std::string& set);
};
