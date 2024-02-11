#include "Token.h"
#include <iostream>

Token::Token(TokenType t, std::unordered_map<std::string, std::string> m, size_t position)
	: type(t)
	, m_meta(m)
	, m_position(position)
{}

void Token::print() const
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

const std::string& Token::get_string(const std::string& key) const
{
	return m_meta.at(key);
}

int Token::get_int(const std::string& key) const
{
	return std::stoi(m_meta.at(key).c_str());
}

float Token::get_float(const std::string& key) const
{
	return std::stof(m_meta.at(key).c_str());
}

char Token::get_char(const std::string& key) const
{
	return m_meta.at(key).c_str()[0];
}

bool Token::is(TokenType type, const std::string& value) const
{
	return this->type == type && m_meta.count("value") && m_meta.at("value") == value;
}

bool Token::is_not(TokenType type, const std::string& value) const
{
	return !is(type, value);
}
