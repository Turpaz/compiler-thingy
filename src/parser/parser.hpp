#ifndef PARSER_PARSER_HPP
#define PARSER_PARSER_HPP

#include "../lexer/lexer.hpp"
#include "node.hpp"

class Parser
{
private:
	Token tok;

public:
	Lexer *lexer;

public:
	Parser(Lexer *Lexer)
	{
		lexer = Lexer;
		tok = lexer->next();
	}

	Nodes::Stmt *next();

private:
	Nodes::Stmt *parse_stmt();
	Nodes::Expr *parse_expr();

	Nodes::Stmt *parse_block_stmt();
	Nodes::Stmt *parse_expr_stmt();
	Nodes::Stmt *parse_var_stmt();
	Nodes::Stmt *parse_func_stmt();
	Nodes::Stmt *parse_return_stmt();

	Nodes::Expr *parse_e();
	Nodes::Expr *parse_t1();
	Nodes::Expr *parse_t2();
	Nodes::Expr *parse_t3();
	Nodes::Expr *parse_t4();
	Nodes::Expr *parse_t5();
	Nodes::Expr *parse_t6();
	Nodes::Expr *parse_t7();
	Nodes::Expr *parse_t8();
	Nodes::Expr *parse_t9();
	Nodes::Expr *parse_t10();
	Nodes::Expr *parse_f();

public:
	void Error(string msg, size_t position)
	{
		lexer->Error(msg, position);
	}
};

#endif // PARSER_PARSER_HPP