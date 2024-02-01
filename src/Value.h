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

/*template<typename T>
class ArrayValue : public Value
{
public:
	inline ArrayValue(size_t size)
		: m_size(size)
	{
		m_ptr = std::shared_ptr<Value>(new T[size], std::default_delete<T[]>());
	}

	inline size_t get_size() const { return m_size; }
	inline Value* get() const { return m_ptr.get(); }

private:
	std::shared_ptr<Value> m_ptr;
	size_t m_size;
};*/
