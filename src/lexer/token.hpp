#ifndef LEXER_TOKEN_HPP
#define LEXER_TOKEN_HPP

#include <string>
using std::string;

size_t get_line(const char *src, size_t position);
size_t get_column(const char *src, size_t position);

typedef enum TokenType
{
	TOKEN_EOF = 0,
	TOKEN_ERROR,

	TOKEN_IDENTIFIER,

	TOKEN_STRING_LITERAL,
	TOKEN_FLOAT_LITERAL,
	TOKEN_INTEGER_LITERAL,
	TOKEN_CHAR_LITERAL,
	TOKEN_BOOL_LITERAL,

	TOKEN_KEYWORD,
	TOKEN_TYPE,
	TOKEN_OPERATOR,
	TOKEN_PUNCTUATOR,
} TokenType;

inline string TokenTypeToString(TokenType type)
{
	switch (type)
	{
	case TOKEN_EOF:
		return "EOF";
	case TOKEN_ERROR:
		return "ERROR";
	case TOKEN_IDENTIFIER:
		return "IDENTIFIER";
	case TOKEN_STRING_LITERAL:
		return "STRING_LITERAL";
	case TOKEN_FLOAT_LITERAL:
		return "FLOAT_LITERAL";
	case TOKEN_INTEGER_LITERAL:
		return "INTEGER_LITERAL";
	case TOKEN_CHAR_LITERAL:
		return "CHAR_LITERAL";
	case TOKEN_BOOL_LITERAL:
		return "BOOL_LITERAL";
	case TOKEN_KEYWORD:
		return "KEYWORD";
	case TOKEN_TYPE:
		return "TYPE";
	case TOKEN_OPERATOR:
		return "OPERATOR";
	case TOKEN_PUNCTUATOR:
		return "PUNCTUATOR";
	default:
		return "UNKNOWN";
	}
}

struct Token
{
	unsigned type; // TokenType
	size_t position;
	// value
	string str; // string literal, identifier, keyword, type, operator, error message etc.
	union
	{
		long double floating;
		long long integer;
		unsigned int character;
		bool boolean;
	};

public:
	Token(TokenType type, size_t position, const string &str) : type(static_cast<unsigned>(type)), position(position), str(str) {}
	Token(TokenType type, size_t position, long double floating) : type(static_cast<unsigned>(type)), position(position), floating(floating) {}
	Token(TokenType type, size_t position, long long integer) : type(static_cast<unsigned>(type)), position(position), integer(integer) {}
	Token(TokenType type, size_t position, unsigned int character) : type(static_cast<unsigned>(type)), position(position), character(character) {}
	Token(TokenType type, size_t position, bool boolean) : type(static_cast<unsigned>(type)), position(position), boolean(boolean) {}
	Token(TokenType type, size_t position) : type(static_cast<unsigned>(type)), position(position) {}
	Token() {}

	string to_string(const char *src);

	inline bool type_is(TokenType type)
	{
		return this->type == static_cast<unsigned>(type);
	}
};

#endif // LEXER_TOKEN_HPP