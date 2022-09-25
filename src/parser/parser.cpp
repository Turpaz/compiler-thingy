#include "parser.hpp"

Nodes::Stmt *Parser::next()
{
	return parse_stmt();
}

Nodes::Stmt *Parser::parse_stmt()
{
	switch (tok.type)
	{
	case TokenType::TOKEN_EOF:
		return nullptr;
	case TokenType::TOKEN_ERROR:
		printf("%s\n", tok.str.c_str());
		return nullptr;
	case TokenType::TOKEN_PUNCTUATOR:
		if (tok.str == ";")
		{
			return new Nodes::EmptyStmt(tok.position);
		}
		else if (tok.str == "{")
		{
			return parse_block_stmt();
		}
		else
		{
			Error("Unknown punctuator \"" + tok.str + "\"", tok.position);
		}
	case TokenType::TOKEN_KEYWORD:
		if (tok.str == "fn")
		{
			return parse_func_stmt();
		}
		else if (tok.str == "return")
		{
			return parse_return_stmt();
		}
		else if (tok.str == "var")
		{
			return parse_var_stmt();
		}
		else if (tok.str == "if")
		{
			return parse_if_stmt();
		}
		else
		{
			Error("Unknown keyword \"" + tok.str + "\"", tok.position);
		}
	default:
		return parse_expr_stmt();
	}
}

/*
// FIXME: this is temporary and doesn't support much options
Nodes::Expr *Parser::parse_expr()
{
	// do some recursive shit with parse_e() and parse_t() and so on and return the expression

	// TEMPORARY:

	size_t pos = tok.position;
	if (tok.type == TokenType::TOKEN_IDENTIFIER)
	{
		string name = tok.str;

		// skip the identifier
		tok = lexer->next();

		// check if it is a function call
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "(")
		{
			// skip the opening parenthesis
			tok = lexer->next();

			// parse the arguments
			vector<Nodes::Expr *> args;
			while (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ")")
			{
				args.push_back(parse_expr());
				if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ",")
				{
					tok = lexer->next();
				}
				else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ")")
				{
					break;
				}
				else
				{
					Error("Expected ',' or ')' after argument", tok.position);
				}
			}
			// skip the closing parenthesis
			tok = lexer->next();

			if (name == "println")
			{
				return new Nodes::PrintlnCallExpr(pos, args);
			}
			else if (name == "exit")
			{
				return new Nodes::ExitCallExpr(pos, args);
			}
			else
			{
				return new Nodes::FuncCallExpr(pos, name, args);
			}
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "=") // assignments
		{
			tok = lexer->next(); // skip the '='

			auto value = parse_expr();

			return new Nodes::AssignExpr(pos, name, value);
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "?") // ternary // FIXME: can only be parsed if the condition is an identifier
		{
			tok = lexer->next(); // skip the '?'

			auto bt = parse_expr();

			if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ":")
			{
				Error("Expected ':' for ternary expression", tok.position);
			}

			tok = lexer->next(); // skip the ':'

			auto bf = parse_expr();

			return new Nodes::TernaryExpr(pos, new Nodes::IdentifierExpr(pos, name), bt, bf);
		}
		// TODO: add array access or member access or something
		else
		{
			return new Nodes::IdentifierExpr(pos, name);
		}
	}
	else if (tok.type == TokenType::TOKEN_STRING_LITERAL)
	{
		string str = tok.str;
		tok = lexer->next();
		return new Nodes::StrLitExpr(pos, str);
	}
	else if (tok.type == TokenType::TOKEN_INTEGER_LITERAL)
	{
		long long i = tok.integer;
		tok = lexer->next();
		return new Nodes::IntLitExpr(pos, i);
	}
	else
	{
		Error("Unexpected token:" + tok.to_string(lexer->src.c_str()), pos);
		exit(1); // will never execute just here to make the warnings disappear
	}
}
*/

