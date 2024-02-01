#pragma once

#include <memory>

class Value
{
public:
	virtual ~Value() {}
	virtual void print(std::ostream&) const = 0;
	friend std::ostream& operator<<(std::ostream& os, const Value& val);
private:

};

inline std::ostream& operator<< (std::ostream& out, const Value& mc) {
	mc.print(out);
	return out;
}

template <typename T>
class NumberValue : public Value
{
public:
	NumberValue(T value) : value(value) {}
	NumberValue() { value = 0; }
	operator T() const { return value; }
	T value;
	void print(std::ostream& stream) const override
	{
		stream << value;
	}
};

class StringValue : public Value
{
public:
	inline StringValue(const std::string& text)
		: text(text) {}
	std::string text;

	void print(std::ostream& stream) const override
	{
		stream << text;
	}
};
