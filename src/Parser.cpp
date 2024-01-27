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
	return Token{ EOF_TOKEN, {} };
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

	while (m_current_token->type != EOF_TOKEN)
	{
		ParseRes res = parse_stmt();
		if (res.is_error())
		{
			errors.push_back(res.get_error());

			//If it's an error we will move to the next statement
			//this is to ensure correct parsing and error reporting of future statements
			while (m_current_token->type != SEMICOLON && m_current_token->type != EOF_TOKEN)
				advance();
		}
		else
			stmts.push_back(std::unique_ptr<ASTNode>(*res));

		if (m_current_token->type != SEMICOLON)
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
	if (m_current_token->type == LET)
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
	if (m_current_token->type != IDENTIFIER)
		return "Expected identifier after 'let'";
	std::string name = (*m_current_token)["name"];

	advance();
	if (m_current_token->type != OPERATOR || (*m_current_token)["operator"] != ":=")
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

	while (m_current_token->type == OPERATOR && std::find(operators.begin(), operators.end(), (*m_current_token)["operator"]) != operators.end())
	{
		const std::string& op = (*m_current_token)["operator"];
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

	if (m_current_token->type == OPERATOR && (
		(*m_current_token)["operator"] == "+" ||
		(*m_current_token)["operator"] == "-"))
	{
		advance();

		ParseRes unary_res = parse_unary();
		if (unary_res.is_error())
			return unary_res;

		return new ASTUnaryNode(tok["operator"], *unary_res);
	}

	return parse_primary();
}

ParseRes Parser::parse_primary()
{
	const Token& tok = *m_current_token;

	if (tok.type == LITERAL)
	{
		if (tok["data_type"] == "integer")
		{
			advance();
			return new ASTLiteralNode(tok.get_int("value"));
		}
		return "Only integer literals supported";
	}
	else if (tok.type == IDENTIFIER)
	{
		advance();
		return new ASTIdentifierNode(tok["name"]);
	}
	else if (tok.type == LPAR)
	{
		advance();

		ParseRes expr_res = parse_expr();
		if (expr_res.is_error())
			return expr_res;

		ASTNode* expr = *expr_res;

		if (m_current_token->type != RPAR)
		{
			delete expr;
			return "Expected ')' in parenthesised expression";
		}
		advance();

		return expr;
	}

	return "Invalid Expression";
}