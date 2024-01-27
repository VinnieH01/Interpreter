#include "Interpreter.h"
#include "AST.h"

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
	const InterpreterResult& operand = node.get_operand()->accept(*this);
	if (operand.is_error()) return operand;

	return *operand * (node.get_operator() == "-" ? -1 : 1);
}

InterpreterResult Interpreter::visit(const ASTBinaryNode& node)
{
	const InterpreterResult& lhs = node.get_lhs()->accept(*this);
	if (lhs.is_error()) return lhs.get_error();

	const InterpreterResult& rhs = node.get_rhs()->accept(*this);
	if (rhs.is_error()) return rhs.get_error();

	const std::string& op = node.get_operator();

	if (op == "+") return *lhs + *rhs;
	if (op == "-") return *lhs - *rhs;
	if (op == "*") return *lhs * *rhs;
	if (op == "/") return *lhs / *rhs;
}

InterpreterResult Interpreter::visit(const ASTLetNode& node)
{
	const InterpreterResult& expr = node.get_expr()->accept(*this);
	if (expr.is_error()) return expr.get_error();

	m_symbol_table[node.get_var_name()] = *expr;
	return {};
}
