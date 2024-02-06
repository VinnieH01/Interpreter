#pragma once

#include <memory>

class Value
{
public:
	virtual ~Value() {}
	inline virtual bool is_truthy() const { return false; }
};

template <typename T>
class NumberValue : public Value
{
public:
	NumberValue(T value) : value(value) {}
	NumberValue() { value = 0; }
	operator T() const { return value; }
	inline virtual bool is_truthy() const override { return value != 0; }
	T value;
};

class StringValue : public Value
{
public:
	inline StringValue(const std::string& text)
		: text(text) {}
	std::string text;
};
