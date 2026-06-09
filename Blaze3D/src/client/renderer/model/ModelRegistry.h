#pragma once

#include <unordered_map>
#include <string>

#include "ModelBase.h"

class ModelRegistry
{
public:
	ModelRegistry();
	static ModelRegistry& GetInstance();
	ModelBase* GetModel(const std::string_view name);
private:
	std::unordered_map<std::string, ModelBase*> m_Models;
};