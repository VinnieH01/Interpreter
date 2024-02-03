#pragma once

#include "lexer.h"
#include "AST.h"
#include <functional>

class Parser
{
	using ParseRes = Result<ASTNode*, const char*>;

public:
	Parser();
	Result<std::vector<std::unique_ptr<ASTNode>>, std::vector<const char*>> parse(const std::vector<Token>& tokens);
private:
	void advance();
	const Token& prev(size_t n = 1);

	bool test(const std::initializer_list<std::function<bool()>>& test_functions);

	bool consume(TokenType type, const std::initializer_list<std::string>& values);
	bool consume(TokenType type);
	bool consume(TokenType type, const std::initializer_list<std::string>& values, const Token*& tok);
	bool consume(TokenType type, const Token*& tok);
	bool test_parse(const std::function<ParseRes()>& parse_fn, std::unique_ptr<ASTNode>& result);

	ParseRes parse_stmt();
	ParseRes parse_expr();

	ParseRes parse_logic();
	ParseRes parse_comparison();
	ParseRes parse_sum();
	ParseRes parse_product();
	ParseRes parse_binary_expr(const std::function<ParseRes()>& parse_x, const std::function<ParseRes()>& parse_y, const std::initializer_list<std::string>& operators);

	ParseRes parse_unary();
	ParseRes parse_primary();

	const std::vector<Token>* m_tokens;
	size_t m_index;
	const Token* m_current_token;
};