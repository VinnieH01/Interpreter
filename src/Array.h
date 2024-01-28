#pragma once

#include <memory>

enum class ArrayType
{
	INT,
	FLOAT,
	CHAR
};

class Array
{
public:
	inline Array(ArrayType type, size_t size)
		: m_size(size)
		, m_type(type)
	{
		switch (m_type)
		{
		case ArrayType::INT:
			m_ptr = std::shared_ptr<int[]>(new int[size]); //Important to use int[] so the shared_ptr deleter uses delete[]
			break;
		case ArrayType::FLOAT:
			m_ptr = std::shared_ptr<float[]>(new float[size]);
			break;
		case ArrayType::CHAR:
			m_ptr = std::shared_ptr<char[]>(new char[size]);
			break;
		}
	}

	inline ArrayType get_type() const { return m_type; }
	inline size_t get_size() const { return m_size; }
	inline void* get() const { return m_ptr.get(); }

private:
	std::shared_ptr<void> m_ptr;
	size_t m_size;
	ArrayType m_type;
};