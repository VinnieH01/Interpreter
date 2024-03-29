#pragma once

template<typename T>
class ASTVisitor
{
public:
	virtual ~ASTVisitor() = default;

	virtual T visit(const class ASTLiteralNode&) = 0;
	virtual T visit(const class ASTUnaryNode&) = 0;
	virtual T visit(const class ASTBlockNode&) = 0;
	virtual T visit(const class ASTBinaryNode&) = 0;
	virtual T visit(const class ASTPrintNode&) = 0;
	virtual T visit(const class ASTCastNode&) = 0;
	virtual T visit(const class ASTInputNode&) = 0;
	virtual T visit(const class ASTIfNode&) = 0;
	virtual T visit(const class ASTWhileNode&) = 0;
	virtual T visit(const class ASTIdentifierNode&) = 0;
	virtual T visit(const class ASTLetNode&) = 0;
	virtual T visit(const class ASTAssignmentNode&) = 0;
	virtual T visit(const class ASTFunctionNode&) = 0;
	virtual T visit(const class ASTCallNode&) = 0;
	virtual T visit(const class ASTReturnNode&) = 0;
};