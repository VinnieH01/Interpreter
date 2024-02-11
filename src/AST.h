#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "ASTVisitor.h"

#include "Result.h"
#include "Value.h"

enum class Operator
{
	MINUS,
	PLUS,
	TIMES,
	DIVIDED,
	GREATER_THAN,
	LESS_THAN,
	EQUALS,
	GEQ,
	LEQ,
	AND,
	OR
};

static const std::unordered_map<std::string, Operator> op_str_to_enum
{
	{"+", Operator::PLUS},
	{"-", Operator::MINUS},
	{"*", Operator::TIMES},
	{"/", Operator::DIVIDED},
	{">", Operator::GREATER_THAN},
	{"<", Operator::LESS_THAN},
	{"==", Operator::EQUALS},
	{">=", Operator::GEQ},
	{"<=", Operator::LEQ},
	{"&&", Operator::AND},
	{"||", Operator::OR}
};

enum class Type
{
	INT,
	CHAR,
	FLOAT,
	STRING
};

static const std::unordered_map<std::string, Type> type_str_to_enum
{
	{"int", Type::INT},
	{"char", Type::CHAR},
	{"float", Type::FLOAT},
	{"string", Type::STRING}
};

using InterpreterResult = Result<std::shared_ptr<Value>, const char*>;

class ASTNode
{
public:
	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const = 0;
};

class ASTLiteralNode : public ASTNode 
{ 
public: 
	template<typename T>
	ASTLiteralNode(T value)
	{
		m_value = std::make_shared<NumberValue<T>>(value);
	} 

	ASTLiteralNode(const std::string value)
	{
		m_value = std::make_shared<StringValue>(value);
	}

	inline const std::shared_ptr<Value>& get_value() const { return m_value; }
	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const 
	{ 
		return visitor.visit(*this); 
	} 
private: 
	std::shared_ptr<Value> m_value; 
};  

class ASTIdentifierNode : public ASTNode
{
public:
	ASTIdentifierNode(const std::string& name)
		: m_name(name)
	{}

	inline const std::string& get_name() const { return m_name; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::string m_name;
};

class ASTUnaryNode : public ASTNode
{
public:
	ASTUnaryNode(const std::string& op, ASTNode* operand)
		: m_operator(op_str_to_enum.at(op))
		, m_operand(operand)
	{}

	inline Operator get_operator() const { return m_operator; }
	inline const std::unique_ptr<ASTNode>& get_operand() const { return m_operand; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const Operator m_operator;
	const std::unique_ptr<ASTNode> m_operand;
};

class ASTBinaryNode : public ASTNode
{
public:
	ASTBinaryNode(const std::string& op, ASTNode* lhs, ASTNode* rhs)
		: m_operator(op_str_to_enum.at(op))
		, m_lhs(lhs)
		, m_rhs(rhs)
	{}

	inline Operator get_operator() const { return m_operator; }
	inline const std::unique_ptr<ASTNode>& get_lhs() const { return m_lhs; }
	inline const std::unique_ptr<ASTNode>& get_rhs() const { return m_rhs; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const Operator m_operator;
	const std::unique_ptr<ASTNode> m_lhs;
	const std::unique_ptr<ASTNode> m_rhs;
};

class ASTLetNode : public ASTNode
{
public:
	ASTLetNode(const std::string& var_name, ASTNode* expr)
		: m_var_name(var_name)
		, m_expr(expr)
	{}

	inline const std::string& get_var_name() const { return m_var_name; }
	inline const std::unique_ptr<ASTNode>& get_expr() const { return m_expr; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::string m_var_name;
	const std::unique_ptr<ASTNode> m_expr;
};

class ASTAssignmentNode : public ASTNode
{
public:
	ASTAssignmentNode(ASTIdentifierNode* variable, ASTNode* expr)
		: m_variable(variable)
		, m_expr(expr)
	{}

	inline const std::unique_ptr<ASTIdentifierNode>& get_variable() const { return m_variable; }
	inline const std::unique_ptr<ASTNode>& get_expr() const { return m_expr; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::unique_ptr<ASTIdentifierNode> m_variable;
	const std::unique_ptr<ASTNode> m_expr;
};

class ASTIfNode : public ASTNode
{
public:
	ASTIfNode(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt)
		: m_condition(condition)
		, m_then_stmt(then_stmt)
		, m_else_stmt(else_stmt)
	{}

	inline const std::unique_ptr<ASTNode>& get_conditon() const { return m_condition; }
	inline const std::unique_ptr<ASTNode>& get_then_stmt() const { return m_then_stmt; }
	inline const std::unique_ptr<ASTNode>& get_else_stmt() const { return m_else_stmt; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::unique_ptr<ASTNode> m_condition;
	const std::unique_ptr<ASTNode> m_then_stmt;
	const std::unique_ptr<ASTNode> m_else_stmt;
};

class ASTWhileNode : public ASTNode
{
public:
	ASTWhileNode(ASTNode* condition, ASTNode* then_stmt)
		: m_condition(condition)
		, m_then_stmt(then_stmt)
	{}

	inline const std::unique_ptr<ASTNode>& get_conditon() const { return m_condition; }
	inline const std::unique_ptr<ASTNode>& get_then_stmt() const { return m_then_stmt; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::unique_ptr<ASTNode> m_condition;
	const std::unique_ptr<ASTNode> m_then_stmt;
};

class ASTPrintNode : public ASTNode
{
public:
	ASTPrintNode(ASTNode* expr)
		: m_expr(expr)
	{}

	inline const std::unique_ptr<ASTNode>& get_expr() const { return m_expr; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::unique_ptr<ASTNode> m_expr;
};

class ASTCastNode : public ASTNode
{
public:
	ASTCastNode(const std::string& type, ASTNode* expr)
		: m_type(type_str_to_enum.at(type))
		, m_expr(expr)
	{}

	inline Type get_type() const { return m_type; }
	inline const std::unique_ptr<ASTNode>& get_expr() const { return m_expr; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const Type m_type;
	const std::unique_ptr<ASTNode> m_expr;
};

class ASTInputNode : public ASTNode
{
public:
	ASTInputNode()
	{}

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}
};

class ASTBlockNode : public ASTNode
{
public:
	ASTBlockNode(std::vector<std::unique_ptr<ASTNode>> stmts)
		: m_stmts(std::move(stmts))
	{}

	inline const std::vector<std::unique_ptr<ASTNode>>& get_stmts() const { return m_stmts; }

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::string m_var_name;
	const std::vector<std::unique_ptr<ASTNode>> m_stmts;
};