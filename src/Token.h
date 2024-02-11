#pragma once

#include <string>
#include <unordered_map>

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

	Token(TokenType t, std::unordered_map<std::string, std::string> m, size_t position);

	inline size_t get_position() const { return m_position; }

	void print() const;

	const std::string& get_string(const std::string& key) const;
	int get_int(const std::string& key) const;
	float get_float(const std::string& key) const;
	char get_char(const std::string& key) const;

	bool is(TokenType type, const std::string& value) const;
	//This only exists beacause it's clearer than saying !tok.is(...)
	bool is_not(TokenType type, const std::string& value) const;

private:
	const std::unordered_map<std::string, std::string> m_meta;
	const size_t m_position;
};