Nodes::Expr *Parser::parse_expr()
{
	return this->parse_e();
}

Nodes::Stmt *Parser::parse_block_stmt()
{
	Nodes::BlockStmt *block = new Nodes::BlockStmt(tok.position);

	// Make sure we have a '{'
	if (tok.str == "{")
	{
		// Skip the '{'
		tok = lexer->next();
		// Parse until we reach the end of the block
		while (!(tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "}"))
		{
			// if we reached the end of the file, throw an error
			if (tok.type == TokenType::TOKEN_EOF)
			{
				Error("Expected '}' to end the block", tok.position);
			}

			// push the statement onto the block, will increment the token too
			block->stmts.push_back(parse_stmt());
		}

		// skip the '}'
		tok = lexer->next();
	}
	else
	{
		// error(tok, "Expected '{ ... }'");
		exit(-1);
		Error("Expected Block { ... }", tok.position);
	}

	return block;
}

Nodes::Stmt *Parser::parse_expr_stmt()
{
	Nodes::Stmt *expr_stmt = new Nodes::ExprStmt(tok.position, parse_expr());

	// Make sure we have a ';'
	if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ";")
	{
		// skip the semicolon
		tok = lexer->next();
	}
	// Throw an error if we don't have a semicolon after the expression
	else
	{
		Error("Expected ';' after expression", tok.position);
	}

	return expr_stmt;
}

Nodes::Stmt *Parser::parse_var_stmt()
{
	size_t pos = tok.position;

	if (tok.str != "var")
	{
		Error("Expected \"var\" keyword (var <name>: <type> = <value>;)", tok.position);
	}

	tok = lexer->next(); // skip the var

	if (tok.type != TokenType::TOKEN_IDENTIFIER)
	{
		Error("Expected identifier for variable name (var <name>: <type> = <value>;)", tok.position);
	}

	string name = tok.str; // var name

	tok = lexer->next(); // skip the name

	if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ":") // TODO: change to support type inference
	{
		Error("Expected \':\' after variable name (var <name>: <type> = <value>;)", tok.position);
	}

	tok = lexer->next(); // skip the ':'

	if (tok.type != TokenType::TOKEN_TYPE)
	{
		Error("Expected type after colon (var <name>: <type> = <value>;)", tok.position);
	}

	string type = tok.str;

	tok = lexer->next();

	if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != "=") // TODO: add a way to not initialize?
	{
		Error("Expected expression for variable's value (var <name>: <type> = <value>;)", tok.position);
	}

	tok = lexer->next();

	auto e = parse_expr();

	if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ";")
	{
		Error("Expected \';\'", tok.position);
	}

	tok = lexer->next();

	return new Nodes::VarDeclStmt(pos, name, type, e);
}

Nodes::Stmt *Parser::parse_if_stmt()
{
	size_t pos = tok.position;

	if (tok.str != "if")
	{
		Error("Expected \"if\" keyword (if <cond> <code> else <code>)", tok.position);
	}

	tok = lexer->next(); // if

	Nodes::Expr *_cond = parse_expr(); // condition

	Nodes::Stmt *_then = parse_stmt(); // then

	if (tok.type != TokenType::TOKEN_KEYWORD || tok.str != "else") // no else
	{
		return new Nodes::IteStmt(pos, _cond, _then, new Nodes::EmptyStmt(pos));
	}

	// there's an "else"

	tok = lexer->next(); // else

	Nodes::Stmt *_else = parse_stmt(); // else

	return new Nodes::IteStmt(pos, _cond, _then, _else);
}

