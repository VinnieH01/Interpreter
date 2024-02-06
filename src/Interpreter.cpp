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

	int mult = (node.get_operator() == Operator::MINUS ? -1 : 1);

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

InterpreterResult Interpreter::visit(const ASTCastNode& node)
{
	//TODO: Refactor this thing and remove duplication (will rework the entire interpreter code soon anyway)

	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;
	
	Value* expr_value = (*expr_res).get();

	if (node.get_type() == Type::INT) 
	{
		if (dynamic_cast<NumberValue<int>*>(expr_value))
			return expr_res;
		if (auto* val = dynamic_cast<NumberValue<float>*>(expr_value))
			return { std::make_shared<NumberValue<int>>(val->value) };
		if (auto* val = dynamic_cast<NumberValue<char>*>(expr_value))
			return { std::make_shared<NumberValue<int>>(val->value) };

		if (auto* val = dynamic_cast<StringValue*>(expr_value))
		{
			int casted = 0;
			try
			{
				casted = std::stoi(val->text);
			}
			catch (const std::exception&) 
			{
				return "String is not a valid integer";
			}

			return { std::make_shared<NumberValue<int>>(casted) };
		}
	}

	if (node.get_type() == Type::FLOAT)
	{
		if (auto* val = dynamic_cast<NumberValue<int>*>(expr_value))
			return { std::make_shared<NumberValue<float>>(val->value) };
		if (dynamic_cast<NumberValue<float>*>(expr_value))
			return expr_res;
		if (auto* val = dynamic_cast<NumberValue<char>*>(expr_value))
			return { std::make_shared<NumberValue<float>>(val->value) };

		if (auto* val = dynamic_cast<StringValue*>(expr_value))
		{
			float casted = 0;
			try
			{
				casted = std::stof(val->text);
			}
			catch (const std::exception&)
			{
				return "String is not a valid float";
			}

			return { std::make_shared<NumberValue<float>>(casted) };
		}
	}

	if (node.get_type() == Type::CHAR)
	{
		if (auto* val = dynamic_cast<NumberValue<int>*>(expr_value))
			return { std::make_shared<NumberValue<char>>(val->value) };
		if (dynamic_cast<NumberValue<char>*>(expr_value))
			return expr_res;
	}

	if (node.get_type() == Type::STRING)
	{
		if (auto* val = dynamic_cast<NumberValue<int>*>(expr_value))
			return { std::make_shared<StringValue>(std::to_string(val->value)) };
		if (auto* val = dynamic_cast<NumberValue<float>*>(expr_value))
			return { std::make_shared<StringValue>(std::to_string(val->value)) };
		if (auto* val = dynamic_cast<NumberValue<char>*>(expr_value))
			return { std::make_shared<StringValue>(std::string(1, *val)) };

		if (dynamic_cast<StringValue*>(expr_value))
			return expr_res;
	}
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
