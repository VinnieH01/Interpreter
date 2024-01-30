#pragma once

#include "Parser.h"

Parser::Parser()
	: m_index(0)
	, m_saved_index(0)
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

void Parser::save_index()
{
	m_saved_index = m_index;
}

void Parser::load_index()
{
	m_index = m_saved_index;
	m_current_token = &(*m_tokens)[m_index];
}

using ParseRes = Result<ASTNode*, const char*>;

Result<std::vector<std::unique_ptr<ASTNode>>, std::vector<const char*>> Parser::parse(const std::vector<Token>& tokens)
{
	m_tokens = &tokens;
	m_current_token = &(*m_tokens)[0];

	m_index = 0;
	m_saved_index = 0;

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

		if (m_current_token->is_not(TokenType::SPECIAL_CHAR, ";"))
		{
			errors.push_back("Expected ';' after statement");
		}
		else advance();
	}

	if (!errors.empty()) 
		return errors;
	return stmts;
}

ParseRes Parser::parse_stmt()
{
	if (m_current_token->is(TokenType::KEYWORD, "let"))
	{
		return parse_let();
	}
	else
	{
		return parse_expr();
	}
}

ParseRes Parser::parse_let()
{
	advance();
	if (m_current_token->type != TokenType::IDENTIFIER)
		return "Expected identifier after 'let'";
	std::string name = m_current_token->get_string("name");

	advance();
	if (m_current_token->is_not(TokenType::OPERATOR, ":="))
		return "Expected assignement operator ':=' after identifier";

	advance();

	ParseRes expr_res = parse_expr();
	if (expr_res.is_error())
		return expr_res;

	return new ASTLetNode(name, *expr_res);
}

ParseRes Parser::parse_expr()
{
	return parse_sum();
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
		ParseRes rhs_res = parse_product();
		if (rhs_res.is_error())
			return rhs_res;

		lhs = std::unique_ptr<ASTNode>(new ASTBinaryNode(op, lhs.release(), *rhs_res));
	}

	return lhs.release();
}

ParseRes Parser::parse_unary()
{
	const Token& tok = *m_current_token;

	if (m_current_token->type == TokenType::OPERATOR && (
		m_current_token->get_string("value") == "+" ||
		m_current_token->get_string("value") == "-"))
	{
		advance();

		ParseRes unary_res = parse_unary();
		if (unary_res.is_error())
			return unary_res;

		return new ASTUnaryNode(tok.get_string("value"), *unary_res);
	}

	return parse_primary();
}

ParseRes Parser::parse_primary()
{
	const Token& tok = *m_current_token;

	if (tok.type == TokenType::LITERAL)
	{
		if (tok.get_string("data_type") == "integer")
		{
			advance();
			return new ASTLiteralNode(tok.get_int("value"));
		}
		if (tok.get_string("data_type") == "float")
		{
			advance();
			return new ASTLiteralNode(tok.get_float("value"));
		}
		if (tok.get_string("data_type") == "char")
		{
			advance();
			return new ASTLiteralNode(tok.get_char("value"));
		}
		return "Only number literals supported";
	}
	else if (tok.type == TokenType::IDENTIFIER)
	{
		advance();
		return new ASTIdentifierNode(tok.get_string("name"));
	}
	else if (tok.type == TokenType::TYPE)
	{
		advance();
		if (m_current_token->is_not(TokenType::SPECIAL_CHAR, "["))
			return "Expected '[' after type in array initialisation";
		advance();

		ParseRes expr_res = parse_expr();
		if (expr_res.is_error())
			return expr_res;
		std::unique_ptr<ASTNode> expr(*expr_res);

		if (m_current_token->is_not(TokenType::SPECIAL_CHAR, "]"))
			return "Expected ']' after array initialisation";
		advance();

		const std::string& dtype = tok.get_string("value");
		return new ASTArrayInitNode(dtype == "int" ? ArrayType::INT : 
			                        dtype == "float" ? ArrayType::FLOAT :
									ArrayType::CHAR, expr.release());
	}
	else if (m_current_token->is(TokenType::SPECIAL_CHAR, "("))
	{
		advance();

		ParseRes expr_res = parse_expr();
		if (expr_res.is_error())
			return expr_res;

		std::unique_ptr<ASTNode> expr(*expr_res);

		if (m_current_token->is_not(TokenType::SPECIAL_CHAR, ")"))
			return "Expected ')' in parenthesised expression";
		advance();

		return expr.release();
	}

	return "Invalid Expression";
}