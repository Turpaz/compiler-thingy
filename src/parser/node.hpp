#ifndef PARSER_NODE_HPP
#define PARSER_NODE_HPP

#include <stdlib.h>
#include <string>
#include <vector>
#include <map>

#include "../generator/generator.hpp"

using std::pair;
using std::string;
using std::vector;

namespace Nodes
{
	enum BOPS
	{
		BOP_OR = 0,
		BOP_AND,
		BOP_NEQ,
		BOP_EQ,
		BOP_GT,
		BOP_GTE,
		BOP_LT,
		BOP_LTE,
		BOP_ADD,
		BOP_SUB,
		BOP_MUL,
		BOP_DIV,
		BOP_MOD,
		BOP_POW
	};

	struct Stmt
	{
		size_t position;

		Stmt(size_t Position) : position(Position) {}

		virtual void print() const = 0;
		virtual void codegen(Generator &gen) const = 0;
	};
	struct Expr
	{
		size_t position;

		Expr(size_t Position) : position(Position) {}

		virtual void print() const = 0;
		virtual string codegen(Generator &gen) const = 0;
	};

	struct EmptyStmt : public Stmt
	{
		EmptyStmt(size_t Position) : Stmt(Position) {}

		void print() const
		{
			printf("(EmptyStmt at %zu)\n", position);
		}

		void codegen(Generator &gen) const {}
	};
	struct BlockStmt : public Stmt
	{
		vector<Stmt *> stmts;

		BlockStmt(size_t position, vector<Stmt *> stmts) : Stmt(position), stmts(stmts) {}
		BlockStmt(size_t position) : Stmt(position), stmts() {}
		~BlockStmt()
		{
			for (Stmt *stmt : stmts)
			{
				delete stmt;
			}
		}

		void print() const
		{
			printf("(BlockStmt at %zu)\n", position);
			this->printBlock();
		}

		void printBlock() const
		{
			printf("{\n");
			for (Stmt *stmt : stmts)
			{
				printf("\t");
				stmt->print();
			}
			printf("}\n");
		}

		void codegen(Generator &gen) const;
	};
	struct VarDeclStmt : public Stmt
	{
		string name;
		string type;
		Expr *value;

		VarDeclStmt(size_t position, string name, string type, Expr *value) : Stmt(position), name(name), type(type), value(value) {}
		~VarDeclStmt()
		{
			delete value;
		}

		void print() const
		{
			printf("(VarDeclStmt at %zu)\n", position);
			printf("\tname: %s\n", name.c_str());
			printf("\ttype: %s\n", type.c_str());
			printf("\tbody: ");
			value->print();
		}

		void codegen(Generator &gen) const;
	};
	struct IteStmt : public Stmt
	{
		Expr *cond;
		Stmt *then_b;
		Stmt *else_b;

		IteStmt(size_t position, Expr *cond, Stmt *then_b, Stmt *else_b) : Stmt(position), cond(cond), then_b(then_b), else_b(else_b) {}
		~IteStmt()
		{
			delete cond;
			delete then_b;
			delete else_b;
		}

		void print() const
		{
			printf("(IteStmt at %zu)\n", position);
			printf("\tif: ");
			cond->print();
			printf("\n\tthen: ");
			then_b->print();
			printf("\n\telse: ");
			else_b->print();
			printf("\n");
		}

		void codegen(Generator &gen) const;
	};
	struct WhileStmt : public Stmt
	{
		Expr *cond;
		Stmt *do_b;

		WhileStmt(size_t position, Expr *cond, Stmt *do_b) : Stmt(position), cond(cond), do_b(do_b) {}
		~WhileStmt()
		{
			delete cond;
			delete do_b;
		}

		void print() const
		{
			printf("(WhileStmt at %zu)\n", position);
			printf("\twhile: ");
			cond->print();
			printf("\n\tdo: ");
			do_b->print();
			printf("\n");
		}

