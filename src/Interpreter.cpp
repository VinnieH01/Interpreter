#include "Interpreter.h"
#include "AST.h"

InterpreterResult Interpreter::interpret(const ASTNode& node)
{
	return node.accept(*this);
}

InterpreterResult Interpreter::visit(const ASTLiteralNode& node)
{
	const auto& literal_value = node.get_value();
	
	switch (literal_value.index())
	{
	case 0:
		return Value(std::get<0>(literal_value));
	case 1:
		return Value(std::get<1>(literal_value));
	case 2:
		return Value(std::get<2>(literal_value));
	}
}

InterpreterResult Interpreter::visit(const ASTArrayInitNode& node)
{
	InterpreterResult size_expr_res = node.get_size_expr()->accept(*this);
	if (size_expr_res.is_error())
		return size_expr_res;

	const Value& size = *size_expr_res;
	if (!std::holds_alternative<int>(size))
		return "Array can only be initialised with integer size";

	return Value(Array(node.get_type(), std::get<int>(size)));
}

InterpreterResult Interpreter::visit(const ASTIdentifierNode& node)
{
	if (m_symbol_table.find(node.get_name()) == m_symbol_table.end())
		return "Symbol does not exist error";

	return Value(m_symbol_table.at(node.get_name()));
}

InterpreterResult Interpreter::visit(const ASTUnaryNode& node)
{
	InterpreterResult operand_res = node.get_operand()->accept(*this);
	if (operand_res.is_error()) return operand_res;

	const Value& operand = *operand_res;

	switch (operand.index())
	{
	case 0:
		return Value(std::get<0>(operand) * (node.get_operator() == "-" ? -1 : 1));
	case 1:
		return Value(std::get<1>(operand) * (node.get_operator() == "-" ? -1 : 1));
	case 2:
		return Value(std::get<2>(operand) * (node.get_operator() == "-" ? -1 : 1));
	}
}

template<int index>
InterpreterResult binary_operation_helper(const Value& lhs, const Value& rhs, const std::string& op)
{
	if (op == "+") return Value(std::get<index>(lhs) + std::get<index>(rhs));
	if (op == "-") return Value(std::get<index>(lhs) - std::get<index>(rhs));
	if (op == "*") return Value(std::get<index>(lhs) * std::get<index>(rhs));
	if (op == "/") return Value(std::get<index>(lhs) / std::get<index>(rhs));

	return "Unsupported binary operator";
};

InterpreterResult Interpreter::visit(const ASTBinaryNode& node)
{
	InterpreterResult lhs_res = node.get_lhs()->accept(*this);
	if (lhs_res.is_error()) return lhs_res.get_error();

	InterpreterResult rhs_res = node.get_rhs()->accept(*this);
	if (rhs_res.is_error()) return rhs_res.get_error();

	const auto& lhs = *lhs_res;
	const auto& rhs = *rhs_res;

	const std::string& op = node.get_operator();

	if (lhs.index() != rhs.index())
		return "Incompatible types in binary operation";

	switch (lhs.index())
	{
	case 0:
		return binary_operation_helper<0>(lhs, rhs, op);
	case 1:
		return binary_operation_helper<1>(lhs, rhs, op);
	case 2:
		return binary_operation_helper<2>(lhs, rhs, op);
	}

	return "Incompatible types in binary operation";
}

InterpreterResult Interpreter::visit(const ASTLetNode& node)
{
	InterpreterResult expr = node.get_expr()->accept(*this);
	if (expr.is_error()) return expr.get_error();

	m_symbol_table[node.get_var_name()] = *expr;

	return {};
}
