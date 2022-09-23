#include "lexer.hpp"

Token Lexer::next()
{
	while (c() != '\0' && skip_blank()) // skip_blank skips whitespace and comments then returns if we're not at the end
	{
		if (isalpha(c()) || c() == '_') // identifier or keyword or type or true | false
			return parse_alpha();
		if (isdigit(c()) || (c() == '.' && isdigit(c(1)))) /* if c is a digit or its a . but has a digit after it */
			return parse_digit();
		if (c() == '\'')
			return parse_char();
		if (c() == '"')
			return parse_string();
		else
			return parse_other();
	}

	return Token(TOKEN_EOF, this->i);
}

Token Lexer::parse_alpha()
{
	size_t at = this->i;
	string word = "";

	while (isalpha(c()) || isdigit(c()) || c() == '_')
	{
		word += c();
		inc();
	}

	if (this->rules.keywords.size() > 0)
	{
		for (auto &keyword : this->rules.keywords)
		{
			if (word == keyword)
				return Token(TOKEN_KEYWORD, at, word);
		}
	}

	if (this->rules.types.size() > 0)
	{
		for (auto &type : this->rules.types)
		{
			if (word == type)
				return Token(TOKEN_TYPE, at, word);
		}
	}

	return Token(TOKEN_IDENTIFIER, at, word);
}

Token Lexer::parse_digit()
{
	size_t at = this->i;
	string word = "";

	if (c() == '.')
	{
		word += '0';
	}
	while (isdigit(c()))
	{
		word += c();
		inc();
	}
	if (c() == '.')
	{
		word += c();
		inc();
		while (isdigit(c()))
		{
			word += c();
			inc();
		}
		return Token(TOKEN_FLOAT_LITERAL, at, std::stold(word));
	}
	else if ((c() == 'x' || c() == 'X') && word == "0")
	{
		word += c();
		inc();
		while (isxdigit(c()))
		{
			word += c();
			inc();
		}
		return Token(TOKEN_INTEGER_LITERAL, at, std::stoll(word, nullptr, 16));
	}
	else if ((c() == 'b' || c() == 'B') && word == "0")
	{
		word.clear();
		inc();
		while (c() == '0' || c() == '1')
		{
			word += c();
			inc();
		}
		return Token(TOKEN_INTEGER_LITERAL, at, std::stoll(word, nullptr, 2));
	}
	else if ((c() == 'o' || c() == 'O') && word == "0")
	{
		word.clear();
		inc();
		while (isdigit(c()) || c() == '0' || c() == '1' || c() == '2' || c() == '3' || c() == '4' || c() == '5' || c() == '6' || c() == '7')
		{
			word += c();
			inc();
		}
		return Token(TOKEN_INTEGER_LITERAL, at, std::stoll(word, nullptr, 8));
	}
	else
	{
		return Token(TOKEN_INTEGER_LITERAL, at, std::stoll(word, nullptr, 10));
	}
}

Token Lexer::parse_string()
{
	size_t at = this->i;
	string word = "";

	inc();
	while (c() != '\0' && (c() != '"' || (c() == '"' && c(-1) == '\\')))
	{
		word += c();
		inc();
	}
	if (c() == '"')
	{
		inc();
		word = this->account_for_special_characters(word);
		return Token(TOKEN_STRING_LITERAL, at, word);
	}
	else
	{
		return Token(TOKEN_ERROR, at, "Unterminated string literal");
	}
}

Token Lexer::parse_char()
{
	size_t at = this->i;
	string word = "";

	inc();
	while (c() != '\0' && (c() != '\'' || (c() == '\'' && c(-1) == '\\')))
	{
		word += c();
		inc();
	}
	if (c() == '\'')
	{
		inc();
		word = this->account_for_special_characters(word);
		if (word.size() == 1)
			return Token(TOKEN_CHAR_LITERAL, at, static_cast<unsigned int>(word[0]));
		else
			return Token(TOKEN_ERROR, at, "Character literal must be a single character");
	}
	else
	{
		return Token(TOKEN_ERROR, at, "Unterminated char literal");
	}
}

