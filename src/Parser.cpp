#pragma once

#include "Parser.h"

Parser::Parser()
	: m_index(0)
	, m_current_token(nullptr)
	, m_tokens(nullptr)
{
}

void Parser::advance()
{
	m_index++;
	if (m_index < m_tokens->size())
	{
		m_current_token = &(*m_tokens)[m_index];
	}
}

//Get the previous token
const Token& Parser::prev(size_t n)
{
	if (m_index - n >= 0)
	{
		return (*m_tokens)[m_index - n];
	}
	return Token{ TokenType::EOF_TOKEN, {} };
}

/*
* Function used to test grammar patterns. Calls a series of functions in order and if they all
* return true then the output is true.
* If one of the functions return false then the parser goes back to the index it was before
* calling test().
*/
bool Parser::test(const std::initializer_list<std::function<bool()>>& test_functions)
{
	int saved_idx = m_index;

	bool result = true;

	for (auto& func : test_functions) 
	{
		result = result && func();
		if (!result) 
		{
			m_index = saved_idx;
			m_current_token = &(*m_tokens)[m_index];
			return false; 
		}
	}

	return true;
}

// Advance if the current token has TokenType type and is one of the listed values.
bool Parser::consume(TokenType type, const std::initializer_list<std::string>& values)
{
	for (const auto& v : values)
	{
		if (m_current_token->is(type, v))
		{
			advance();
			return true;
		}
	}

	return false;
}

//If the token was consumed then it will be pointed to by tok.
bool Parser::consume(TokenType type, const std::initializer_list<std::string>& values, const Token*& tok)
{
	bool result = consume(type, values);
	if (result) tok = &prev();
	return result;
}

bool Parser::consume(TokenType type)
{
	bool result = m_current_token->type == type;
	if (result) advance();
	return result;
}

bool Parser::consume(TokenType type, const Token*& tok)
{
	bool result = consume(type);
	if (result) tok = &prev();
	return result;
}

/*
* Function used together with test() used to try to parse statements and expressions.
* Calls parse_fn and if it succeeds "result" gains ownership of a pointer to the resulting ASTNode.
* As with all parsing functions parse_fn should heap allocate its resulting ASTNode
*/
bool Parser::test_parse(const std::function<ParseRes()>& parse_fn, std::unique_ptr<ASTNode>& result)
{
	ParseRes res = parse_fn();
	if (res.is_error())
		return false;
	result.reset(*res);
	return true;
}

Result<std::vector<std::unique_ptr<ASTNode>>, std::vector<const char*>> Parser::parse(const std::vector<Token>& tokens)
{
	m_tokens = &tokens;
	m_current_token = &(*m_tokens)[0];

	m_index = 0;

	std::vector<const char*> errors;
	std::vector<std::unique_ptr<ASTNode>> stmts;

	while (m_current_token->type != TokenType::EOF_TOKEN)
	{
		ParseRes res = parse_stmt();
		if (res.is_error())
		{
			errors.push_back(res.get_error());

			//If it's an error we will move to the next statement
			//this is to ensure correct parsing and error reporting of future statements
			while (m_current_token->is_not(TokenType::SPECIAL_CHAR, ";") && m_current_token->type != TokenType::EOF_TOKEN)
				advance();
		}
		else
			stmts.push_back(std::unique_ptr<ASTNode>(*res));

		if (!consume(TokenType::SPECIAL_CHAR, {";"} ))
			errors.push_back("Expected ';' after statement");
	}

	if (!errors.empty()) 
		return errors;
	return stmts;
}

using ParseRes = Result<ASTNode*, const char*>;

