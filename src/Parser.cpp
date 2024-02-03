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

const Token& Parser::peek(size_t n)
{
	if (m_index + n < m_tokens->size())
	{
		return (*m_tokens)[m_index + n];
	}
	return Token{ TokenType::EOF_TOKEN, {} };
}

const Token& Parser::prev(size_t n)
{
	if (m_index - n >= 0)
	{
		return (*m_tokens)[m_index - n];
	}
	return Token{ TokenType::EOF_TOKEN, {} };
}

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

bool Parser::consume(TokenType type, const std::string& v)
{
	bool result = m_current_token->is(type, v);
	if (result) advance();
	return result;
}

bool Parser::consume(TokenType type)
{
	bool result = m_current_token->type == type;
	if (result) advance();
	return result;
}

bool Parser::consume(TokenType type, const std::string& v, const Token*& tok)
{
	bool result = consume(type, v);
	if (result) tok = &prev();
	return result;
}

bool Parser::consume(TokenType type, const Token*& tok)
{
	bool result = consume(type);
	if (result) tok = &prev();
	return result;
}

bool Parser::test_parse(const std::function<ParseRes()>& parse_fn, std::unique_ptr<ASTNode>& result)
{
	ParseRes res = parse_fn();
	if (res.is_error())
		return false;
	result.reset(*res);
	return true;
}

bool Parser::test_parse(const std::function<ParseRes()>& parse_fn)
{
	ParseRes res = parse_fn();
	if (res.is_error())
		return false;
	delete *res; //Parse function creates ptr using new so we have to delete in order to avoid memory leak
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

		if (!consume(TokenType::SPECIAL_CHAR, ";"))
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
	if (consume(TokenType::SPECIAL_CHAR, "{")) 
	{
		std::vector<std::unique_ptr<ASTNode>> stmts;
		while (!consume(TokenType::SPECIAL_CHAR, "}")) 
		{
			ParseRes res = parse_stmt();
			if (res.is_error())
				return "error in block";
			else
				stmts.push_back(std::unique_ptr<ASTNode>(*res));

			if(!consume(TokenType::SPECIAL_CHAR, ";"))
				return "Expected ';' after statement";
		}
		return new ASTBlockNode(std::move(stmts));
	}

	//"print" expr
	std::unique_ptr<ASTNode> print_expr;
	if (test({
		[this]() { return consume(TokenType::KEYWORD, "print"); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), print_expr); }
		}))
	{
		return new ASTPrintNode(print_expr.release());
	}

	//"let" IDENTIFIER ":=" expr
	std::unique_ptr<ASTNode> let_expr;
	const Token* identifier = nullptr;
	if (test({
		[this]() { return consume(TokenType::KEYWORD, "let"); },
		[&]() { return consume(TokenType::IDENTIFIER, identifier); },
		[this]() { return consume(TokenType::OPERATOR, ":="); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), let_expr); }
	}))
	{
		return new ASTLetNode(identifier->get_string("name"), let_expr.release());
	}

	//"if" "(" expr ")" stmt
	std::unique_ptr<ASTNode> conditional_expr;
	std::unique_ptr<ASTNode> body_stmt;
	if (test({
		[this]() { return consume(TokenType::KEYWORD, "if"); },
		[this]() { return consume(TokenType::SPECIAL_CHAR, "("); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), conditional_expr); },
		[this]() { return consume(TokenType::SPECIAL_CHAR, ")"); },
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
	return parse_logic();
}

ParseRes Parser::parse_logic()
{
	return parse_binary_expr(std::bind(&Parser::parse_comparison, this), { "&&", "||" });
}

ParseRes Parser::parse_comparison()
{
	return parse_binary_expr(std::bind(&Parser::parse_sum, this), { ">", "<", "==", ">=", "<="});
}

ParseRes Parser::parse_sum()
{
	return parse_binary_expr(std::bind(&Parser::parse_product, this), {"+", "-"});
}

ParseRes Parser::parse_product()
{
	return parse_binary_expr(std::bind(&Parser::parse_unary, this), { "*", "/" });
}

ParseRes Parser::parse_binary_expr(const std::function<ParseRes()>& operand_parse_fn, const std::vector<std::string>& operators)
{
	ParseRes lhs_res = operand_parse_fn();
	if (lhs_res.is_error())
		return lhs_res;

	std::unique_ptr<ASTNode> lhs(*lhs_res);

	while (m_current_token->type == TokenType::OPERATOR && std::find(operators.begin(), operators.end(), m_current_token->get_string("value")) != operators.end())
	{
		const std::string& op = m_current_token->get_string("value");
		advance();
		ParseRes rhs_res = operand_parse_fn();
		if (rhs_res.is_error())
			return rhs_res;

		lhs = std::unique_ptr<ASTNode>(new ASTBinaryNode(op, lhs.release(), *rhs_res));
	}

	return lhs.release();
}

ParseRes Parser::parse_unary()
{
	// "-" unary
	std::unique_ptr<ASTNode> unary;
	if (test({
		[this]() { return consume(TokenType::OPERATOR, "-"); },
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
		[this]() { return consume(TokenType::SPECIAL_CHAR, "("); },
		[&]() { return test_parse(std::bind(&Parser::parse_expr, this), expr); },
		[this]() { return consume(TokenType::SPECIAL_CHAR, ")"); }
	})) 
	{
		return expr.release();
	}

	return "Invalid Expression";
}