		void codegen(Generator &gen) const;
	};
	struct FuncDeclStmt : public Stmt
	{
		string name;
		// vector<pair<pair<vartypes, string>, Expression*>> args;
		vector<pair<string, string>> params; // name, type
		string rType;						 // if not specified, it is set to void
		BlockStmt *body;

		FuncDeclStmt(size_t position, string name, vector<pair<string, string>> params, string rType, BlockStmt *body) : Stmt(position), name(name), params(params), rType(rType), body(body) {}
		FuncDeclStmt(size_t position) : Stmt(position), name(""), params(), rType("void"), body(nullptr) {}
		~FuncDeclStmt()
		{
			delete body;
		}

		void print() const
		{
			printf("(FuncDeclStmt at %zu)\n", position);
			printf("\tname: %s\n", name.c_str());
			printf("\targs: ");
			for (pair<string, string> arg : params)
			{
				printf("%s %s, ", arg.first.c_str(), arg.second.c_str());
			}
			printf("\n\trType: %s\n", rType.c_str());
			printf("\tbody: ");
			body->print();
		}

		void codegen(Generator &gen) const;
	};
	struct RetStmt : public Stmt
	{
		Expr *value;

		RetStmt(size_t position, Expr *value) : Stmt(position), value(value) {}
		RetStmt(size_t position) : Stmt(position) {}

		void print() const
		{
			printf("(RetStmt at %zu)\n", position);
			printf("\tvalue: ");
			value->print();
		}

		void codegen(Generator &gen) const;
	};
	struct ExprStmt : public Stmt
	{
		Expr *expr;

		ExprStmt(size_t position, Expr *expr) : Stmt(position), expr(expr) {}
		~ExprStmt()
		{
			delete expr;
		}

		void print() const
		{
			printf("(ExprStmt at %zu)\n", position);
			expr->print();
		}

		void codegen(Generator &gen) const;
	};
	struct FuncCallExpr : public Expr
	{
		string name;
		vector<Expr *> args;

		FuncCallExpr(size_t position, string name, vector<Expr *> args) : Expr(position), name(name), args(args) {}
		~FuncCallExpr()
		{
			for (Expr *arg : args)
			{
				delete arg;
			}
		}