ParseRes Parser::parse_stmt()
{
	//"{" program "}"
	if (consume(TokenType::SPECIAL_CHAR, {"{"}))
	{
		std::vector<std::unique_ptr<ASTNode>> stmts;
		while (!consume(TokenType::SPECIAL_CHAR, { "}" }))
		{
			ParseRes res = parse_stmt();
			if (res.is_error())
				return "error in block";
			else
				stmts.push_back(std::unique_ptr<ASTNode>(*res));

			if (!consume(TokenType::SPECIAL_CHAR, { ";" }))
				return "Expected ';' after statement";
		}
		return new ASTBlockNode(std::move(stmts));
	}

	//"print" expr
	std::unique_ptr<ASTNode> print_expr;
	if (test({
		[this]() { return consume(TokenType::KEYWORD, {"print"}); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), print_expr); }
		}))
	{
		return new ASTPrintNode(print_expr.release());
	}

	//"let" IDENTIFIER ":=" expr
	std::unique_ptr<ASTNode> let_expr;
	const Token* identifier = nullptr;
	if (test({
		[this]() { return consume(TokenType::KEYWORD, {"let"}); },
		[&]() { return consume(TokenType::IDENTIFIER, identifier); },
		[this]() { return consume(TokenType::OPERATOR, {":="}); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), let_expr); }
	}))
	{
		return new ASTLetNode(identifier->get_string("name"), let_expr.release());
	}

	//"if" "(" expr ")" stmt
	std::unique_ptr<ASTNode> conditional_expr;
	std::unique_ptr<ASTNode> body_stmt;
	if (test({
		[this]() { return consume(TokenType::KEYWORD, {"if"}); },
		[this]() { return consume(TokenType::SPECIAL_CHAR, {"("}); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), conditional_expr); },
		[this]() { return consume(TokenType::SPECIAL_CHAR, {")"}); },
		[&]() { return test_parse(std::bind(&Parser::parse_stmt, this), body_stmt); }
		}))
	{
		return new ASTIfNode(conditional_expr.release(), body_stmt.release());
	}

	//expr
	return parse_expr();
}

ParseRes Parser::parse_expr()
{
	//logic
	return parse_logic();
}

ParseRes Parser::parse_logic()
{
	//comparison ("&&"|"||") logic
	//comparison
	return parse_binary_expr(std::bind(&Parser::parse_comparison, this), std::bind(&Parser::parse_logic, this), { "&&", "||" });
}

ParseRes Parser::parse_comparison()
{
	//sum (">"|"<"|"=="|">="|"<=") comparison
	//sum
	return parse_binary_expr(std::bind(&Parser::parse_sum, this), std::bind(&Parser::parse_comparison, this), { ">", "<", "==", ">=", "<=" });
}

ParseRes Parser::parse_sum()
{
	//product ("+"|"-") sum
	//product
	return parse_binary_expr(std::bind(&Parser::parse_product, this), std::bind(&Parser::parse_sum, this), {"+", "-"});
}

ParseRes Parser::parse_product()
{
	//unary ("*"|"/") product
	//unary
	return parse_binary_expr(std::bind(&Parser::parse_unary, this), std::bind(&Parser::parse_product, this), { "*", "/" });
}

/*
* Checks for any binary expression pattern where x is the operand and y is the adjacent 
* lower-precedence expression (this should be the calling function).
* Effectively looking for grammar rule y -> x (operator x)*
*/
ParseRes Parser::parse_binary_expr(const std::function<ParseRes()>& parse_x, const std::function<ParseRes()>& parse_y, const std::initializer_list<std::string>& operators)
{
	//x operator y
	std::unique_ptr<ASTNode> x_expr;
	std::unique_ptr<ASTNode> y_expr;
	const Token* op = nullptr;
	if (test({
		[&]() { return test_parse(parse_x, x_expr); },
		[&]() { return consume(TokenType::OPERATOR, operators, op); },
		[&]() { return test_parse(parse_y, y_expr); }
		}))
	{
		return new ASTBinaryNode(op->get_string("value"), x_expr.release(), y_expr.release());
	}

	//x
	return parse_x();
}

ParseRes Parser::parse_unary()
{
	// "-" unary
	std::unique_ptr<ASTNode> unary;
	if (test({
		[this]() { return consume(TokenType::OPERATOR, {"-"}); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), unary); },
	}))
	{
		return new ASTUnaryNode("-", unary.release());
	}

	//primary
	return parse_primary();
}

ParseRes Parser::parse_primary()
{
	// LITERAL
	if (consume(TokenType::LITERAL))
	{
		if (prev().get_string("data_type") == "integer")
			return new ASTLiteralNode(prev().get_int("value"));

		if (prev().get_string("data_type") == "float")
			return new ASTLiteralNode(prev().get_float("value"));

		if (prev().get_string("data_type") == "char")
			return new ASTLiteralNode(prev().get_char("value"));

		if (prev().get_string("data_type") == "string")
			return new ASTLiteralNode(prev().get_string("value"));
	}

	// IDENTIFIER
	if (consume(TokenType::IDENTIFIER))
		return new ASTIdentifierNode(prev().get_string("name"));
	
	// "(" expr ")"
	std::unique_ptr<ASTNode> expr;
	if(test({
		[this]() { return consume(TokenType::SPECIAL_CHAR, {"("}); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), expr); },
		[this]() { return consume(TokenType::SPECIAL_CHAR, {")"}); }
	})) 
	{
		return expr.release();
	}

	return "Invalid Expression";
}