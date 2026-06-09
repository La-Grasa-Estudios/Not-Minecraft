#include "ModelRegistry.h"

#include "engine/RenderingInterface.h"

#include "client/Client.h"

#include "ModelHumanoid.h"
#include "ModelCow.h"
#include "ModelPig.h"
#include "ModelChicken.h"
#include "ModelCreeper.h"

ModelRegistry::ModelRegistry()
{
	m_Models["humanoid"] = new ModelHumanoid();

	m_Models["zombie"] = new ModelHumanoid();
	m_Models["zombie"]->Texture = Client::GetInstance().GraphicsContext->LoadTexture("mc/textures/mob/zombie.png");
	m_Models["zombie"]->SetParameter(1, 80.0f);

	m_Models["pigzombie"] = new ModelHumanoid();
	m_Models["pigzombie"]->Texture = Client::GetInstance().GraphicsContext->LoadTexture("mc/textures/mob/pigzombie.png");
	m_Models["pigzombie"]->SetParameter(1, 80.0f);

	Client::GetInstance().GraphicsContext->SetTextureFilter(m_Models["zombie"]->Texture, RI_FILTER_NEAREST);
	Client::GetInstance().GraphicsContext->SetTextureFilter(m_Models["pigzombie"]->Texture, RI_FILTER_NEAREST);
	m_Models["cow"] = new ModelCow();
	m_Models["pig"] = new ModelPig();
	m_Models["chicken"] = new ModelChicken();
	m_Models["creeper"] = new ModelCreeper();
}

ModelRegistry& ModelRegistry::GetInstance()
{
	static ModelRegistry instance;
	return instance;
}

ModelBase* ModelRegistry::GetModel(const std::string_view name)
{
	auto it = m_Models.find(name.data());
	if (it != m_Models.end())
	{
		return it->second;
	}
	return nullptr;
}
