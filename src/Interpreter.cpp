#include "Interpreter.h"
#include "AST.h"
#include "Value.h"

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

#undef apply_unary

}

template<typename T, typename T2>
bool number_op(Value* lhs, Value* rhs, const std::string& op, std::shared_ptr<Value>& out)
{
	if (auto* val = dynamic_cast<NumberValue<T>*>(lhs))
	{
		if (auto* val2 = dynamic_cast<NumberValue<T2>*>(rhs))
		{
			if (op == "+")
			{
				out = std::make_shared<NumberValue<T>>(val->value + val2->value);
			}
			else if (op == "-")
			{
				out = std::make_shared<NumberValue<T>>(val->value - val2->value);
			}
			else if (op == "*")
			{
				out = std::make_shared<NumberValue<T>>(val->value * val2->value);
			}
			else if (op == "/")
			{
				out = std::make_shared<NumberValue<T>>(val->value / val2->value);
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

	const std::string& op = node.get_operator();

	std::shared_ptr<Value> value(nullptr);

	if (number_op<int, int>(lhs, rhs, op, value)) 
		return value;
	if (number_op<float, float>(lhs, rhs, op, value))
		return value;
	if (number_op<char, char>(lhs, rhs, op, value))
		return value;

	return "Incompatible types in binary operation";
}

InterpreterResult Interpreter::visit(const ASTLetNode& node)
{
	InterpreterResult expr = node.get_expr()->accept(*this);
	if (expr.is_error()) return expr.get_error();

	m_symbol_table[node.get_var_name()] = *expr;

	return {};
}
