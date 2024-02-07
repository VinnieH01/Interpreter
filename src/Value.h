#pragma once

#include <memory>
#include <string>
#include "Result.h"

template <typename T>
class NumberValue;

class ValueVisitor 
{
public:
	virtual Result<std::shared_ptr<class Value>, const char*> visit(const NumberValue<int>&) = 0;
	virtual Result<std::shared_ptr<class Value>, const char*> visit(const NumberValue<float>&) = 0;
	virtual Result<std::shared_ptr<class Value>, const char*> visit(const NumberValue<char>&) = 0;
	virtual Result<std::shared_ptr<class Value>, const char*> visit(const class StringValue&) = 0;
};

class Value
{
public:
	virtual ~Value() {}
	inline virtual Result<std::shared_ptr<Value>, const char*> accept(ValueVisitor& visitor) const = 0;
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
	inline virtual Result<std::shared_ptr<Value>, const char*> accept(ValueVisitor& visitor) const override
	{
		return visitor.visit(*this);
	}
	T value;
};

class StringValue : public Value
{
public:
	inline StringValue(const std::string& text)
		: text(text) {}
	std::string text;
	inline virtual Result<std::shared_ptr<Value>, const char*> accept(ValueVisitor& visitor) const override
	{
		return visitor.visit(*this);
	}
};
