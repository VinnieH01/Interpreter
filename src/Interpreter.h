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
		inline InterpreterResult number_operation(const NumberValue<T>& value)
		{
			if (op == Operator::MINUS)
				return { std::make_shared<NumberValue<T>>(value.value * -1) };
		}

		Operator op;
	};

	struct BinaryOperationVisitor : ValueVisitor
	{
		BinaryOperationVisitor(Operator op, Value* other)
			: op(op) 
			, other(other)
		{};

		InterpreterResult visit(const NumberValue<int>&) override;
		InterpreterResult visit(const NumberValue<float>&) override;
		InterpreterResult visit(const NumberValue<char>&) override;
		InterpreterResult visit(const StringValue&) override;
	private:
		template<typename T>
		inline InterpreterResult number_operation(const NumberValue<T>& lhs, Type type)
		{
			//TODO: "int op float" casts the float to an int so that things like this happen
			/*
			* 0.9 && 1 -> 1
			* 1 && 0.9 -> 0
			*/
			//Could solve this by giving float a higher "casting precedence" or something like that
			//But for now this is an okay and simple solution

			//Implicit cast to compatible number type
			CastVisitor visitor(type);
			InterpreterResult rhs_res = other->accept(visitor);
			if (rhs_res.is_error())
				return "Types are not compatible in binary operation";

			if (auto* rhs = dynamic_cast<NumberValue<T>*>((*rhs_res).get()))
			{
				switch (op)
				{
				case Operator::PLUS:
					return { std::make_shared<NumberValue<T>>(lhs.value + rhs->value) };
				case Operator::MINUS:
					return { std::make_shared<NumberValue<T>>(lhs.value - rhs->value) };
				case Operator::TIMES:
					return { std::make_shared<NumberValue<T>>(lhs.value * rhs->value) };
				case Operator::DIVIDED:
					return { std::make_shared<NumberValue<T>>(lhs.value / rhs->value) };
				case Operator::EQUALS:
					return { std::make_shared<NumberValue<int>>(lhs.value == rhs->value) };  //TODO: For now int takes the place of bool
				case Operator::LEQ:
					return { std::make_shared<NumberValue<int>>(lhs.value <= rhs->value) };
				case Operator::GEQ:
					return { std::make_shared<NumberValue<int>>(lhs.value >= rhs->value) };
				case Operator::LESS_THAN:
					return { std::make_shared<NumberValue<int>>(lhs.value < rhs->value) };
				case Operator::GREATER_THAN:
					return { std::make_shared<NumberValue<int>>(lhs.value > rhs->value) };
				case Operator::AND:
					return { std::make_shared<NumberValue<int>>(lhs.is_truthy() && rhs->is_truthy()) };
				case Operator::OR:
					return { std::make_shared<NumberValue<int>>(lhs.is_truthy() || rhs->is_truthy()) };
				}

				return "Binary operator is not supported on type";
			}
			_ASSERT(false); //We should never reach this point as the cast should fail and return an error
			return "Types are not compatible in binary operation";
		}
		Operator op;
		Value* other;
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