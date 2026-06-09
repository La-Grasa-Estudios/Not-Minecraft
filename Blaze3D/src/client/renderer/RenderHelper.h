#pragma once

#include "thirdparty/glm/ext.hpp"

class VertexProducer;
class ItemStack;
class riDevice;

class RenderHelper
{
public:
	static void RenderTexturedRect(glm::vec2 pos, glm::vec2 size, glm::vec2 minUv, glm::vec2 maxUv, VertexProducer& producer);
	static void RenderBlockOverlay(glm::vec2 pos, glm::vec2 size, int texture, VertexProducer& producer);
	static void RenderCenteredTexturedRect(glm::vec2 pos, glm::vec2 size, glm::vec2 minUv, glm::vec2 maxUv, VertexProducer& producer);
	static void RenderItemStack(ItemStack* itemStack, const glm::vec2& pos, VertexProducer& producer, riDevice* device);
	static void EnableItemLighting();
	static void DisableItemLighting();
};