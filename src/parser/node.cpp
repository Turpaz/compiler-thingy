#include "node.hpp"

namespace Nodes
{
	void BlockStmt::codegen(Generator &gen) const
	{
		// gen.code += "{\n";
		for (Stmt *stmt : stmts)
		{
			stmt->codegen(gen);
		}
		// gen.code += "}\n";
	}

	void VarDeclStmt::codegen(Generator &gen) const
	{
		if (gen.prog.named_variables.find(name) != gen.prog.named_variables.end())
		{
			gen.Error("Variable \"" + name + "\" already declared", position);
		}

		string fv = gen.toFreeVariable(value->codegen(gen));
		gen.code += "%" + gen.vci() + " = alloca " + type + "\n";
		gen.code += "store " + type + " " + fv + ", ptr %" + gen.vcl() + "\n";
		gen.prog.variables["%" + gen.vcl()] = type;
		gen.prog.named_variables[name] = "%" + gen.vcl();
	}

	void IteStmt::codegen(Generator &gen) const
	{
		string c = cond->codegen(gen);

		string id = gen.vci();
		gen.code += "%" + id + " = icmp ne " + gen.prog.variables.at(c) + " " + c + ", 0\n";
		gen.prog.addVar("%" + id, "i1");
		gen.code += "br i1 %" + id + ", label %ifthen" + id + ", label %ifelse" + id + "\n";

		gen.code += "ifthen" + id + ":\n";
		then_b->codegen(gen);
		gen.code += "br label %ifend" + id + "\n";
		// gen.code += "ttoend" + id + ":\n\tbr label %ifend" + id + "\n"; // if i don't have this label and the block has a return statement it's automatically creating a new numbered label so my variable naming will be wrong

		gen.code += "ifelse" + id + ":\n";
		else_b->codegen(gen);
		gen.code += "br label %ifend" + id + "\n";

		gen.code += "ifend" + id + ":\n";
	}

	void FuncDeclStmt::codegen(Generator &gen) const
	{
		// might need to move this elsewhere
		vector<string> params_without_names;
		for (auto p : params)
		{
			params_without_names.push_back(p.second);
		}

		gen.prog.addFunc(name, rType, params_without_names); // add the function to the program struct

		string actual_rType = rType;						 // rType translated to llvm ir code
		vector<pair<string, string>> actual_params = params; // params translated to llvm ir code
		if (name == "main")
		{
			actual_rType = "i32";
			actual_params.clear();
			actual_params.push_back(pair<string, string>("i32", "argc"));
			actual_params.push_back(pair<string, string>("ptr", "argv"));
			body->stmts.push_back(new RetStmt(position, new IntLitExpr(position, 0)));
		}

		gen.code += "define " + actual_rType + " @" + name + "(";
		for (auto p : actual_params)
		{
			gen.code += p.first + " %" + p.second + ",";
		}
		gen.code[gen.code.size() - 1] = ')';
		gen.code += "\n{\n";
		body->codegen(gen);
		gen.code += "}\n";
	}

	void RetStmt::codegen(Generator &gen) const
	{
		string fv = gen.toFreeVariable(value->codegen(gen)); // the variable
		string t = gen.prog.variables[fv];

		if (gen.prog.functions.empty())
		{
			gen.Error("Unexpected return statement, no functions declared", position);
		}

		string rtype = std::prev(gen.prog.functions.end())->second.first;

		if (gen.isIntegerType(t) && gen.isIntegerType(rtype))
		{
			fv = gen.integerCast(fv, rtype); // Make sure our types match
		}

		gen.code += "ret " + gen.prog.variables.at(fv) + " " + fv + "\n";

		gen.vci(); // increment... because of something with it creating an automatic new numbered label
	}

	void ExprStmt::codegen(Generator &gen) const
	{
		// works for now
		expr->codegen(gen);
	}

