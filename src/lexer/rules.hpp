#ifndef LEXER_RULES_HPP
#define LEXER_RULES_HPP

#include "token.hpp"
#include <string>
#include <vector>
using std::string;
using std::vector;

struct Rules
{
	const vector<string> keywords;
	const vector<string> types;
	// max length of an operator is 3
	const vector<string> operators;
	// max length of a punctuator is 3
	const vector<string> punctuators;

	Rules(const vector<string> _keywords, const vector<string> _types, const vector<string> _operators, const vector<string> _punctuators) : keywords(_keywords), types(_types), operators(_operators), punctuators(_punctuators) {}
};

#endif // LEXER_RULES_HPP