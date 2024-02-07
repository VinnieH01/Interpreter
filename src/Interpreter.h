#pragma once

#include <unordered_map>
#include <string>
#include "ASTVisitor.h"
#include "AST.h"
#include <functional>

using InterpreterResult = Result<std::shared_ptr<Value>, const char*>;

class Interpreter : public ASTVisitor<InterpreterResult>
{
public:
	InterpreterResult interpret(const ASTNode&);

	virtual InterpreterResult visit(const ASTLiteralNode&) override;
	virtual InterpreterResult visit(const ASTIdentifierNode&) override;
	virtual InterpreterResult visit(const ASTUnaryNode&) override;
	virtual InterpreterResult visit(const ASTIfNode&) override;
	virtual InterpreterResult visit(const ASTPrintNode&) override;
	virtual InterpreterResult visit(const ASTCastNode&) override;
	virtual InterpreterResult visit(const ASTInputNode&) override;
	virtual InterpreterResult visit(const ASTBinaryNode&) override;
	virtual InterpreterResult visit(const ASTBlockNode&) override;
	virtual InterpreterResult visit(const ASTLetNode&) override;
private:
	template<typename T, typename T2>
	bool number_op(Value* lhs, Value* rhs, Operator op, std::shared_ptr<Value>& out);

	std::unordered_map<std::string, std::shared_ptr<Value>> m_symbol_table;

public:
	struct UnaryOperationVisitor : ValueVisitor
	{
		UnaryOperationVisitor(Operator op)
			: op(op) {};

		InterpreterResult visit(const NumberValue<int>&) override;
		InterpreterResult visit(const NumberValue<float>&) override;
		InterpreterResult visit(const NumberValue<char>&) override;
		InterpreterResult visit(const StringValue&) override;
	private:
		template<typename T>
		inline InterpreterResult visit_number(const NumberValue<T>& value)
		{
			if (op == Operator::MINUS)
				return { std::make_shared<NumberValue<T>>(value.value * -1) };
		}

		Operator op;
	};

	struct PrintVisitor : ValueVisitor
	{
		InterpreterResult visit(const NumberValue<int>&) override;
		InterpreterResult visit(const NumberValue<float>&) override;
		InterpreterResult visit(const NumberValue<char>&) override;
		InterpreterResult visit(const StringValue&) override;
	private:
		template<typename T>
		inline InterpreterResult print(T printable)
		{
			std::cout << ">> " << printable << std::endl;
			return {};
		}
	};

	struct CastVisitor : ValueVisitor
	{
		CastVisitor(Type type)
			: type(type) {};

		InterpreterResult visit(const NumberValue<int>&) override;
		InterpreterResult visit(const NumberValue<float>&) override;
		InterpreterResult visit(const NumberValue<char>&) override;
		InterpreterResult visit(const StringValue&) override;
	private:
		template<typename T>
		inline InterpreterResult num_to_num(const NumberValue<T>& value)
		{
			if (type == Type::INT)
				return { std::make_shared<NumberValue<int>>(value.value) };
			if (type == Type::FLOAT)
				return { std::make_shared<NumberValue<float>>(value.value) };
			if (type == Type::CHAR)
				return { std::make_shared<NumberValue<char>>(value.value) };

			return "Cannot cast T to x"; //TODO better runtime errors
		}

		template<typename T>
		inline InterpreterResult str_to_num(const std::string& str, std::function<T(const std::string&)> conversion_fn)
		{
			T casted = 0;
			try
			{
				casted = conversion_fn(str);
			}
			catch (const std::exception&)
			{
				return "String is not a valid number";
			}

			return { std::make_shared<NumberValue<T>>(casted) };
		}
		Type type;
	};
};