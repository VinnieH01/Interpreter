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
	inline Array(const std::string& type, size_t size)
		: m_size(size)
	{
		if (type == "int") m_type = ArrayType::INT; //TODO Move this to parser and use ArrayType in ASTArrayInitNode
		else if (type == "float") m_type = ArrayType::FLOAT;
		else if (type == "char") m_type = ArrayType::CHAR;
		else m_type = ArrayType::INT; //TODO: Should throw exception or something instead? Probably wont need this once the above code's been moved anyway

		switch (m_type)
		{
		case ArrayType::INT:
			m_ptr = std::shared_ptr<int>(new int[size], std::default_delete<int[]>());
			break;
		case ArrayType::FLOAT:
			m_ptr = std::shared_ptr<float>(new float[size], std::default_delete<float[]>());
			break;
		case ArrayType::CHAR:
			m_ptr = std::shared_ptr<char>(new char[size], std::default_delete<char[]>());
			break;
		default:
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