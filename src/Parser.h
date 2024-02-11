#pragma once

#include <functional>

#include "Token.h"
#include "Result.h"
#include "AST.h"

class Parser
{
public:
	Parser();
	Result<std::vector<std::unique_ptr<ASTNode>>, std::vector<Error>> parse(const std::vector<Token>& tokens);
private:
	void advance();
	const Token& prev(size_t n = 1);

	bool test(const std::initializer_list<std::function<bool()>>& test_functions);

	bool consume(TokenType type, const std::initializer_list<std::string>& values);
	bool consume(TokenType type);
	bool consume(TokenType type, const std::initializer_list<std::string>& values, const Token*& tok);
	bool consume(TokenType type, const Token*& tok);
	bool test_parse(const std::function<Result<ASTNode*>()>& parse_fn, std::unique_ptr<ASTNode>& result);

	Result<ASTNode*> parse_stmt();
	Result<ASTNode*> parse_expr();

	Result<ASTNode*> parse_logic();
	Result<ASTNode*> parse_comparison();
	Result<ASTNode*> parse_sum();
	Result<ASTNode*> parse_product();
	Result<ASTNode*> parse_binary_expr(const std::function<Result<ASTNode*>()>& parse_x, 
									   const std::function<Result<ASTNode*>()>& parse_y, 
									   const std::initializer_list<std::string>& operators);

	Result<ASTNode*> parse_unary();
	Result<ASTNode*> parse_primary();

	const std::vector<Token>* m_tokens;
	size_t m_index;
	const Token* m_current_token;
};