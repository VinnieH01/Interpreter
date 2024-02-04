#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "Lexer.h"
#include "Error.h"

Result<std::vector<Token>> Lexer::tokenize(std::string text) 
{
	std::vector<Token> tokens;
	m_position = 0;
	while (text.size() > 0)
	{
		std::smatch match;
		for (const auto& pattern : m_token_patterns)
		{
			if (std::regex_search(text, match, pattern.second, std::regex_constants::match_continuous)) //Only match at start of string
			{
				const std::string& value = match[0];
				std::cout << match.position();
				if (pattern.first != WHITESPACE && pattern.first != COMMENT)
				{
					switch (pattern.first)
					{
					case NUMBER: tokens.push_back(tokenize_number(value));
						break;
					case TEXT: tokens.push_back(tokenize_text(value));
						break;
					case CHAR_LITERAL: tokens.push_back(Token(TokenType::LITERAL, { { "data_type", "char" }, { "value", match[1]} }, m_position));
						break;
					case STRING_LITERAL: tokens.push_back(Token(TokenType::LITERAL, { { "data_type", "string" }, { "value", match[1]}}, m_position));
						break;
					case OPERATOR: tokens.push_back(Token(TokenType::OPERATOR, { { "value", value } }, m_position));
						break;
					case SPECIAL: tokens.push_back(Token(TokenType::SPECIAL_CHAR, { { "value", value } }, m_position));
						break;
					}
				}
				text = text.substr(value.size());
				m_position += value.size();
				break;
			}
		}
		if (match.empty())
			return Error("Lexer error", m_position);
	}

	tokens.push_back(Token(TokenType::EOF_TOKEN, {}, m_position));
	return tokens;
}

Token Lexer::tokenize_text(const std::string& value)
{
	if (std::find(m_keywords.begin(), m_keywords.end(), value) != m_keywords.end())
	{
		return Token(TokenType::KEYWORD, { { "value", value } }, m_position);
	}
	if (std::find(m_types.begin(), m_types.end(), value) != m_types.end())
	{
		return Token(TokenType::TYPE, { { "value", value } }, m_position);
	}
	return Token(TokenType::IDENTIFIER, { { "name", value } }, m_position);
}

Token Lexer::tokenize_number(const std::string& value)
{
	if (value.find('.') != std::string::npos)
	{
		return Token(TokenType::LITERAL, { { "data_type", "float" }, { "value", value } }, m_position);
	}
	return Token(TokenType::LITERAL, { { "data_type", "integer" }, { "value", value } }, m_position);
}