Nodes::Stmt *Parser::parse_func_stmt()
{
	Nodes::FuncDeclStmt *func_stmt = new Nodes::FuncDeclStmt(tok.position);

	// Make sure we have a 'fn'
	if (tok.str == "fn")
	{
		// skip the 'fn'
		tok = lexer->next();
	}
	else
	{
		Error("Expected a function declaration (fn <name>( <params> ) : <rType> { ... })", tok.position);
	}

	// TODO: Maybe add support to lambda like functions, support syntax like this - fun foo(a, b) = (a + b);

	// fun <name> ( <args> ) : <rType> { <body> }
	if (tok.type == TokenType::TOKEN_IDENTIFIER)
	{
		// Get the function name
		func_stmt->name = tok.str;
		tok = lexer->next();
	}
	else
	{
		Error("Expected function name (fn <name>( <params> ) : <rType> { ... })", tok.position);
	}
	// Get the arguments
	if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "(")
	{
		// skip the '('
		tok = lexer->next();
	}
	else
	{
		Error("Expected '(' after function name (fn <name>( <params> ) : <rType> { ... })", tok.position);
	}

	while (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ")")
	{
		pair<string, string> param;

		// param name
		if (tok.type == TokenType::TOKEN_IDENTIFIER)
		{
			param.first = tok.str;
			tok = lexer->next();
		}
		else
		{
			Error("Expected parameter name (fn <name>( <parameter_name : parameter_type> ) : <rType> { ... })", tok.position);
		}

		// make sure we have a ':'
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ":")
		{
			// skip the ':'
			tok = lexer->next();
		}
		else
		{
			Error("Expected ':' after parameter name (fn <name>( <parameter_name : parameter_type> ) : <rType> { ... })", tok.position);
		}

		// get param type
		if (tok.type == TokenType::TOKEN_TYPE)
		{
			param.second = tok.str;
			tok = lexer->next();
		}
		else
		{
			Error("Expected type after ':' (fn <name>( <parameter_name : parameter_type> ) : <rType> { ... })", tok.position);
		}

		// Add the parameter to the list
		func_stmt->params.push_back(param);

		// Check if we reached the end of the parameters
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ")")
			break;
		// Check if we reached the end of this parameter
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ",")
		{
			// skip the ','
			tok = lexer->next();
		}
		else
		{
			Error("Expected ',' or ')' after parameter", tok.position);
		}
	}

	// skip the ')'
	tok = lexer->next();

	func_stmt->rType = "void";

	// Get the return type if exists, otherwise assume void
	if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ":")
	{
		tok = lexer->next();
		if (tok.type == TokenType::TOKEN_TYPE)
		{
			func_stmt->rType = tok.str;
			tok = lexer->next();
		}
		else
		{
			Error("Expected return type after colon (fn <name>( <params> ) : <rType> { ... })", tok.position);
		}
	}

	// Get the function body
	func_stmt->body = (Nodes::BlockStmt *)parse_block_stmt();

	return func_stmt;
}

Nodes::Stmt *Parser::parse_return_stmt()
{
	Nodes::RetStmt *ret_stmt = new Nodes::RetStmt(tok.position);

	if (tok.str != "return")
	{
		Error("Expected \"return\" keyword (return <value>;)", tok.position);
	}

	tok = lexer->next(); // skip the return keyword

	ret_stmt->value = parse_expr();

	if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ";")
	{
		Error("Expected ';'", tok.position);
	}

	tok = lexer->next();
	return ret_stmt;
}

// Expressions

