#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include "Result.h"
#include <regex>

enum TokenType
{
	EOF_TOKEN,
	SEMICOLON,
	IDENTIFIER,
	LITERAL,
	OPERATOR,
	LET,
	LPAR,
	RPAR,
	LBRACKET,
	RBRACKET,
	TYPE
};

struct Token
{
	const TokenType type;

	inline Token(TokenType t, std::unordered_map<std::string, std::string> m)
		: type(t)
		, m_meta(m)
	{}

	inline void print() const
	{
		if (m_meta.empty())
		{
			std::cout << type;
		}
		else
		{
			std::cout << type << " [";
			for (const auto& entry : m_meta)
			{
				std::cout << "'" << entry.first << "'" << ":" << " '" << entry.second << "', ";
			}
			std::cout << "]";
		}
	}

	inline const std::string& operator[](const std::string& key) const { return m_meta.at(key); }

	inline int get_int(const std::string& key) const
	{
		return std::stoi(m_meta.at(key).c_str());
	}

	inline float get_float(const std::string& key) const
	{
		return std::stof(m_meta.at(key).c_str());
	}

	inline char get_char(const std::string& key) const
	{
		return m_meta.at(key).c_str()[0];
	}

private:
	const std::unordered_map<std::string, std::string> m_meta;
};

class Lexer
{
public:
	Result<std::vector<Token>, size_t> tokenize(std::string text);

private:
	Token tokenize_text(const std::string& value);
	Token tokenize_number(const std::string& value);

	const std::vector<std::pair<std::string, std::regex>> m_token_patterns
	{
		{ "WHITESPACE", std::regex("^\\s+")},
		{ "NUMBER", std::regex("^[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?")},
		{ "TEXT", std::regex("^[a-zA-Z_]\\w*")},
		{ "CHAR_LITERAL", std::regex("^'(.)'")},
		{ "OPERATOR", std::regex("^[+\\-*\\/]|^:=")},
		{ "SPECIAL", std::regex("^[;()\\[\\]]")},
	};

	const std::unordered_map<std::string, TokenType> m_keywords
	{
		{ "let", LET },
	};

	const std::vector<std::string> m_types
	{
		"int", "float", "char"
	};

	const std::unordered_map<char, TokenType> m_special_chars
	{
		{ ';', SEMICOLON },
		{ '(', LPAR },
		{ ')', RPAR },
		{ '[', LBRACKET },
		{ ']', RBRACKET },
	};

	const std::vector<char> m_operator_constituents
	{
		'+', '-', '*', ':', '/', '='
	};

	const std::vector<std::string> m_operators
	{
		"+", "-", "*", ":=", "/"
	};
};