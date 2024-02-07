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

template<typename T, typename T2>
bool Interpreter::number_op(Value* lhs, Value* rhs, Operator op, std::shared_ptr<Value>& out)
{
	if (auto* val = dynamic_cast<NumberValue<T>*>(lhs))
	{
		if (auto* val2 = dynamic_cast<NumberValue<T2>*>(rhs))
		{
			switch (op) 
			{
			case Operator::PLUS:
				out = std::make_shared<NumberValue<T>>(val->value + val2->value);
				break;
			case Operator::MINUS:
				out = std::make_shared<NumberValue<T>>(val->value - val2->value);
				break;
			case Operator::TIMES:
				out = std::make_shared<NumberValue<T>>(val->value * val2->value);
				break;
			case Operator::DIVIDED:
				out = std::make_shared<NumberValue<T>>(val->value / val2->value);
				break;
			case Operator::EQUALS:
				out = std::make_shared<NumberValue<int>>(val->value == val2->value);
				break;
			case Operator::LEQ:
				out = std::make_shared<NumberValue<int>>(val->value <= val2->value);
				break;
			case Operator::GEQ:
				out = std::make_shared<NumberValue<int>>(val->value >= val2->value);
				break;
			case Operator::LESS_THAN:
				out = std::make_shared<NumberValue<int>>(val->value < val2->value);
				break;
			case Operator::GREATER_THAN:
				out = std::make_shared<NumberValue<int>>(val->value > val2->value);
				break;
			case Operator::AND:
				out = std::make_shared<NumberValue<int>>(val->is_truthy() && val2->is_truthy());
				break;
			case Operator::OR:
				out = std::make_shared<NumberValue<int>>(val->is_truthy() || val2->is_truthy());
				break;
			}
		}
	}

	return out.get() != nullptr;
};

InterpreterResult Interpreter::visit(const ASTBinaryNode& node)
{
	//TODO Use some kind of double dispatch here as well

	InterpreterResult lhs_res = node.get_lhs()->accept(*this);
	if (lhs_res.is_error()) return lhs_res.get_error();

	InterpreterResult rhs_res = node.get_rhs()->accept(*this);
	if (rhs_res.is_error()) return rhs_res.get_error();

	Value* lhs = (*lhs_res).get();
	Value* rhs = (*rhs_res).get();

	Operator op = node.get_operator();

	std::shared_ptr<Value> value(nullptr);

	if (number_op<int, int>(lhs, rhs, op, value)) 
		return value;
	if (number_op<float, float>(lhs, rhs, op, value))
		return value;
	if (number_op<char, char>(lhs, rhs, op, value))
		return value;

	if (op == Operator::PLUS || op == Operator::EQUALS)
	{
		if (auto* str = dynamic_cast<StringValue*>(lhs))
		{
			if (auto* str2 = dynamic_cast<StringValue*>(rhs))
			{
				if (op == Operator::PLUS)
					return { std::make_shared<StringValue>(str->text + str2->text) };
				if (op == Operator::EQUALS)
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

/*
 * UNARY
*/

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<int>& value)
{
	return visit_number(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<float>& value)
{
	return visit_number(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<char>& value)
{
	return visit_number(value);
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
		return { std::make_shared<StringValue>(std::to_string(value.value)) };

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
