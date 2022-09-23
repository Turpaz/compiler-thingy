#ifndef LEXER_LEXER_HPP
#define LEXER_LEXER_HPP

#include "token.hpp"
#include "rules.hpp"
#include <string>
#include <vector>
#include <algorithm>
using std::string;

class Lexer
{
public:
	Rules rules;
	string src;
	string path;
	size_t size;
	size_t i;

public:
	Lexer(Rules &rules, const string &_path, const string &_src) : rules(rules), src(_src), path(_path), size(src.size()), i(0) {}

	Token next();

	void Error(const string &msg, size_t at)
	{
		fprintf(stderr, "[%s:%d:%d] Error: %s\n", path.c_str(), get_line(src.c_str(), at), get_column(src.c_str(), at), msg.c_str());
		exit(1);
	}
	void Error(const string &msg)
	{
		this->Error(msg, this->i);
	}

private:
	Token parse_alpha();
	Token parse_digit();
	Token parse_string();
	Token parse_char();
	Token parse_other();
	bool skip_whitespace(); // return true if skipped anything
	bool skip_comment();	// return true if skipped anything
	bool skip_blank();		// return !is_at_end()
	bool is_at_end() { return i >= size || src[i] == '\0'; }
	inline void inc()
	{
		if (!is_at_end())
			i++;
	}
	inline void inc(size_t n)
	{
		for (size_t i = 0; i < n; i++)
		{
			inc();
		}
	}

	string account_for_special_characters(const string &og);

	inline char c() const { return this->src[this->i]; }
	inline char c(size_t a) const { return this->src[this->i + a]; }
};

#endif // LEXER_LEXER_HPP