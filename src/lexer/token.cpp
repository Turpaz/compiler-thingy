#include "token.hpp"

size_t get_line(const char *src, size_t position)
{
	size_t line = 1;
	for (size_t i = 0; i < position; i++)
	{
		if (src[i] == '\n')
		{
			line++;
		}
	}
	return line;
}

size_t get_column(const char *src, size_t position)
{
	size_t column = 0;
	for (size_t i = 0; i < position; i++)
	{
		if (src[i] == '\n')
		{
			column = 0;
		}
		else
		{
			column++;
		}
	}
	return column;
}

string Token::to_string(const char *src)
{
	string out;
	out += std::to_string(get_line(src, position));
	out += ":";
	out += std::to_string(get_column(src, position));
	out += ": Token(";
	out += TokenTypeToString(static_cast<TokenType>(type));
	out += ", ";

	switch (type)
	{
	case TOKEN_STRING_LITERAL:
		out += "\"" + str + "\"";
		break;
	case TOKEN_FLOAT_LITERAL:
		out += std::to_string(floating);
		break;
	case TOKEN_INTEGER_LITERAL:
		out += std::to_string(integer);
		break;
	case TOKEN_CHAR_LITERAL:
		out += '\'';
		out += static_cast<char>(character);
		out += '\'';
		break;
	case TOKEN_BOOL_LITERAL:
		out += boolean ? "true" : "false";
		break;
	default:
		out += str;
		break;
	}

	out += ")";
	return out;
}