	string FuncCallExpr::codegen(Generator &gen) const
	{
		string code = "";
		string rType_params = gen.prog.getFuncRetType(name) + " " + gen.prog.getFuncArgs(name); // getting the return type and parameter structure of the called function
		code += "%" + gen.vci() + " = call " + rType_params + " @" + name + "(";				// calling the function and storing the return value in a variable
		gen.prog.addVar("%" + gen.vcl(), gen.prog.getFuncRetType(name));						// adding the variable to the program struct

		if (gen.prog.functions[name].second.size() != args.size())
		{
			// error wrong number of arguments given
			gen.Error("Expected " + std::to_string(gen.prog.functions[name].second.size()) + " arguments, " + std::to_string(args.size()) + " where given", position);
		}
		for (int i = 0; i < args.size(); i++)
		{
			string fv = gen.toFreeVariable(args[i]->codegen(gen)); // the variable

			string pt = gen.prog.functions[name].second[i]; // the type of the parameter
			string at = gen.prog.variables.at(fv);			// the type of the argument
			if (pt != at)
			{
				// error wrong type of variable given
				gen.Error("Expected argument of type \"" + pt + "\", type \"" + at + "\" was given", args[i]->position);
			}
			code += at + " " + fv + ",";
		}
		code[code.size() - 1] = ')'; // replacing the last ',' with ')'
		code += '\n';

		gen.code += code;
		return "%" + gen.vcl();
	}

	string PrintlnCallExpr::codegen(Generator &gen) const
	{
		string name = "println";

		string code = "";
		string rType_params = gen.prog.getFuncRetType(name) + " " + gen.prog.getFuncArgs(name); // getting the return type and parameter structure of the called function
		code += "%" + gen.vci() + " = call " + rType_params + " @" + name + "(";				// calling the function and storing the return value in a variable
		gen.prog.addVar("%" + gen.vcl(), gen.prog.getFuncRetType(name));						// adding the variable to the program struct

		if (gen.prog.functions[name].second.size() != args.size())
		{
			// error wrong number of arguments given
			gen.Error("Expected " + std::to_string(gen.prog.functions[name].second.size()) + " arguments, got " + std::to_string(args.size()), position);
		}
		for (int i = 0; i < args.size(); i++)
		{
			string fv = gen.toFreeVariable(args[i]->codegen(gen)); // the variable

			string pt = gen.prog.functions[name].second[i]; // the type of the parameter
			string at = gen.prog.variables.at(fv);			// the type of the argument
			if (pt != at)
			{
				// error wrong type of variable given
				gen.Error("Expected argument of type \"" + pt + "\", type \"" + at + "\" was given", args[i]->position);
			}
			code += at + " " + fv + ",";
		}
		code[code.size() - 1] = ')'; // replacing the last ',' with ')'
		code += '\n';

		gen.code += code;
		return "%" + gen.vcl();
	}

	string ExitCallExpr::codegen(Generator &gen) const
	{
		string name = "exit";

		string rType_params = gen.prog.getFuncRetType(name) + " " + gen.prog.getFuncArgs(name); // getting the return type and parameter structure of the called function
		gen.code += "%" + gen.vci() + " = call " + rType_params + " @" + name + "(";			// calling the function and storing the return value in a variable
		gen.prog.addVar("%" + gen.vcl(), gen.prog.getFuncRetType(name));						// adding the variable to the program struct

		if (gen.prog.functions[name].second.size() != args.size())
		{
			// error wrong number of arguments given
			gen.Error("Expected " + std::to_string(gen.prog.functions[name].second.size()) + " arguments, " + std::to_string(args.size()) + " where given", position);
		}
		for (int i = 0; i < args.size(); i++)
		{
			string fv = gen.toFreeVariable(args[i]->codegen(gen)); // the variable

			string pt = gen.prog.functions[name].second[i]; // the type of the parameter
			string at = gen.prog.variables.at(fv);			// the type of the argument
			if (pt != at)
			{
				// error wrong type of variable given
				gen.Error("Expected argument of type \"" + pt + "\", type \"" + at + "\" was given", position);
			}
			gen.code += at + " " + fv + ",";
		}
		gen.code[gen.code.size() - 1] = ')'; // replacing the last ',' with ')'
		gen.code += '\n';

		return "%" + gen.vcl();
	}

