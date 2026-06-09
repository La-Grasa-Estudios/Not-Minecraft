#pragma once

#include "thirdparty/glm/ext.hpp"

#include <string>

class riDevice;
class ItemStack;

enum ScreenClickType
{
	SCREEN_CLICK_INVALID,
	ITEM_SLOT_MAIN_CLICK,
	ITEM_SLOT_SECONDARY_CLICK,
};

enum ScreenType
{
	E_ScreenType_None,
	E_ScreenType_Inventory,
	E_ScreenType_Crafting,
	E_ScreenType_Container,
	E_ScreenType_Furnace,
};

class ScreenBase
{
public:
	virtual void Render(glm::vec2 sz) {}
	virtual bool DoesGuiPauseGame() { return false; }
	virtual void ReceiveSlotUpdate(int slotId, ItemStack& newItemStack) {}
	virtual ScreenType GetScreenType() { return E_ScreenType_None; }
protected:
	riDevice* m_Device;
	void Begin();
	void End();
	void DrawCenteredTexturedRect(glm::vec2 pos, glm::vec2 size, glm::vec2 minUv, glm::vec2 maxUv, glm::vec2 texSize, void* texture, glm::vec4 color = glm::vec4(1.0f));
	void DrawGradient(glm::vec2 position, glm::vec2 size, glm::vec4 color1, glm::vec4 color2);
	void DrawText(glm::vec2 position, const std::string& text, float align, glm::vec4 color = glm::vec4(1.0f));
	bool DrawButton(glm::vec2 position, glm::vec2 size, const std::string& text, bool enabled = true);
	ScreenClickType DrawItemSlot(glm::vec2 position, float scale, ItemStack& stack, int slotId, bool enabled = true, bool mouseEnabled = true);
	void InternalButtonSnap(glm::vec2 position, const std::string& sound = "");
	glm::vec2 m_NextCursorPosition = {};
	bool m_HasHitButton = false;

	void RenderMenuBase(glm::vec2 sz);
};