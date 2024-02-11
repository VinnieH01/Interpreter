#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include "Value.h"

class ScopeManager
{
public:
	std::shared_ptr<Value>* get_variable(const std::string& name);
	void add_variable(const std::string& name, std::shared_ptr<Value> value);
	void push_scope();
	void pop_scope();
private:
	std::vector<std::unordered_map<std::string, std::shared_ptr<Value>>> m_scopes;
};