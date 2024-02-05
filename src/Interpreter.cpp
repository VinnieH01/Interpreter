#include "Interpreter.h"
#include "AST.h"
#include "Value.h"
#include <iostream>

InterpreterResult Interpreter::interpret(const ASTNode& node)
{
	return node.accept(*this);
}

InterpreterResult Interpreter::visit(const ASTLiteralNode& node)
{
	return node.get_value();
}

InterpreterResult Interpreter::visit(const ASTIdentifierNode& node)
{
	if (m_symbol_table.find(node.get_name()) == m_symbol_table.end())
		return "Symbol does not exist error";

	return m_symbol_table.at(node.get_name());
}

InterpreterResult Interpreter::visit(const ASTUnaryNode& node)
{
	InterpreterResult operand_res = node.get_operand()->accept(*this);
	if (operand_res.is_error()) 
		return operand_res;

	Value* operand = (*operand_res).get();

	int mult = (node.get_operator() == "-" ? -1 : 1);

	if (auto* val = dynamic_cast<NumberValue<int>*>(operand))
		return { std::make_shared<NumberValue<int>>((*val) * mult)};

	if (auto* val = dynamic_cast<NumberValue<float>*>(operand))
		return { std::make_shared<NumberValue<float>>((*val) * mult) };

	if (auto* val = dynamic_cast<NumberValue<char>*>(operand))
		return { std::make_shared<NumberValue<char>>((*val) * mult) };

	return "Unsupported unary operation";
}

InterpreterResult Interpreter::visit(const ASTIfNode& node)
{
	InterpreterResult condition_res = node.get_conditon()->accept(*this);
	if (condition_res.is_error())
		return condition_res;

	if (isTruthy((*condition_res).get()))
	{
		return node.get_then_stmt()->accept(*this);
	}
	else
	{
		ASTNode* else_stmt = node.get_else_stmt().get();
		if(else_stmt)
			return else_stmt->accept(*this);
	}

	return {};
}

InterpreterResult Interpreter::visit(const ASTPrintNode& node)
{
	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;

	Value* value = (*expr_res).get();

	if (auto* val = dynamic_cast<NumberValue<int>*>(value))
		std::cout << ">> " << val->value << std::endl;
	else if (auto* val = dynamic_cast<NumberValue<float>*>(value))
		std::cout << ">> " << val->value << std::endl;
	else if (auto* val = dynamic_cast<NumberValue<char>*>(value))
		std::cout << ">> " << val->value << std::endl;
	else if (auto* val = dynamic_cast<StringValue*>(value))
		std::cout << ">> " << val->text << std::endl;
	else
		return "Unsupported expression in print statement.";

	return {};
}

InterpreterResult Interpreter::visit(const ASTInputNode&)
{
	std::string input;
	std::cout << "Input: ";
	std::getline(std::cin, input);

	return { std::make_shared<StringValue>(input) };
}

template<typename T, typename T2>
bool Interpreter::number_op(Value* lhs, Value* rhs, const std::string& op, std::shared_ptr<Value>& out)
{
	if (auto* val = dynamic_cast<NumberValue<T>*>(lhs))
	{
		if (auto* val2 = dynamic_cast<NumberValue<T2>*>(rhs))
		{
			if (op == "+")
				out = std::make_shared<NumberValue<T>>(val->value + val2->value);
			else if (op == "-")
				out = std::make_shared<NumberValue<T>>(val->value - val2->value);
			else if (op == "*")
				out = std::make_shared<NumberValue<T>>(val->value * val2->value);
			else if (op == "/")
				out = std::make_shared<NumberValue<T>>(val->value / val2->value);
			else if (op == "==")
				out = std::make_shared<NumberValue<int>>(val->value == val2->value);
			else if (op == "<=")
				out = std::make_shared<NumberValue<int>>(val->value <= val2->value);
			else if (op == ">=")
				out = std::make_shared<NumberValue<int>>(val->value >= val2->value);
			else if (op == "<")
				out = std::make_shared<NumberValue<int>>(val->value < val2->value);
			else if (op == ">")
				out = std::make_shared<NumberValue<int>>(val->value > val2->value);
			else if (op == "&&")
				out = std::make_shared<NumberValue<int>>(isTruthy(val) && isTruthy(val2));
			else if (op == "||")
				out = std::make_shared<NumberValue<int>>(isTruthy(val) || isTruthy(val2));
		}
	}

	return out.get() != nullptr;
};

InterpreterResult Interpreter::visit(const ASTBinaryNode& node)
{
	InterpreterResult lhs_res = node.get_lhs()->accept(*this);
	if (lhs_res.is_error()) return lhs_res.get_error();

	InterpreterResult rhs_res = node.get_rhs()->accept(*this);
	if (rhs_res.is_error()) return rhs_res.get_error();

	Value* lhs = (*lhs_res).get();
	Value* rhs = (*rhs_res).get();

	const std::string& op = node.get_operator();

	std::shared_ptr<Value> value(nullptr);

	if (number_op<int, int>(lhs, rhs, op, value)) 
		return value;
	if (number_op<float, float>(lhs, rhs, op, value))
		return value;
	if (number_op<char, char>(lhs, rhs, op, value))
		return value;

	if (op == "+" || op == "==")
	{
		if (auto* str = dynamic_cast<StringValue*>(lhs))
		{
			if (auto* str2 = dynamic_cast<StringValue*>(rhs))
			{
				if (op == "+")
					return { std::make_shared<StringValue>(str->text + str2->text) };
				if (op == "==")
					return { std::make_shared<NumberValue<int>>(str->text == str2->text) };
			}
		}
	}

	return "Incompatible types in binary operation";
}

InterpreterResult Interpreter::visit(const ASTBlockNode& node)
{
	for (const auto& stmt : node.get_stmts())
	{
		InterpreterResult stmt_res = stmt->accept(*this);
		if (stmt_res.is_error()) 
			return stmt_res;
	}

	return {};
}

InterpreterResult Interpreter::visit(const ASTLetNode& node)
{
	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error()) 
		return expr_res;

	m_symbol_table[node.get_var_name()] = *expr_res;

	return {};
}

bool Interpreter::isTruthy(Value* value)
{
	if (auto* val = dynamic_cast<NumberValue<int>*>((value)))
		return *val != 0;
	if (auto* val = dynamic_cast<NumberValue<float>*>((value)))
		return *val != 0;
	if (auto* val = dynamic_cast<NumberValue<char>*>((value)))
		return *val != 0;

	return false;
}
