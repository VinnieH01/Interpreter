#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <regex>

#include "Result.h"
#include "Token.h"

class Lexer
{
public:
	Result<std::vector<Token>> tokenize(std::string text);

private:
	Token tokenize_text(const std::string& value);
	Token tokenize_number(const std::string& value);

	enum 
	{
		WHITESPACE,
		COMMENT,
		NUMBER,
		TEXT,
		CHAR_LITERAL,
		STRING_LITERAL,
		OPERATOR,
		SPECIAL
	};

	const std::vector<std::pair<int, std::regex>> m_token_patterns
	{
		{ WHITESPACE, std::regex("\\s+")},
		{ COMMENT, std::regex("(?:\\/\\/.*\\n?|\\/\\*(?:.|\\n)*?\\*\\/)")},
		{ NUMBER, std::regex("[0-9]*\\.?[0-9]+")},
		{ TEXT, std::regex("[a-zA-Z_]\\w*")},
		{ CHAR_LITERAL, std::regex("'(.)'")},
		{ STRING_LITERAL, std::regex("\"(.+?)\"")},
		{ OPERATOR, std::regex("(?::=|&&|\\|\\||>=|<=|==|[+\\-*\\/<>])")},
		{ SPECIAL, std::regex("[;()\\[\\]{}]")},
	};

	const std::vector<std::string> m_keywords
	{
		"let", "if", "else", "while", "print", "input"
	};

	const std::vector<std::string> m_types
	{
		"int", "float", "char", "string"
	};

	size_t m_position = 0;
};