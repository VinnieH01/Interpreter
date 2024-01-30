#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "Lexer.h"

Result<std::vector<Token>, size_t> Lexer::tokenize(std::string text) 
{
	std::vector<Token> tokens;
	size_t cntr = 0;
	while (text.size() > 0)
	{
		std::smatch match;
		for (const auto& pattern : m_token_patterns)
		{
			if (std::regex_search(text, match, pattern.second))
			{
				const std::string& value = match[0];
				if (pattern.first != "WHITESPACE")
				{
					if (pattern.first == "NUMBER") tokens.push_back(tokenize_number(value));
					else if (pattern.first == "TEXT") tokens.push_back(tokenize_text(value));
					else if (pattern.first == "CHAR_LITERAL") tokens.push_back(Token(LITERAL, { { "data_type", "char" }, { "value", match[1]} }));
					else if (pattern.first == "OPERATOR") tokens.push_back(Token(OPERATOR, { { "value", value } }));
					else if (pattern.first == "SPECIAL") tokens.push_back(Token(SPECIAL_CHAR, { { "value", value } }));
				}
				text = text.substr(value.size());
				cntr += value.size();
				break;
			}
		}
		if (match.empty())
			return cntr;
	}

	tokens.push_back(Token(EOF_TOKEN, {}));
	return tokens;
}

Token Lexer::tokenize_text(const std::string& value)
{
	if (std::find(m_keywords.begin(), m_keywords.end(), value) != m_keywords.end())
	{
		return Token(KEYWORD, { { "value", value } });
	}
	if (std::find(m_types.begin(), m_types.end(), value) != m_types.end())
	{
		return Token(TYPE, { { "value", value } });
	}
	return Token(IDENTIFIER, { { "name", value } });
}

Token Lexer::tokenize_number(const std::string& value)
{
	if (value.find('.') != std::string::npos)
	{
		return Token(LITERAL, { { "data_type", "float" }, { "value", value } });
	}
	return Token(LITERAL, { { "data_type", "integer" }, { "value", value } });
}