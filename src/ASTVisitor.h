#pragma once
#include "Result.h"

template<typename T>
class ASTVisitor
{
public:
	virtual T visit(const class ASTIntLiteralNode&) = 0;
	virtual T visit(const class ASTFloatLiteralNode&) = 0;
	virtual T visit(const class ASTStringLiteralNode&) = 0;
	virtual T visit(const class ASTUnaryNode&) = 0;
	virtual T visit(const class ASTBinaryNode&) = 0;
	virtual T visit(const class ASTIdentifierNode&) = 0;
	virtual T visit(const class ASTLetNode&) = 0;
};