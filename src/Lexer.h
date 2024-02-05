#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include "Result.h"
#include <regex>
#include "Error.h"

enum class TokenType
{
	EOF_TOKEN,
	SPECIAL_CHAR,
	IDENTIFIER,
	LITERAL,
	OPERATOR,
	KEYWORD,
	TYPE
};

struct Token
{
	const TokenType type;

	inline Token(TokenType t, std::unordered_map<std::string, std::string> m, size_t position)
		: type(t)
		, m_meta(m)
		, m_position(position)
	{}

	inline void print() const
	{
		if (m_meta.empty())
		{
			std::cout << (int)type;
		}
		else
		{
			std::cout << (int)type << " [";
			for (const auto& entry : m_meta)
			{
				std::cout << "'" << entry.first << "'" << ":" << " '" << entry.second << "', ";
			}
			std::cout << "]";
		}
	}

	inline size_t get_position() const { return m_position; }

	inline const std::string& get_string(const std::string& key) const 
	{
		return m_meta.at(key); 
	}

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

	inline bool is(TokenType type, const std::string& value) const
	{
		return this->type == type && m_meta.count("value") && m_meta.at("value") == value;
	}

	//This only exists beacause it's clearer than saying !tok.is(...)
	inline bool is_not(TokenType type, const std::string& value) const
	{
		return !is(type, value);
	}

private:
	const std::unordered_map<std::string, std::string> m_meta;
	const size_t m_position;
};

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
		"let", "if", "else", "print", "input"
	};

	const std::vector<std::string> m_types
	{
		"int", "float", "char", "string"
	};

	size_t m_position = 0;
};