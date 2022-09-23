#ifndef GENERATOR_PROGRAM_HPP
#define GENERATOR_PROGRAM_HPP

#include <string>
#include <vector>
#include <map>
#include <map>

using std::map;
using std::pair;
using std::string;
using std::vector;

struct Program
{
	map<string, // name
		pair<
			string,		   // type
			vector<string> // params
			>>
		functions;

	map<
		string, // name
		string	// type
		>
		variables;

	map<
		string, // identifier
		string	// llvm_name
		>
		named_variables;

	map<
		string, // simple variable
		string	// source variable
		>
		variable_sources;

	Program() : functions(), variables(), named_variables()
	{
		// TODO: make println better, currently it only prints string, i want it to print everything
		addFunc("println", "i32", {"ptr"}); // returns the length of what was printed
		addFunc("exit", "void", {"i32"});
	}

	void addFunc(string n, string t, vector<string> p)
	{
		functions[n] = {t, p};
	}

	void addVar(string n, string t)
	{
		variables[n] = t;
	}

	string getFuncArgs(string n) const
	{
		string a = "(";
		for (auto p : functions.at(n).second)
		{
			a += p;
			a += ",";
		}
		a[a.size() - 1] = ')'; // replacing the last ',' with '('
		return a;
	}

	inline string getFuncRetType(string n) const
	{
		return functions.at(n).first;
	}
};

#endif // GENERATOR_PROGRAM_HPP