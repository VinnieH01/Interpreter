#pragma once

#include <unordered_map>
#include <string>
#include "ASTVisitor.h"
#include "AST.h"


using InterpreterResult = Result<int, const char*>;
class Interpreter : public ASTVisitor<InterpreterResult>
{
public:
	InterpreterResult interpret(const ASTNode&);

	virtual InterpreterResult visit(const ASTLiteralNode&) override;
	virtual InterpreterResult visit(const ASTIdentifierNode& node) override;
	virtual InterpreterResult visit(const ASTUnaryNode&) override;
	virtual InterpreterResult visit(const ASTBinaryNode&) override;
	virtual InterpreterResult visit(const ASTLetNode&) override;
private:
	std::unordered_map<std::string, int> m_symbol_table;
};