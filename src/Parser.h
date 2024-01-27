#pragma once

#include "lexer.h"
#include "AST.h"

class Parser
{
	using ParseRes = Result<ASTNode*, const char*>;

public:
	Parser();
	Result<std::vector<std::unique_ptr<ASTNode>>, std::vector<const char*>> parse(const std::vector<Token>& tokens);
private:
	void advance();
	const Token& peek(size_t n = 1);

	void save_index();
	void load_index();

	ParseRes parse_stmt();
	ParseRes parse_let();
	ParseRes parse_expr();
	ParseRes parse_term();
	ParseRes parse_factor();
	ParseRes parse_unary();
	ParseRes parse_primary();

	const std::vector<Token>* m_tokens;
	size_t m_index;
	const Token* m_current_token;
	size_t m_saved_index;
};