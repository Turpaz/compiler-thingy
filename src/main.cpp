#include <stdio.h>
#include <string>
#include <fstream>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "generator/generator.hpp"

using std::string;

void read_from_file(string path, string &src);

int main(int argc, char **argv)
{
	// setting the rules for the lexer

	Rules rules({"fn", "return", "var"},
				{"i32", "str", "void"},
				{
					"||",
					"&&",
					"!=",
					"==",
					">",
					">=",
					"<",
					"<=",
					"+",
					"-",
					"*",
					"/",
					"%",
					"^",
					"++",
					"--",
					"!",
				},
				{";", "(", ")", "{", "}", "[", "]", "\"", ",", ":", ".", "?", "=", "+=", "-=", "*=", "/=", "%=", "^="});

	if (argc < 2)
	{
		// maybe in the future open some kind of JIT thingy...
		printf("error: no input files\n");
		exit(1);
	}
	string path = argv[1];
	string output_ll = path.substr(0, path.find_last_of('.')) + ".ll"; // TODO: support -o
	string output = path.substr(0, path.find_last_of('.')) + ".exe";   // TODO: support -o
	string src = "";
	// read the file into src
	read_from_file(path, src);

	// create a lexer, parser and code generator
	Lexer lexer(rules, path, src);
	Parser parser(&lexer);
	Generator generator(path, &src);

	// -- printing the tokens
	/*
	Token token = lexer.next();
	while (token.type != TokenType::TOKEN_EOF)
	{
		printf("%s\n", token.to_string(src.c_str()).c_str());
		token = lexer.next();
	}
	*/

	// -- printing the statements
	/*
	Nodes::Stmt *stmt = parser.next();
	while (stmt != nullptr)
	{
		stmt->print();
		stmt = parser.next();
	}
	*/

	// -- printing the llvm assembly code
	/*
	Nodes::Stmt *stmt = parser.next();
	while (stmt != nullptr)
	{
		// stmt->print(); // emit the AST representation
		stmt->codegen(generator);
		stmt = parser.next();
	}
	generator.print();
	*/

	// -- generating the .ll code and compiling it to .exe
	///*
	Nodes::Stmt *stmt = parser.next();
	while (stmt != nullptr)
	{
		stmt->codegen(generator);
		stmt = parser.next();
	}
	// create file
	std::ofstream outFile(output_ll);
	outFile << generator.generate();
	outFile.close();
	// compile with clang
	system(("clang -w -Ofast " + output_ll + " -o " + output).c_str());
	//*/

	return 0;
}

void read_from_file(string path, string &src)
{
	std::ifstream file(path);
	if (file.is_open())
	{
		std::string line;
		while (getline(file, line))
		{
			src += line + "\n";
		}
		file.close();
	}
	else
	{
		printf("Unable to open file\n");
		exit(1);
	}
}