#include "Interpreter.h"
#include "Value.h"
#include <iostream>

Interpreter::Interpreter()
{
	//Add global scope
	scope_manager.push_scope();
}

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
	if (const auto& variable = scope_manager.get_variable(node.get_name()))
		return { std::make_shared<ReferenceValue>(variable) };
	return "Symbol does not exist error";
}

InterpreterResult Interpreter::visit(const ASTUnaryNode& node)
{
	InterpreterResult operand_res = node.get_operand()->accept(*this);
	if (operand_res.is_error()) 
		return operand_res;

	Value* operand = (*operand_res).get();

	UnaryOperationVisitor visitor(node.get_operator());
	return operand->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTIfNode& node)
{
	InterpreterResult condition_res = node.get_conditon()->accept(*this);
	if (condition_res.is_error())
		return condition_res;

	if ((*condition_res)->is_truthy())
	{
		return node.get_then_stmt()->accept(*this);
	}
	else
	{
		ASTNode* else_stmt = node.get_else_stmt().get();
		if(else_stmt)
			return else_stmt->accept(*this);
	}

	return {};
}

InterpreterResult Interpreter::visit(const ASTWhileNode& node)
{
	InterpreterResult condition_res = node.get_conditon()->accept(*this);
	if (condition_res.is_error())
		return condition_res;

	std::shared_ptr<Value> cond_expr = *condition_res;
	while (cond_expr->is_truthy())
	{
		InterpreterResult stmt_res = node.get_then_stmt()->accept(*this);
		if (stmt_res.is_error())
			return stmt_res;

		InterpreterResult condition_res = node.get_conditon()->accept(*this);
		if (condition_res.is_error())
			return condition_res;

		cond_expr = *condition_res;
	}

	return {};
}

InterpreterResult Interpreter::visit(const ASTPrintNode& node)
{
	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;

	Value* value = (*expr_res).get();

	PrintVisitor visitor;
	return value->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTCastNode& node)
{
	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;
	
	Value* expr_value = (*expr_res).get();
	CastVisitor visitor(node.get_type());
	return expr_value->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTInputNode&)
{
	std::string input;
	std::cout << "Input: ";
	std::getline(std::cin, input);

	return { std::make_shared<StringValue>(input) };
}

InterpreterResult Interpreter::visit(const ASTBinaryNode& node)
{
	InterpreterResult lhs_res = node.get_lhs()->accept(*this);
	if (lhs_res.is_error()) return lhs_res.get_error();

	InterpreterResult rhs_res = node.get_rhs()->accept(*this);
	if (rhs_res.is_error()) return rhs_res.get_error();

	Value* lhs = (*lhs_res).get();
	Value* rhs = (*rhs_res).get();

	BinaryOperationVisitor visitor(node.get_operator(), rhs);
	return lhs->accept(visitor);
}

InterpreterResult Interpreter::visit(const ASTBlockNode& node)
{
	scope_manager.push_scope();
	for (const auto& stmt : node.get_stmts())
	{
		try
		{
			InterpreterResult stmt_res = stmt->accept(*this);
			if (stmt_res.is_error())
				return stmt_res;
		}
		catch (const std::shared_ptr<Value>& returned)
		{
			//If we return we have to pop this blocks scope before continuing with the return
			scope_manager.pop_scope();
			throw returned;
		}
	}
	scope_manager.pop_scope();

	return {};
}

InterpreterResult Interpreter::deref_expr(ASTNode* expr)
{
	InterpreterResult expr_res = expr->accept(*this);
	if (expr_res.is_error())
		return expr_res;

	std::shared_ptr<Value> deref = *expr_res;
	if (const auto& ref = dynamic_cast<ReferenceValue*>(deref.get()))
		deref = ref->get_variable_value();

	return deref;
}

InterpreterResult Interpreter::visit(const ASTLetNode& node)
{
	//If the variable is set to a reference we want to dereference it 
	InterpreterResult deref_res = deref_expr(node.get_expr().get());
	if (deref_res.is_error())
		return deref_res;
	scope_manager.add_variable(node.get_var_name(), *deref_res);
}

InterpreterResult Interpreter::visit(const ASTAssignmentNode& node)
{
	InterpreterResult literal_res = visit(*node.get_variable().get());
	if (literal_res.is_error())
		return literal_res;

	InterpreterResult expr_res = node.get_expr()->accept(*this);
	if (expr_res.is_error())
		return expr_res;

	//If the expression returned a reference we want to dereference it 
	std::shared_ptr<Value> var_value = *expr_res;
	if (const auto& ref = dynamic_cast<ReferenceValue*>(var_value.get()))
		var_value = ref->get_variable_value();

	//Literal node visit always returns reference
	static_cast<ReferenceValue*>((*literal_res).get())->set_variable_value(var_value);

	return {};
}

InterpreterResult Interpreter::visit(const ASTFunctionNode& node)
{
	function_table[node.get_name()] = { node.get_block().get(), &node.get_args()};
	return {};
}