// Assignments =, +=, ...
Nodes::Expr *Parser::parse_e()
{
	// E -> T1 {= T1}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t1();

	// =, +=, -=, *=, /=, %=, ^=
	while (true)
	{
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "=") // =
		{
			tok = lexer->next(); // =
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, b);
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "+=") // +=
		{
			tok = lexer->next(); // +=
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_ADD, a, b));
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "-=") // -=
		{
			tok = lexer->next(); // -=
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_SUB, a, b));
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "*=") // *=
		{
			tok = lexer->next(); // *=
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_MUL, a, b));
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "/=") // /=
		{
			tok = lexer->next(); // /=
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_DIV, a, b));
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "%=") // %=
		{
			tok = lexer->next(); // %=
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_MOD, a, b));
		}
		else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "^=") // ^=
		{
			tok = lexer->next(); // ^=
			Nodes::Expr *b = parse_t1();

			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_POW, a, b));
		}
		else
		{
			return a;
		}
	}
}
// Ternary operator ? :
Nodes::Expr *Parser::parse_t1()
{
	// T1 -> T2 {? T2} {: T2}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t2();

	// a ? b : c
	while (true)
	{
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "?") // ?
		{
			tok = lexer->next(); // ?
			Nodes::Expr *b = parse_t2();

			if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ":") // :
			{
				tok = lexer->next(); // :
				Nodes::Expr *c = parse_t2();
				a = new Nodes::TernaryExpr(pos, a, b, c);
			}
			else
			{
				Error("Expected ':' in ternary expression (<cond> ? <then> : <else>)", pos);
			}
		}
		else
		{
			return a;
		}
	}
}
// Or operator ||
Nodes::Expr *Parser::parse_t2()
{
	// T2 -> T3 {|| T3}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t3();

	// a || b
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "||") // ||
		{
			tok = lexer->next(); // ||
			Nodes::Expr *b = parse_t3();
			a = new Nodes::BopExpr(pos, Nodes::BOP_OR, a, b);
		}
		else
		{
			return a;
		}
	}
}
// And operator &&
Nodes::Expr *Parser::parse_t3()
{
	// T2 -> T3 {&& T3}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t4();

	// a && b
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "&&") // &&
		{
			tok = lexer->next(); // &&
			Nodes::Expr *b = parse_t4();
			a = new Nodes::BopExpr(pos, Nodes::BOP_AND, a, b);
		}
		else
		{
			return a;
		}
	}
}
// Neq and Eq operators != and ==
Nodes::Expr *Parser::parse_t4()
{
	// T4 -> T5 {!=/== T5}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t5();

	// a == b, ...
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "==") // ==
		{
			tok = lexer->next(); // ==
			Nodes::Expr *b = parse_t5();
			a = new Nodes::BopExpr(pos, Nodes::BOP_EQ, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "!=") // !=
		{
			tok = lexer->next(); // !=
			Nodes::Expr *b = parse_t5();
			a = new Nodes::BopExpr(pos, Nodes::BOP_NEQ, a, b);
		}
		else
		{
			return a;
		}
	}
}
// Gt, Gte, Lt, Lte operators >, >=, <, <=
Nodes::Expr *Parser::parse_t5()
{
	// T5 -> T6 {</>/<=/>= T6}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t6();

	// a > b, ...
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == ">") // >
		{
			tok = lexer->next(); // >
			Nodes::Expr *b = parse_t6();
			a = new Nodes::BopExpr(pos, Nodes::BOP_GT, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == ">=") // >=
		{
			tok = lexer->next(); // >=
			Nodes::Expr *b = parse_t6();
			a = new Nodes::BopExpr(pos, Nodes::BOP_GTE, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "<") // <
		{
			tok = lexer->next(); // <
			Nodes::Expr *b = parse_t6();
			a = new Nodes::BopExpr(pos, Nodes::BOP_LT, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "<=") // <=
		{
			tok = lexer->next(); // <=
			Nodes::Expr *b = parse_t6();
			a = new Nodes::BopExpr(pos, Nodes::BOP_LTE, a, b);
		}
		else
		{
			return a;
		}
	}
}
// Add and Sub operators + and -
Nodes::Expr *Parser::parse_t6()
{
	// T6 -> T7 {+/- T7}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t7();

	// a + b, ...
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "+") // +
		{
			tok = lexer->next(); // +
			Nodes::Expr *b = parse_t7();
			a = new Nodes::BopExpr(pos, Nodes::BOP_ADD, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "-") // -
		{
			tok = lexer->next(); // -
			Nodes::Expr *b = parse_t7();
			a = new Nodes::BopExpr(pos, Nodes::BOP_SUB, a, b);
		}
		else
		{
			return a;
		}
	}
}
// Mul, Div and Mod operators *, / and %
Nodes::Expr *Parser::parse_t7()
{
	// T7 -> T8 {*/% T8}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t8();

	// a * b, ...
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "*") // *
		{
			tok = lexer->next(); // *
			Nodes::Expr *b = parse_t8();
			a = new Nodes::BopExpr(pos, Nodes::BOP_MUL, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "/") // /
		{
			tok = lexer->next(); // /
			Nodes::Expr *b = parse_t8();
			a = new Nodes::BopExpr(pos, Nodes::BOP_DIV, a, b);
		}
		else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "%") // %
		{
			tok = lexer->next(); // %
			Nodes::Expr *b = parse_t8();
			a = new Nodes::BopExpr(pos, Nodes::BOP_MOD, a, b);
		}
		else
		{
			return a;
		}
	}
}
// Pow ^
Nodes::Expr *Parser::parse_t8()
{
	// T7 -> T8 {^ T8}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t9();

	// a ^ b
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "^") // ^
		{
			tok = lexer->next(); // ^
			Nodes::Expr *b = parse_t9();
			a = new Nodes::BopExpr(pos, Nodes::BOP_POW, a, b);
		}
		else
		{
			return a;
		}
	}
}
// Inc, Dec operators ++, --
Nodes::Expr *Parser::parse_t9()
{
	// T9 -> T10 {++/-- T10}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_t10();

	// a++, a--
	while (true)
	{
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "++") // ++
		{
			tok = lexer->next(); // ++
			Nodes::Expr *b = parse_t10();
			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_ADD, a, new Nodes::IntLitExpr(pos, 1)));
		}
		if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "--") // --
		{
			tok = lexer->next(); // --
			Nodes::Expr *b = parse_t10();
			a = new Nodes::AssignExpr(pos, a, new Nodes::BopExpr(pos, Nodes::BOP_SUB, a, new Nodes::IntLitExpr(pos, 1)));
		}
		else
		{
			return a;
		}
	}
}
// array access, struct member access, slicing ., []
Nodes::Expr *Parser::parse_t10() // TODO: implement! i need to create the matching nodes and validate the logic
{
	// T10 -> F {. F} | F{[F]}
	size_t pos = tok.position;

	Nodes::Expr *a = parse_f();

	// a.b, a[b], a[b:c], a[b:c:d]
	while (true)
	{
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ".") // .
		{
			tok = lexer->next(); // .
			Nodes::Expr *b = parse_f();
			// a = new Nodes::MemberAccExpr(pos, a, b);
			Error("Not implemented yet!", pos); // to be removed
		}
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "[") // [
		{
			tok = lexer->next(); // [
			Nodes::Expr *b = parse_f();
			if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ":") // no :
			{
				// normal array access
				// a = new Nodes::ArrAccExpr(pos, a, b);
				Error("Not implemented yet!", pos); // to be removed
			}
			else // there is a colon!
			{
				Nodes::Expr *c = parse_f();
				if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ":") // no :
				{
					// simple sliced access (beginning:end)
					// a = new Nodes::SlicedArrAccExpr(pos, a, b, c, 1);
					Error("Not implemented yet!", pos); // to be removed
				}
				else // there's another colon!
				{
					Nodes::Expr *d = parse_f();

					// complex sliced access (beginning:end:step)
					// a = new Nodes::SlicedArrAccExpr(pos, a, b, c, d);
					Error("Not implemented yet!", pos); // to be removed
				}
			}

			if (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != "]") // no ]
			{
				Error("Expected a closing bracket ']' in treatable access ( <arr>[<index or start:end:step>] )", tok.position);
			}
		}
		else
		{
			return a;
		}
	}
}
// identifier, literal, parenthesis, neg, not, function call
Nodes::Expr *Parser::parse_f() // TODO: array/dict literal
{
	// F -> ID(E) | ID | (E) | -E | +E | !E | literal
	size_t pos = tok.position;

	if (tok.type == TokenType::TOKEN_IDENTIFIER) // ID(E) | ID
	{
		string name = tok.str;
		tok = lexer->next(); // id
		// function call
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "(")
		{
			tok = lexer->next(); // (

			// parse the arguments
			vector<Nodes::Expr *> args;
			while (tok.type != TokenType::TOKEN_PUNCTUATOR || tok.str != ")")
			{
				args.push_back(parse_expr());

				if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ",")
				{
					tok = lexer->next();
				}
				else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ")")
				{
					break;
				}
				else
				{
					Error("Expected ',' or ')' after argument in function call arguments list", tok.position);
				}
			}
			// skip the closing parenthesis
			tok = lexer->next();

			if (name == "println")
			{
				return new Nodes::PrintlnCallExpr(pos, args);
			}
			else if (name == "exit")
			{
				return new Nodes::ExitCallExpr(pos, args);
			}
			else
			{
				return new Nodes::FuncCallExpr(pos, name, args);
			}
		}
		// variable
		else
		{
			return new Nodes::IdentifierExpr(pos, name);
		}
	}
	else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "(") // (E)
	{
		tok = lexer->next(); // (
		Nodes::Expr *a = parse_expr();
		if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == ")") // )
		{
			tok = lexer->next();
			return a;
		}
		else
		{
			Error("Expected closing parenthesis ( (<expression>) )", tok.position);
		}
	}
	else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "-") // -E
	{
		tok = lexer->next();		  // -
		Nodes::Expr *a = parse_t10(); // the value can't be a bop expression ( -(a + b) != -a + b )
		return new Nodes::NegExpr(pos, a);
	}
	else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "+") // +E
	{
		tok = lexer->next(); // +
		return new Nodes::NegExpr(pos, parse_t10());
	}
	else if (tok.type == TokenType::TOKEN_OPERATOR && tok.str == "!") // !E
	{
		tok = lexer->next();		  // !
		Nodes::Expr *a = parse_t10(); // the value can't be a bop expression ( -(a + b) != -a + b )
		return new Nodes::NotExpr(pos, a);
	}
	else if (tok.type == TokenType::TOKEN_INTEGER_LITERAL) // int literal
	{
		auto temp = tok.integer;
		tok = lexer->next();
		return new Nodes::IntLitExpr(pos, temp);
	}
	else if (tok.type == TokenType::TOKEN_BOOL_LITERAL) // bool literal
	{
		auto temp = tok.boolean;
		tok = lexer->next();
		return new Nodes::BoolLitExpr(pos, temp);
	}
	else if (tok.type == TokenType::TOKEN_FLOAT_LITERAL) // float literal
	{
		auto temp = tok.floating;
		tok = lexer->next();
		return new Nodes::FloatLitExpr(pos, temp);
	}
	else if (tok.type == TokenType::TOKEN_STRING_LITERAL) // string literal
	{
		auto temp = tok.str;
		tok = lexer->next();
		return new Nodes::StrLitExpr(pos, temp);
	}
	else if (tok.type == TokenType::TOKEN_CHAR_LITERAL) // character literal
	{
		auto temp = tok.character;
		tok = lexer->next();
		return new Nodes::CharLitExpr(pos, temp);
	}
	else if (tok.type == TokenType::TOKEN_PUNCTUATOR && tok.str == "[") // array/dict literal // TODO: implement
	{
		// [1, 2, 3, 4] array
		// [0:5] array -> [0, 1, 2, 3, 4], [3:9:2] array -> [3, 5, 7]
		// [2.5, 'c', "hi"] list? tuple?
		// dict? ["a": b, "b": c]. key has to be string? '[' or '{'? distinct from range?
	}
	else
	{
		Error("Unknown expression (should be unreachable but i doubt it)", pos); // shouldn't happen
	}

	return nullptr; // Unreachable (i hope)
}