#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "ASTVisitor.h"

#include "Result.h"
#include <variant>

#include "Array.h"

using Value = std::variant<int, float, char, Array>;
using InterpreterResult = Result<Value, const char*>;

class ASTNode
{
public:
	inline virtual void print() const = 0;
	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const = 0;
};

class ASTLiteralNode : public ASTNode 
{ 
public: 
	ASTLiteralNode(const std::variant<int, float, char>& value)
		: m_value(value) 
	{} 
	inline const std::variant<int, float, char>& get_value() const { return m_value; } 
	inline virtual void print() const override 
	{ 
		//TODO: Print correctly std::cout << "{Literal: " << m_value << "}"; 
		//Printing is only used for debugging anyway so for now it's fine

		std::cout << "{Literal}";
	} 
	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const 
	{ 
		return visitor.visit(*this); 
	} 
private: 
	std::variant<int, float, char> m_value; 
};  

class ASTArrayInitNode : public ASTNode
{
public:
	ASTArrayInitNode(ArrayType data_type, ASTNode* size_expr)
		: m_data_type(data_type)
		, m_size_expr(size_expr)
	{}

	inline const std::unique_ptr<ASTNode>& get_size_expr() const { return m_size_expr; }
	inline ArrayType get_type() const { return m_data_type; }

	inline virtual void print() const override
	{
		std::cout << "{Array: " << (int)m_data_type << "*";
		m_size_expr->print();
		std::cout << "}";
	}

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const ArrayType m_data_type;
	const std::unique_ptr<ASTNode> m_size_expr;
};

class ASTIdentifierNode : public ASTNode
{
public:
	ASTIdentifierNode(const std::string& name)
		: m_name(name)
	{}

	inline const std::string& get_name() const { return m_name; }

	inline virtual void print() const override
	{
		std::cout << "{Identifier: " << m_name << "}";
	}

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
		: m_operator(op)
		, m_operand(operand)
	{}

	inline const std::string& get_operator() const { return m_operator; }
	inline const std::unique_ptr<ASTNode>& get_operand() const { return m_operand; }

	inline virtual void print() const override
	{
		std::cout << "{Unary: ";
		std::cout << "operator: " << m_operator << ", operand: ";
		m_operand->print();
		std::cout << "}";
	}

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::string m_operator;
	const std::unique_ptr<ASTNode> m_operand;
};

class ASTBinaryNode : public ASTNode
{
public:
	ASTBinaryNode(const std::string& op, ASTNode* lhs, ASTNode* rhs)
		: m_operator(op)
		, m_lhs(lhs)
		, m_rhs(rhs)
	{}

	inline const std::string& get_operator() const { return m_operator; }
	inline const std::unique_ptr<ASTNode>& get_lhs() const { return m_lhs; }
	inline const std::unique_ptr<ASTNode>& get_rhs() const { return m_rhs; }

	inline virtual void print() const override
	{
		std::cout << "{Binary: ";
		std::cout << "operator: " << m_operator << ", operands: ";
		m_lhs->print();
		std::cout << ", ";
		m_rhs->print();
		std::cout << "}";
	}

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::string m_operator;
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

	inline virtual void print() const override
	{
		std::cout << "{Let " << m_var_name << " be ";
		m_expr->print();
		std::cout << "}";
	}

	inline virtual InterpreterResult accept(ASTVisitor<InterpreterResult>& visitor) const
	{
		return visitor.visit(*this);
	}

private:
	const std::string m_var_name;
	const std::unique_ptr<ASTNode> m_expr;
};