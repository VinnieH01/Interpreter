#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#include "Lexer.h"

Lexer::Lexer()
	: m_text(nullptr)
	, m_index(0)
	, m_current_char('\0')
{}

void Lexer::advance()
{
	m_index++;
	m_current_char = (m_index < m_text->size()) ? ((*m_text)[m_index]) : '\0';
}

using TokenResult = Result<Token, const char*>;

Result<std::vector<Token>, std::vector<const char*>> Lexer::tokenize(const std::string& text)
{
	m_index = 0;
	m_text = &text;
	m_current_char = text[0];

	std::vector<const char*> errors;
	std::vector<Token> tokens;
	while (m_current_char != '\0')
	{
		if (std::isspace(m_current_char))
		{
			advance();
		}
		else if (std::isalpha(m_current_char) || m_current_char == '_')
		{
			TokenResult res = tokenize_identifier();
			if (res.is_error()) 
				errors.push_back(res.get_error());
			else
			tokens.push_back(*res);
		}
		else if (std::find(m_operator_constituents.begin(), m_operator_constituents.end(), m_current_char) != m_operator_constituents.end())
		{
			TokenResult res = tokenize_operator();
			if (res.is_error())
				errors.push_back(res.get_error());
			else
				tokens.push_back(*res);
		}

		//This has to be done after tokenize_operator since some special chars could potentially be operator constituents in the future
		else if (m_special_chars.find(m_current_char) != m_special_chars.end())
		{
			TokenResult res = tokenize_special_char();
			if (res.is_error())
				errors.push_back(res.get_error());
			else
				tokens.push_back(*res);
		}
		else if (std::isdigit(m_current_char))
		{
			TokenResult res = tokenize_number();
			if (res.is_error())
				errors.push_back(res.get_error());
			else
				tokens.push_back(*res);
		}
		else if (m_current_char == '"')
		{
			TokenResult res = tokenize_string();
			if (res.is_error())
				errors.push_back(res.get_error());
			else
				tokens.push_back(*res);
		}
		else
		{
			errors.push_back("Unkown token");
			advance();
		}
	}
	tokens.push_back(Token(EOF_TOKEN, {}));

	if (!errors.empty())
		return errors;
	return tokens;
}

TokenResult Lexer::tokenize_identifier()
{
	std::string result = "";
	while (m_current_char != '\0' && (std::isalnum(m_current_char) || m_current_char == '_'))
	{
		result += m_current_char;
		advance();
	}
	if (m_keywords.find(result) != m_keywords.end())
	{
		return Token(m_keywords.at(result), {});
	}
	return Token(IDENTIFIER, { { "name", result } });
}

TokenResult Lexer::tokenize_operator()
{
	std::string result(1, m_current_char);
	advance();
	std::string double_op = result + std::string(1, m_current_char);
	if (std::find(m_operators.begin(), m_operators.end(), double_op) != m_operators.end())
	{
		advance();
		return Token(OPERATOR, { { "operator", double_op } });
	}
	if (m_keywords.find(result) != m_keywords.end())
	{
		return Token(m_keywords.at(result), {});
	}
	if (std::find(m_operators.begin(), m_operators.end(), result) != m_operators.end())
	{
		return Token(OPERATOR, { { "operator", result } });
	}

	return "Unknown token";
}

TokenResult Lexer::tokenize_number()
{
	std::string result = "";
	while (m_current_char != '\0' && std::isdigit(m_current_char))
	{
		result += m_current_char;
		advance();
	}
	if (m_current_char == '.')
	{
		result += m_current_char;
		advance();
		while (m_current_char != '\0' && std::isdigit(m_current_char))
		{
			result += m_current_char;
			advance();
		}
		return Token(LITERAL, { { "data_type", "float" }, { "value", result } });
	}
	return Token(LITERAL, { { "data_type", "integer" }, { "value", result } });
}

TokenResult Lexer::tokenize_special_char()
{
	char special = m_current_char;
	advance();
	return Token(m_special_chars.at(special), {});
}

TokenResult Lexer::tokenize_string()
{
	std::string result = "";
	advance(); // Skip the first "
	while (m_current_char != '\0' && m_current_char != '"')
	{
		result += m_current_char;
		advance();
	}
	advance(); // Skip the last "
	return Token(LITERAL, { { "data_type", "string" }, { "value", result } });
}