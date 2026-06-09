#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <memory>

class riDevice;
class IEntityRenderer;
class BaseEntity;

class EntityRendererDispatcher
{
public:

	template<typename Entity, typename Renderer>
	static void RegisterEntityRenderer()
	{
		static_assert(std::is_base_of<IEntityRenderer, Renderer>());
		s_Renderers[typeid(Entity)] = new Renderer();
	}

	static void Init();
	static void DispatchEntityRender(BaseEntity* pEntity, riDevice* pDevice);

private:

	static inline std::unordered_map<std::type_index, IEntityRenderer*> s_Renderers;

};