Token Lexer::parse_other()
{
	size_t at = this->i;
	string word = "";

	// max length of an operator or a punctuator is 3
	for (int j = 3; j >= 0; j--)
	{
		if (this->i + j >= this->src.size()) // check we are not overflowing the source
			continue;

		word = this->src.substr(this->i, j); // get the word in the length of j

		if (std::find(this->rules.operators.begin(), this->rules.operators.end(), word) != this->rules.operators.end()) // if the word is an operator
		{
			inc(j);
			return Token(TOKEN_OPERATOR, at, word); // return the operator
		}
		if (std::find(this->rules.punctuators.begin(), this->rules.punctuators.end(), word) != this->rules.punctuators.end()) // if the word is a punctuator
		{
			inc(j);
			return Token(TOKEN_PUNCTUATOR, at, word); // return the punctuator
		}
	}

	inc();
	return Token(TOKEN_ERROR, at, string("Unknown token: ") + src.substr(i - 1, 1)); // if none of the operators or punctuators match with all the lengths, return an error
}

bool Lexer::skip_whitespace()
{
	bool skipped = false;
	while (c() == ' ' || c() == '\t' || c() == '\n' || c() == '\r')
	{
		skipped = true;
		inc();
	}
	return skipped;
}

bool Lexer::skip_comment()
{
	bool skipped = false;
	if (c() == '/' && c(1) == '/')
	{
		inc(2);
		skipped = true;
		while (c() != '\n')
		{
			inc();
		}
	}
	else if (c() == '/' && c(1) == '*')
	{
		inc(2);
		skipped = true;
		while (!is_at_end())
		{
			if (c() == '*' && c(1) == '/')
			{
				inc(2);
				break;
			}
			inc();
		}
		if (is_at_end())
		{
			this->Error("Unterminated comment");
		}
	}
	return skipped;
}

bool Lexer::skip_blank()
{
	while (!is_at_end())
	{
		if (skip_whitespace() || skip_comment())
			continue;
		else
			break;
	}
	return !is_at_end();
}

string Lexer::account_for_special_characters(const string &og)
{
	// take a string and replace all the special characters with their escape sequences
	// "\\n" => "\n"
	// account for \n, \t, \r, \a, \b, \f, \v,
	// \[num] (ascii value to char), \x[hex num] (ascii value to char), \o[octal num] (ascii value to char),
	// \[other] (just replace with the escape sequence with the other)
	string ret = "";
	for (size_t i = 0; i < og.size(); i++)
	{
		if (og[i] == '\\')
		{
			if (i + 1 < og.size())
			{
				string temp = "";
				switch (og[i + 1])
				{
				case 'n':
					ret += '\n';
					break;
				case 't':
					ret += '\t';
					break;
				case 'r':
					ret += '\r';
					break;
				case 'a':
					ret += '\a';
					break;
				case 'b':
					ret += '\b';
					break;
				case 'f':
					ret += '\f';
					break;
				case 'v':
					ret += '\v';
					break;
				case 'x':
					i += 2;
					while (i < og.size() && isxdigit(og[i]))
					{
						temp += og[i];
						i++;
					}
					ret += (char)std::stoi(temp, nullptr, 16);
					break;
				case 'o':
					i += 2;
					while (i < og.size() && isdigit(og[i]))
					{
						temp += og[i];
						i++;
					}
					ret += (char)std::stoi(temp, nullptr, 8);
					break;
				default:
					// either a number or just a character
					if (isdigit(og[i + 1]))
					{
						while (i < og.size() && isdigit(og[i]))
						{
							temp += og[i];
							i++;
						}
						ret += (char)std::stoi(temp, nullptr, 10);
						break;
					}
					else
					{
						ret += og[i];
					}
				}
			}
		}
		else
		{
			ret += og[i];
		}
	}

	return ret;
}