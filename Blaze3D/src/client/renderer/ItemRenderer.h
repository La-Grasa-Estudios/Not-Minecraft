#pragma once

#include "common/ItemStack.h"

#include <thirdparty/glm/ext.hpp>

class VertexProducer;
class riDevice;

enum ItemRendererEnviroment
{
	ITEM_RENDERER_ENV_FIRST_PERSON,
	ITEM_RENDERER_ENV_THIRD_PERSON,
	ITEM_RENDERER_ENV_WORLD,
};

class ItemRenderer
{
public:
	// Renders an item in 3d view, call with the producer being in a clean state and with the desired lightLevel set
	static void RenderItemStack(const ItemStack& stack, VertexProducer* producer, riDevice* device, const glm::mat4& model, ItemRendererEnviroment env);
};