	string AssignExpr::codegen(Generator &gen) const
	{
		string fv = value->codegen(gen);
		string fd = dest->codegen(gen);

		if (gen.prog.variable_sources.find(fd) == gen.prog.variable_sources.end())
		{
			// the left side is most likely not a named variable, anyway it's not changeable
			gen.Error("Can't assign a new value to an immutable variable", position);
		}

		fd = gen.prog.variable_sources.at(fd); // target the address variable- the one you allocate. not the extracted value

		if (gen.prog.variables.at(fv) != gen.prog.variables.at(fd))
		{
			// types don't match
			gen.Error("Can't assign expression of type \"" + gen.prog.variables.at(fv) + "\" to a variable of type \"" + gen.prog.variables.at(fd) + "\"", position);
		}

		gen.code += "store " + gen.prog.variables.at(fv) + " " + fv + ", ptr " + fd + "\n";

		return gen.prog.variables.at(fd); // ?
	}

	string BopExpr::codegen(Generator &gen) const
	{
		string vlhs = lhs->codegen(gen);
		string vrhs = rhs->codegen(gen);

		if (gen.prog.variables.at(vlhs) != gen.prog.variables.at(vrhs))
		{
			gen.Error("Expected the two sides of the binary expression to be of the same type (one was \"" + gen.prog.variables.at(vlhs) + "\" the other was \"" + gen.prog.variables.at(vrhs) + "\")", position);
		}

		string exprType = gen.prog.variables.at(vlhs);

		string vresult = "%bop" + std::to_string(gen.bvc++);

		// currently only supports integer types, add fadd, fsub, fcmp, ... to support floating types

		if (op == BOPS::BOP_ADD) // +
		{
			gen.code += vresult + " = add " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_SUB) // -
		{
			gen.code += vresult + " = sub " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_MUL) // *
		{
			gen.code += vresult + " = mul " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_DIV) // /
		{
			// currently only supports signed integer operands and result (3 / 2 = 1)
			gen.code += vresult + " = sdiv " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_MOD) // %
		{
			// currently only supports signed integer operands and result
			gen.code += vresult + " = srem " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_POW) // ^
		{
			// currently only supports signed integer operands and result
			gen.code += vresult + " = call " + exprType + " @power(" + exprType + " " + vlhs + ", " + exprType + " " + vrhs + ")";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_OR) // ||
		{
			gen.code += "%" + gen.vci() + " = or " + exprType + " " + vlhs + ", " + vrhs + "\n"; // TODO: validate
			gen.prog.addVar("%" + gen.vcl(), exprType);
			gen.code += vresult + " = icmp ne " + exprType + " 0, %" + gen.vcl() + "\n";
			gen.prog.addVar(vresult, "i1");
		}
		else if (op == BOPS::BOP_AND) // &&
		{
			gen.code += "%" + gen.vci() + " = icmp ne " + exprType + " 0, " + vlhs + "\n";
			gen.prog.addVar("%a" + gen.vcl(), "i1");
			gen.code += "%b" + gen.vcl() + " = icmp ne " + exprType + " 0, " + vrhs + "\n";
			gen.prog.addVar("%b" + gen.vcl(), "i1");
			gen.code += vresult + " = and i1 %" + gen.vcl() + ", %b" + gen.vcl() + "\n"; // if lhs: return rhs; else: return 0;
			gen.prog.addVar(vresult, "i1");
		}
		else if (op == BOPS::BOP_EQ) // ==
		{
			gen.code += vresult + " = icmp eq " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_NEQ) // !=
		{
			gen.code += vresult + " = icmp ne " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_GT) // !=
		{
			// currently only works for signed integers
			gen.code += vresult + " = icmp sgt " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_GTE) // !=
		{
			// currently only works for signed integers
			gen.code += vresult + " = icmp sge " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_LT) // !=
		{
			// currently only works for signed integers
			gen.code += vresult + " = icmp slt " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		else if (op == BOPS::BOP_LTE) // !=
		{
			// currently only works for signed integers
			gen.code += vresult + " = icmp sle " + exprType + " " + vlhs + ", " + vrhs + "\n";
			gen.prog.addVar(vresult, exprType);
		}
		// TODO: Finish
		else
		{
			gen.Error("Unknown binary operator", position);
		}

		return vresult;
	}

	string NegExpr::codegen(Generator &gen) const
	{
		string v = value->codegen(gen);

		// add support for floating points - fmul
		gen.code += "%" + gen.vci() + " = mul " + gen.prog.variables.at(v) + " " + v + ", -1\n";
		gen.prog.addVar("%" + gen.vcl(), gen.prog.variables.at(v));

		return "%" + gen.vcl();
	}

	string NotExpr::codegen(Generator &gen) const
	{
		string v = value->codegen(gen);

		// TODO: add support for floating points
		gen.code += "%" + gen.vci() + " = icmp eq " + gen.prog.variables.at(v) + " " + v + ", 0\n";
		gen.prog.addVar("%" + gen.vcl(), "i1");

		return "%" + gen.vcl();
	}

	string IdentifierExpr::codegen(Generator &gen) const
	{
		if (gen.prog.named_variables.find(name) == gen.prog.named_variables.end())
		{
			// variable doesn't exist
			gen.Error("Variable \"" + name + "\" wasn't declared", position);
		}

		gen.code += "%" + gen.vci() + " = load " + gen.prog.variables[gen.prog.named_variables[name]] + ", ptr " + gen.prog.named_variables[name] + "\n";
		gen.prog.variables["%" + gen.vcl()] = gen.prog.variables[gen.prog.named_variables[name]];
		gen.prog.variable_sources["%" + gen.vcl()] = gen.prog.named_variables.at(name);

		return "%" + gen.vcl();
	}

	string TernaryExpr::codegen(Generator &gen) const
	{
		string cond = condition->codegen(gen);
		string t = if_true->codegen(gen);  // to the branch
		string f = if_false->codegen(gen); // to the branch

		if (gen.prog.variables.at(t) != gen.prog.variables.at(f))
		{
			gen.Error("The two possible values in a ternary expression must be of the same type (one was \"" + gen.prog.variables.at(t) + "\" the other was \"" + gen.prog.variables.at(f) + "\")", position);
		}

		string cond_res = "%" + gen.vci();
		gen.code += cond_res + " = icmp ne " + gen.prog.variables.at(cond) + " 0, " + cond + "\n";
		gen.prog.addVar(cond_res, "i1"); // add variable to prog

		gen.code += "%" + gen.vci() + " = select " + gen.prog.variables.at(cond_res) + " " + cond_res + ", " + gen.prog.variables.at(t) + " " + t + ", " + gen.prog.variables.at(f) + " " + f + "\n";
		gen.prog.addVar("%" + gen.vcl(), gen.prog.variables.at(t)); // add variable to prog

		return "%" + gen.vcl();
	}

	string StrLitExpr::codegen(Generator &gen) const
	{
		return gen.addStrLit(value);
	}

	string IntLitExpr::codegen(Generator &gen) const
	{
		return gen.addIntLit(value);
	}

	string BoolLitExpr::codegen(Generator &gen) const
	{
		return gen.addBoolLit(value);
	}

	string FloatLitExpr::codegen(Generator &gen) const
	{
		return gen.addFloatLit(value);
	}

	string CharLitExpr::codegen(Generator &gen) const
	{
		return gen.addCharLit(value);
	}
}