InterpreterResult Interpreter::visit(const ASTCallNode& node)
{
	if(function_table.find(node.get_name()) != function_table.end()) 
	{
		++runtime_data.n_function_calls;

		Function func = function_table.at(node.get_name());

		if (func.arg_names->size() != node.get_args().size())
			return "Incorrect number of arguments in function call";

		//Place arguments in their own scope
		scope_manager.push_scope();

		// We have to evaluate all arguments before initializing them
		// Otherwise values depening on each other such as in fn foo(x, y) {...};
		// foo(x+1, x) would become -> foo(x+1, x+1+1) since x will have the new value 
		// x+1 before evauating the second argument
		std::vector<std::shared_ptr<Value>> args_values;
		for(size_t i = 0; i < func.arg_names->size(); ++i) 
		{
			//Similar to let, we don't want references here
			InterpreterResult deref_res = deref_expr(node.get_args().at(i).get());
			if (deref_res.is_error())
				return deref_res;
			args_values.push_back(*deref_res);
		}

		for (size_t i = 0; i < func.arg_names->size(); ++i)
		{
			scope_manager.add_variable(func.arg_names->at(i), args_values.at(i));
		}

		std::shared_ptr<Value> return_val;
		try
		{
			InterpreterResult res = visit(*func.body);
			if (res.is_error())
				return res;
			return_val = void_val;
		}
		catch (const std::shared_ptr<Value>& returned)
		{
			return_val = returned;

			//Don't want to return references
			if (const auto& ref = dynamic_cast<ReferenceValue*>(return_val.get()))
				return_val = ref->get_variable_value();
		}

		--runtime_data.n_function_calls;
		scope_manager.pop_scope();

		return return_val;
	}

	return "Function does not exist";
}

InterpreterResult Interpreter::visit(const ASTReturnNode& node)
{
	if (runtime_data.n_function_calls == 0)
		return "Cannot return outside function";

	if(node.get_expr()) 
	{
		InterpreterResult expr_res = node.get_expr()->accept(*this);
		if (expr_res.is_error())
			return expr_res;

		throw *expr_res;
	}

	throw static_cast<std::shared_ptr<Value>>(void_val);
	
	return {}; //Won't reach this line but it's fine to keep it for consistency
}

/*
 * UNARY
*/

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const ReferenceValue& value)
{
	return value.get_variable_value()->accept(*this);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<int>& value)
{
	return number_operation(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<float>& value)
{
	return number_operation(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const NumberValue<char>& value)
{
	return number_operation(value);
}

InterpreterResult Interpreter::UnaryOperationVisitor::visit(const StringValue&)
{
	return "Cannot perform unary operation on string";
}

/*
 * PRINT
*/

InterpreterResult Interpreter::PrintVisitor::visit(const ReferenceValue& value)
{
	return value.get_variable_value()->accept(*this);
}

InterpreterResult Interpreter::PrintVisitor::visit(const NumberValue<int>& value)
{
	return print(value.value);
}

InterpreterResult Interpreter::PrintVisitor::visit(const NumberValue<float>& value)
{
	return print(value.value);
}

InterpreterResult Interpreter::PrintVisitor::visit(const NumberValue<char>& value)
{
	return print(value.value);
}

InterpreterResult Interpreter::PrintVisitor::visit(const StringValue& value)
{
	return print(value.text);
}

/*
 * CAST
*/

InterpreterResult Interpreter::CastVisitor::visit(const ReferenceValue& value)
{
	return value.get_variable_value()->accept(*this);
}

InterpreterResult Interpreter::CastVisitor::visit(const NumberValue<int>& value)
{
	if (type == Type::INT || type == Type::CHAR || type == Type::FLOAT)
		return num_to_num(value);
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(std::to_string(value.value)) };

	return "Cannot cast int to x"; //TODO Create better runtime errors (not just const char*) to replace x
}

InterpreterResult Interpreter::CastVisitor::visit(const NumberValue<float>& value)
{
	if (type == Type::INT || type == Type::FLOAT)
		return num_to_num(value);
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(std::to_string(value.value)) };

	return "Cannot cast float to x";
}

InterpreterResult Interpreter::CastVisitor::visit(const NumberValue<char>& value)
{
	if (type == Type::INT || type == Type::CHAR || type == Type::FLOAT)
		return num_to_num(value);
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(std::string(1, value.value)) };

	return "Cannot cast char to x";
}

InterpreterResult Interpreter::CastVisitor::visit(const StringValue& value)
{
	if (type == Type::INT)
		return str_to_num<int>(value.text, [](const std::string& str) { return std::stoi(str); });
	if (type == Type::FLOAT)
		return str_to_num<float>(value.text, [](const std::string& str) { return std::stof(str); });
	if (type == Type::STRING)
		return { std::make_shared<StringValue>(value.text) };

	return "Cannot convert string to x";
}

/*
* BINARY OPERATION
*/

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const ReferenceValue& value)
{
	return value.get_variable_value()->accept(*this);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const NumberValue<int>& value)
{
	return number_operation(value, Type::INT);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const NumberValue<float>& value)
{
	return number_operation(value, Type::FLOAT);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const NumberValue<char>& value)
{
	return number_operation(value, Type::CHAR);
}

InterpreterResult Interpreter::BinaryOperationVisitor::visit(const StringValue& value)
{
	if (auto* ref = dynamic_cast<ReferenceValue*>(other))
		other = ref->get_variable_value().get();

	if (auto* other_val = dynamic_cast<StringValue*>(other))
	{
		switch (op)
		{
		case Operator::PLUS:
			return { std::make_shared<StringValue>(value.text + other_val->text) };
		case Operator::EQUALS:
			return { std::make_shared<NumberValue<int>>(value.text == other_val->text) };
		}

		return "Binary operator is not supported on string";
	}

	return "Types are not compatible in binary operation";
}