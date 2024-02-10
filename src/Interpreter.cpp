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

	UnaryOperationVisitor visitor(node.get_operator());
	return operand->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTIfNode& node)
{
	InterpreterResult condition_res = node.get_conditon()->accept(*this);
	if (condition_res.is_error())
		return condition_res;

	if ((*condition_res)->is_truthy())
	{
		node.get_then_stmt()->accept(*this);
	}
	else
	{
		ASTNode* else_stmt = node.get_else_stmt().get();
		if(else_stmt)
			else_stmt->accept(*this);
	}

	return {}; //TODO separate statements and expressions so these don't have to return empty results
}

InterpreterResult Interpreter::visit(const ASTWhileNode& node)
{
	InterpreterResult condition_res = node.get_conditon()->accept(*this);
	if (condition_res.is_error())
		return condition_res;

	std::shared_ptr<Value> cond_expr = *condition_res;
	while (cond_expr->is_truthy())
	{
		node.get_then_stmt()->accept(*this);

		InterpreterResult condition_res = node.get_conditon()->accept(*this);
		if (condition_res.is_error())
			return condition_res;

		cond_expr = *condition_res;
	}

	return {};
}

InterpreterResult Interpreter::visit(const ASTPrintNode& node)
{
	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;

	Value* value = (*expr_res).get();

	PrintVisitor visitor;
	return value->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTCastNode& node)
{
	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;
	
	Value* expr_value = (*expr_res).get();
	CastVisitor visitor(node.get_type());
	return expr_value->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTInputNode&)
{
	std::string input;
	std::cout << "Input: ";
	std::getline(std::cin, input);

	return { std::make_shared<StringValue>(input) };
}

InterpreterResult Interpreter::visit(const ASTBinaryNode& node)
{
	InterpreterResult lhs_res = node.get_lhs()->accept(*this);
	if (lhs_res.is_error()) return lhs_res.get_error();

	InterpreterResult rhs_res = node.get_rhs()->accept(*this);
	if (rhs_res.is_error()) return rhs_res.get_error();

	Value* lhs = (*lhs_res).get();
	Value* rhs = (*rhs_res).get();

	BinaryOperationVisitor visitor(node.get_operator(), rhs);
	return lhs->accept(visitor);
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

/*
 * UNARY
*/

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<int>& value)
{
	return number_operation(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<float>& value)
{
	return number_operation(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<char>& value)
{
	return number_operation(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const StringValue&)
{
	return "Cannot perform unary operation on string";
}

/*
 * PRINT
*/

InterpreterResult Interpreter::PrintVisitor::visit(const NumberValue<int>& value)
{
	return print(value.value);
}

InterpreterResult Interpreter::PrintVisitor::visit(const NumberValue<float>& value)
{
	return print(value.value);
}

InterpreterResult Interpreter::PrintVisitor::visit(const NumberValue<char>& value)
{
	return print(value.value);
}

InterpreterResult Interpreter::PrintVisitor::visit(const StringValue& value)
{
	return print(value.text);
}

/*
 * CAST
*/

InterpreterResult Interpreter::CastVisitor::visit(const NumberValue<int>& value)
{
	if (type == Type::INT || type == Type::CHAR || type == Type::FLOAT)
		return num_to_num(value);
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(std::to_string(value.value)) };

	return "Cannot cast int to x"; //TODO Create better runtime errors (not just const char*) to replace x
}

InterpreterResult Interpreter::CastVisitor::visit(const NumberValue<float>& value)
{
	if (type == Type::INT || type == Type::FLOAT)
		return num_to_num(value);
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(std::to_string(value.value)) };

	return "Cannot cast float to x";
}

InterpreterResult Interpreter::CastVisitor::visit(const NumberValue<char>& value)
{
	if (type == Type::INT || type == Type::CHAR || type == Type::FLOAT)
		return num_to_num(value);
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(std::string(1, value.value)) };

	return "Cannot cast char to x";
}

InterpreterResult Interpreter::CastVisitor::visit(const StringValue& value)
{
	if (type == Type::INT)
		return str_to_num<int>(value.text, [](const std::string& str) { return std::stoi(str); });
	if (type == Type::FLOAT)
		return str_to_num<float>(value.text, [](const std::string& str) { return std::stof(str); });
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(value.text) };

	return "Cannot convert string to x";
}

/*
* BINARY OPERATION
*/

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const NumberValue<int>& value)
{
	return number_operation(value, Type::INT);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const NumberValue<float>& value)
{
	return number_operation(value, Type::FLOAT);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const NumberValue<char>& value)
{
	return number_operation(value, Type::CHAR);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const StringValue& value)
{
	if (auto* other_val = dynamic_cast<StringValue*>(other))
	{
		switch (op)
		{
		case Operator::PLUS:
			return { std::make_shared<StringValue>(value.text + other_val->text) };
		case Operator::EQUALS:
			return { std::make_shared<NumberValue<int>>(value.text == other_val->text) };
		}

		return "Binary operator is not supported on string";
	}

	return "Types are not compatible in binary operation";
}