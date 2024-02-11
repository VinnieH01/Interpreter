#pragma once
#include <unordered_map>
#include <vector>
#include "Value.h"

class ScopeManager
{
public:
	inline std::shared_ptr<Value>* get_variable(const std::string& name)
	{
		for (auto scopes_it = m_scopes.rbegin(); scopes_it != m_scopes.rend(); ++scopes_it) 
		{
			auto vars_it = scopes_it->find(name);
			if (vars_it != scopes_it->end()) 
				return &vars_it->second;
		}

		return nullptr;
	}
	inline void add_variable(const std::string& name, std::shared_ptr<Value> value) 
	{
		m_scopes.back()[name] = value;
	}
	inline void push_scope() 
	{
		m_scopes.emplace_back();
	}
	inline void pop_scope()
	{
		m_scopes.pop_back();
	}
private:
	std::vector<std::unordered_map<std::string, std::shared_ptr<Value>>> m_scopes;
};