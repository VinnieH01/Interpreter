#pragma once

#include <unordered_map>
#include <string>
#include "ASTVisitor.h"
#include "AST.h"
#include <variant>


using Value = std::variant<int, float, std::string>;
using InterpreterResult = Result<Value, const char*>;

class Interpreter : public ASTVisitor<InterpreterResult>
{
public:
	InterpreterResult interpret(const ASTNode&);

	virtual InterpreterResult visit(const ASTIntLiteralNode&) override;
	virtual InterpreterResult visit(const ASTFloatLiteralNode&) override;
	virtual InterpreterResult visit(const ASTStringLiteralNode&) override;
	virtual InterpreterResult visit(const ASTIdentifierNode& node) override;
	virtual InterpreterResult visit(const ASTUnaryNode&) override;
	virtual InterpreterResult visit(const ASTBinaryNode&) override;
	virtual InterpreterResult visit(const ASTLetNode&) override;
private:
	std::unordered_map<std::string, Value> m_symbol_table;
};