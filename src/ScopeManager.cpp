#include "ScopeManager.h"

std::shared_ptr<Value>* ScopeManager::get_variable(const std::string& name)
{
	for (auto scopes_it = m_scopes.rbegin(); scopes_it != m_scopes.rend(); ++scopes_it)
	{
		auto vars_it = scopes_it->find(name);
		if (vars_it != scopes_it->end())
			return &vars_it->second;
	}

	return nullptr;
}

void ScopeManager::add_variable(const std::string& name, std::shared_ptr<Value> value)
{
	m_scopes.back()[name] = value;
}

void ScopeManager::push_scope()
{
	m_scopes.emplace_back();
}

void ScopeManager::pop_scope()
{
	m_scopes.pop_back();
}