		void print() const
		{
			printf("(FuncCallExpr at %zu)\n", position);
			printf("\tname: %s\n", name.c_str());
			printf("\targs: ");
			for (Expr *arg : args)
			{
				arg->print();
				printf(", ");
			}
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct PrintlnCallExpr : public Expr
	{
		vector<Expr *> args;

		PrintlnCallExpr(size_t position, vector<Expr *> args) : Expr(position), args(args) {}
		~PrintlnCallExpr()
		{
			for (Expr *arg : args)
			{
				delete arg;
			}
		}

		void print() const
		{
			printf("(PrintlnCallExpr at %zu)\n", position);
			printf("\targs: ");
			for (Expr *arg : args)
			{
				arg->print();
				printf(", ");
			}
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct ExitCallExpr : public Expr
	{
		vector<Expr *> args;

		ExitCallExpr(size_t position, vector<Expr *> args) : Expr(position), args(args) {}
		~ExitCallExpr()
		{
			for (Expr *arg : args)
			{
				delete arg;
			}
		}

		void print() const
		{
			printf("(ExitCallExpr at %zu)\n", position);
			printf("\targs: ");
			for (Expr *arg : args)
			{
				arg->print();
				printf(", ");
			}
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct AssignExpr : public Expr
	{
		Expr *dest;
		Expr *value;

		AssignExpr(size_t position, Expr *dest, Expr *value) : Expr(position), dest(dest), value(value) {}
		~AssignExpr()
		{
			delete value;
			delete dest;
		}

		void print() const
		{
			printf("(AssignExpr at %zu)\n", position);
			printf("\name: ");
			dest->print();
			printf("\n\tvalue: ");
			value->print();
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct BopExpr : public Expr
	{
		unsigned char op;
		Expr *lhs;
		Expr *rhs;

		BopExpr(size_t position, unsigned char op, Expr *lhs, Expr *rhs) : Expr(position), op(op), lhs(lhs), rhs(rhs) {}
		~BopExpr()
		{
			delete lhs;
			delete rhs;
		}

		void print() const
		{
			printf("(BopExpr at %zu)\n", position);
			printf("\top id: %d\n", op);
			printf("\tlhs:");
			lhs->print();
			printf("\n\trhs false:");
			rhs->print();
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct NegExpr : public Expr
	{
		Expr *value;

		NegExpr(size_t position, Expr *value) : Expr(position), value(value) {}
		~NegExpr()
		{
			delete value;
		}

		void print() const
		{
			printf("(NegExpr at %zu)\n", position);
			printf("\tvalue:");
			value->print();
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct NotExpr : public Expr
	{
		Expr *value;

		NotExpr(size_t position, Expr *value) : Expr(position), value(value) {}
		~NotExpr()
		{
			delete value;
		}

		void print() const
		{
			printf("(NotExpr at %zu)\n", position);
			printf("\tvalue:");
			value->print();
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct IdentifierExpr : public Expr
	{
		string name;

		IdentifierExpr(size_t position, string name) : Expr(position), name(name) {}

		void print() const
		{
			printf("(IdentifierExpr at %zu)\n", position);
			printf("\tname: %s\n", name.c_str());
		}

		string codegen(Generator &gen) const;
	};
	struct TernaryExpr : public Expr
	{
		Expr *condition;
		Expr *if_true;
		Expr *if_false;

		TernaryExpr(size_t position, Expr *condition, Expr *ift, Expr *iff) : Expr(position), condition(condition), if_true(ift), if_false(iff) {}
		~TernaryExpr()
		{
			delete condition;
			delete if_true;
			delete if_false;
		}

		void print() const
		{
			printf("(TernaryExpr at %zu)\n", position);
			printf("\tcondition:");
			condition->print();
			printf("\n\tif true:");
			if_true->print();
			printf("\n\tif false:");
			if_false->print();
			printf("\n");
		}

		string codegen(Generator &gen) const;
	};
	struct StrLitExpr : public Expr
	{
		string value;

		StrLitExpr(size_t position, string value) : Expr(position), value(value) {}

		void print() const
		{
			printf("(StrLitExpr at %zu)\n", position);
			printf("\tvalue: %s\n", value.c_str());
		}

		string codegen(Generator &gen) const;
	};
	struct IntLitExpr : public Expr
	{
		long long value;

		IntLitExpr(size_t position, long long value) : Expr(position), value(value) {}

		void print() const
		{
			printf("(IntLitExpr at %zu)\n", position);
			printf("\tvalue: %lld\n", value);
		}

		string codegen(Generator &gen) const;
	};
	struct BoolLitExpr : public Expr
	{
		bool value;

		BoolLitExpr(size_t position, bool value) : Expr(position), value(value) {}

		void print() const
		{
			printf("(BoolLitExpr at %zu)\n", position);
			printf("\tvalue: %s\n", value ? "true" : "false");
		}

		string codegen(Generator &gen) const;
	};
	struct FloatLitExpr : public Expr
	{
		long double value;

		FloatLitExpr(size_t position, long double value) : Expr(position), value(value) {}

		void print() const
		{
			printf("(FloatLitExpr at %zu)\n", position);
			printf("\tvalue: %Lf\n", value);
		}

		string codegen(Generator &gen) const;
	};
	struct CharLitExpr : public Expr
	{
		unsigned int value; // support UTF-8 or something might reduce it later

		CharLitExpr(size_t position, unsigned int value) : Expr(position), value(value) {}

		void print() const
		{
			printf("(CharLitExpr at %zu)\n", position);
			printf("\tvalue: %c\n", value);
		}

		string codegen(Generator &gen) const;
	};
}

#endif // PARSER_NODE_HPP