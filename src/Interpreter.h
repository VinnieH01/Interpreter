#pragma once

#include <unordered_map>
#include <string>
#include "ASTVisitor.h"
#include "AST.h"

using InterpreterResult = Result<std::shared_ptr<Value>, const char*>;

class Interpreter : public ASTVisitor<InterpreterResult>
{
public:
	InterpreterResult interpret(const ASTNode&);

	virtual InterpreterResult visit(const ASTLiteralNode&) override;
	virtual InterpreterResult visit(const ASTIdentifierNode&) override;
	virtual InterpreterResult visit(const ASTUnaryNode&) override;
	virtual InterpreterResult visit(const ASTIfNode&) override;
	virtual InterpreterResult visit(const ASTBinaryNode&) override;
	virtual InterpreterResult visit(const ASTBlockNode&) override;
	virtual InterpreterResult visit(const ASTLetNode&) override;
private:
	bool isTruthy(Value* value);

	template<typename T, typename T2>
	bool number_op(Value* lhs, Value* rhs, const std::string& op, std::shared_ptr<Value>& out);

	std::unordered_map<std::string, std::shared_ptr<Value>> m_symbol_table;
};