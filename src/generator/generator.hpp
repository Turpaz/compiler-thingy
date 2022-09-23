#ifndef GENERATOR_GENERATOR_HPP
#define GENERATOR_GENERATOR_HPP

#include <string>
#include <vector>
#include <map>

#include "program.hpp"

using std::map;
using std::pair;
using std::string;
using std::vector;

size_t get_line(const char *src, size_t position);
size_t get_column(const char *src, size_t position);

struct Generator
{
	string path;
	string *src;
	Program prog;

	string literals;
	const char *constantFunctions =
		"declare i32 @printf(ptr noalias nocapture, ...)\n\
declare void @exit(i32)\n\
define i32 @print(ptr %str)\n\
{\n\
	%1 = call i32 (ptr, ...) @printf(ptr %str)\n\
	ret i32 %1\n\
}\n\
define i32 @println(ptr %str)\n\
{\n\
        %nlp = alloca ptr\n\
        store ptr @.str.nl, ptr %nlp\n\
        %nl = load ptr, ptr %nlp\n\
        %1 = call i32 (ptr, ...) @print(ptr %str)\n\
        %2 = call i32 (ptr, ...) @print(ptr %nl)\n\
        %3 = add i32 %1, %2\n\
        ret i32 %3\n\
}\n";
	const char *powerFunction =
		"define dso_local i32 @power(i32 %0, i32 %1) {\n\
  %3 = icmp eq i32 %1, 0\n\
  br i1 %3, label %18, label %4\n\
4: ; preds = %2\n\
  %5 = sdiv i32 %1, 2\n\
  %6 = tail call i32 @power(i32 %0, i32 %5)\n\
  %7 = and i32 %1, 1\n\
  %8 = icmp eq i32 %7, 0\n\
  br i1 %8, label %9, label %11\n\
9: ; preds = %4\n\
  %10 = mul nsw i32 %6, %6\n\
  br label %18\n\
11: ; preds = %4\n\
  %12 = icmp sgt i32 %1, 0\n\
  %13 = mul i32 %6, %6\n\
  br i1 %12, label %14, label %16\n\
14: ; preds = %11\n\
  %15 = mul i32 %13, %0\n\
  br label %18\n\
16: ; preds = %11\n\
  %17 = sdiv i32 %13, %0\n\
  br label %18\n\
18: ; preds = %9, %14, %16, %2\n\
  %19 = phi i32 [ 1, %2 ], [ %10, %9 ], [ %15, %14 ], [ %17, %16 ]\n\
  ret i32 %19\n\
}\n";

	string code;
	// ...

	std::map<string, size_t> literal_count;
	long long vc;  // variable counter
	long long evc; // expression variable counter
	long long bvc; // binary operation variable counter

	Generator(const string &path, string *src) : path(path), src(src), vc(1), evc(0), bvc(0)
	{
		literals = "@.str.nl = constant [2 x i8] c\"\\0A\\00\"\n";
		prog.variables["@.str.nl"] = "ptr";
	}

	string generate() const
	{
		string output = ((literals + constantFunctions) + powerFunction) + code;

		return output;
	}

	void print() const
	{
		printf("%s", generate().c_str());
	}

	void Error(const string &msg, size_t position)
	{
		fprintf(stderr, "[%s:%d:%d] Error: %s\n", path.c_str(), get_line(src->c_str(), position), get_column(src->c_str(), position), msg.c_str());
		exit(1);
	}

	// get the variable counter as a string and increment it
	inline string vci()
	{
		return std::to_string(vc++);
	}
	// get the previous variable counter
	inline string vcl() const { return std::to_string(vc - 1); }

	// Operations
	string addStrLit(string s)
	{
		size_t c = literal_count["str"];
		literal_count["str"] = c + 1;
		string len = std::to_string(s.size() + 1);
		string vname = "@.str." + std::to_string(c);

		literals += vname + " = constant [" + len + " x i8] c\"" + s + "\\00\"\n";
		prog.variables[vname] = "ptr"; // adding the variable the the program struct

		return vname;
	}

	string addIntLit(long long i)
	{
		// size_t c = literal_count["int"];
		// literal_count["int"] = c + 1;

		// literals += "@.int." + std::to_string(c) + " = constant i32 " + std::to_string(i) + "\n";
		// prog.variables["@.int." + std::to_string(c)] = "i32"; // adding the variable the the program struct

		// return "@.int." + std::to_string(c);

		size_t c = literal_count["%int"];
		literal_count["%int"] = c + 1;

		code += "%int" + std::to_string(c) + " = add i32 0, " + std::to_string(i) + "\n";
		prog.variables["%int" + std::to_string(c)] = "i32"; // adding the variable the the program struct

		return "%int" + std::to_string(c);
	}

	string addBoolLit(bool i)
	{
		size_t c = literal_count["%bool"];
		literal_count["%bool"] = c + 1;

		code += "%bool" + std::to_string(c) + " = add i1 0, " + std::to_string(i) + "\n";
		prog.variables["%bool" + std::to_string(c)] = "i1"; // adding the variable the the program struct

		return "%bool" + std::to_string(c);
	}

	string addFloatLit(long double i)
	{
		size_t c = literal_count["%float"];
		literal_count["%float"] = c + 1;

		code += "%float" + std::to_string(c) + " = fadd double 0, " + std::to_string(i) + "\n";
		prog.variables["%float" + std::to_string(c)] = "double"; // adding the variable the the program struct

		return "%float" + std::to_string(c);
	}

	string addCharLit(unsigned int i)
	{
		size_t c = literal_count["%char"];
		literal_count["%char"] = c + 1;

		code += "%char" + std::to_string(c) + " = add i32 0, " + std::to_string(i) + "\n";
		prog.variables["%char" + std::to_string(c)] = "i32"; // adding the variable the the program struct

		return "%char" + std::to_string(c);
	}

	string toFreeVariable(string s)
	{
		if (s[0] == '@')
		{
			string type = prog.variables.at(s);
			code += "%e" + std::to_string(evc) + "p = alloca " + type + "\n";
			code += "store ptr " + s + ", ptr %e" + std::to_string(evc) + "p\n";
			code += "%e" + std::to_string(evc) + " = load " + type + ", ptr %e" + std::to_string(evc) + "p\n";
			prog.variables["%e" + std::to_string(evc)] = type; // adding the variable the the program struct
			return "%e" + std::to_string(evc++);			   // it'll increment evc only after it uses the value because it's postfix (v++, not ++v)
		}
		else if (s[0] == '%')
		{
			return s;
		}

		return s;
	}
};

#endif // GENERATOR_GENERATOR_HPP