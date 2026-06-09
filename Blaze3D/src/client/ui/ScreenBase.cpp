#include "ScreenBase.h"

#include "engine/Font.h"
#include "engine/RenderingInterface.h"
#include "engine/System.h"
#include "engine/Audio.h"

#include "client/renderer/VertexProducer.h"
#include "client/renderer/RenderHelper.h"
#include "client/ResourceManager.h"
#include "client/Client.h"

#include "common/entity/PlayerEntity.h"
#include "common/ItemStack.h"

extern VertexProducer producer;
extern sysInputData g_InputData;

void ScreenBase::Begin()
{
	m_NextCursorPosition = {};
	m_HasHitButton = false;
}

void ScreenBase::End()
{
	auto& client = Client::GetInstance();
	if (glm::ivec2(m_NextCursorPosition) != glm::ivec2(0))
	{
		glm::vec2 absolutePos = (m_NextCursorPosition / client.UiScreenSize) * client.ScreenSize;
		sysSetVMousePosition(absolutePos.x, absolutePos.y);
	}
}

void ScreenBase::DrawCenteredTexturedRect(glm::vec2 pos, glm::vec2 size, glm::vec2 minUv, glm::vec2 maxUv, glm::vec2 texSize, void* texture, glm::vec4 color1)
{
	producer.Reset();
	producer.SetColor(color1.r, color1.g, color1.b, color1.a);
	RenderHelper::RenderCenteredTexturedRect(pos, size, minUv / texSize, maxUv / texSize, producer);

	m_Device->SetTexturing(true);
	m_Device->SetTexture(texture, 0);
	m_Device->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	m_Device->SetTexturing(false);
}

void ScreenBase::DrawGradient(glm::vec2 position, glm::vec2 size, glm::vec4 color1, glm::vec4 color2)
{
	size /= 2.0f;
	producer.Reset();
	producer.SetColor(color1.r, color1.g, color1.b, color1.a);
	producer.AddVertex(position.x - size.x, position.y + size.y, 0.0f, 0.0f, 0.0f);
	producer.AddVertex(position.x + size.x, position.y + size.y, 0.0f, 0.0f, 0.0f);
	producer.SetColor(color2.r, color2.g, color2.b, color2.a);
	producer.AddVertex(position.x + size.x, position.y - size.y, 0.0f, 0.0f, 0.0f);
	producer.AddVertex(position.x - size.x, position.y - size.y, 0.0f, 0.0f, 0.0f);
	m_Device->SetTexturing(false);
	m_Device->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
}

void ScreenBase::DrawText(glm::vec2 position, const std::string& text, float align, glm::vec4 color)
{
	if (align > 0.0f)
	{
		position.x += Font::Width(text) * align;
	}

	uint32_t r = static_cast<uint32_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
	uint32_t g = static_cast<uint32_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
	uint32_t b = static_cast<uint32_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
	uint32_t a = static_cast<uint32_t>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f);

	Font::DrawShadow(text, position.x, position.y, (a << 24) | (b << 16) | (g << 8) | r);
}

bool ScreenBase::DrawButton(glm::vec2 position, glm::vec2 size, const std::string& text, bool enabled)
{
	size /= 2.0f;
	producer.Reset();

	glm::vec2 minUv = glm::vec2{ 0, 46 };
	glm::vec2 maxUv = glm::vec2{ 200, 66 };

	if (enabled)
	{
		minUv.y += 20.0f;
		maxUv.y += 20.0f;
	}

	auto& client = Client::GetInstance();

	producer.SetColor(0xFFFFFFFF);

	bool active = false;

	if (enabled)
	{
		InternalButtonSnap(position, "misc.focus");
	}

	int textColor = 16777215;

	if (client.CursorPosition.x > position.x - size.x &&
		client.CursorPosition.y > position.y - size.y &&
		client.CursorPosition.x < position.x + size.x &&
		client.CursorPosition.y < position.y + size.y && enabled)
	{
		minUv.y += 20.0f;
		maxUv.y += 20.0f;

		textColor = 0xFF00FFFF;

		if (g_InputData.UiMainPressed && !m_HasHitButton)
		{
			m_HasHitButton = true;
			active = true;
			AudioEngine::PlaySound("misc.click", 0.2f);
		}
	}

	RenderHelper::RenderCenteredTexturedRect(position, size * 2.0f, minUv / 256.0f, maxUv / 256.0f, producer);

	client.pResourceManager->BindTexture(E_TextureResource_GuiControls, 0);
	m_Device->SetTexturing(true);
	m_Device->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	m_Device->SetTexturing(false);

	Font::DrawCenteredString(text, position.x, position.y - 4.0f, textColor);

	return active;
}

ScreenClickType ScreenBase::DrawItemSlot(glm::vec2 position, float scale, ItemStack& stack, int slotId, bool enabled, bool mouseEnabled)
{
	auto& client = Client::GetInstance();
	const glm::vec2 size = glm::vec2(16.0f);

	ScreenClickType clickType = SCREEN_CLICK_INVALID;

	if (mouseEnabled &&
		client.CursorPosition.x > position.x - size.x / 2.0f &&
		client.CursorPosition.y > position.y - size.y / 2.0f &&
		client.CursorPosition.x < position.x + size.x / 2.0f &&
		client.CursorPosition.y < position.y + size.y / 2.0f)
	{
		DrawGradient(position, size, glm::vec4(glm::vec3(1.0f), 0.5f), glm::vec4(glm::vec3(1.0f), 0.5f));

		if (g_InputData.UiMainPressed)
		{
			clickType = ITEM_SLOT_MAIN_CLICK;

			if (enabled)
			{
				auto& playerStack = client.Player->CursorStack;
				// PlayerStack and Stack are the same, add amount from player stack
				if (playerStack.Id == stack.Id)
				{
					auto left = stack.Expand(playerStack.Amount);
					playerStack.Shrink(playerStack.Amount - left);
				}
				// PlayerStack is empty or is not the same, swap them
				else
				{
					playerStack.Take(stack);
				}
			}
		}
		else if (g_InputData.UiSecondaryPressed)
		{
			clickType = ITEM_SLOT_SECONDARY_CLICK;
			if (enabled)
			{
				auto& playerStack = client.Player->CursorStack;
				// PlayerStack is empty then take half of the stack
				if (playerStack.Id == 0 || playerStack.Amount == 0)
				{
					playerStack.TakeHalf(stack);
				}
				else
				{
					// PlayerStack Id doesn't match Stack Id, swap them
					if (playerStack.Id != stack.Id && stack.Id != 0)
					{
						playerStack.Take(stack);
					}
					// PlayerStack id matches or Stack is empty, add one item to Stack
					else if (playerStack.Id != 0 && (stack.Id == playerStack.Id || stack.Id == 0))
					{
						stack.Id = playerStack.Id;
						auto amt = stack.Expand(1);
						playerStack.Shrink(1 - amt);
					}
				}
			}
		}
	}

	producer.Reset();
	RenderHelper::RenderItemStack(&stack, position + glm::vec2(-1, -2.25f), producer, m_Device);

	if (mouseEnabled)
		InternalButtonSnap(position, "misc.focus.slot");

	return clickType;
}

void ScreenBase::InternalButtonSnap(glm::vec2 position, const std::string& sound)
{
	auto& client = Client::GetInstance();
	if (g_InputData.UiUpPressed || g_InputData.UiDownPressed || g_InputData.UiLeftPressed || g_InputData.UiRightPressed)
	{
		if (glm::ivec2(m_NextCursorPosition) != glm::ivec2(position) && glm::ivec2(client.CursorPosition) != glm::ivec2(position))
		{
			float currDist = glm::distance(client.CursorPosition, m_NextCursorPosition);
			if (m_NextCursorPosition.x == 0.0f && m_NextCursorPosition.y == 0.0f)
			{
				currDist = 999999.0f;
			}
			float nxtDist = glm::distance(client.CursorPosition, position);
			bool allowNxt =
				(g_InputData.UiUpPressed && client.CursorPosition.y > position.y + 8.0f) ||
				(g_InputData.UiDownPressed && client.CursorPosition.y < position.y - 8.0f) ||
				(g_InputData.UiLeftPressed && client.CursorPosition.x > position.x + 8.0f) ||
				(g_InputData.UiRightPressed && client.CursorPosition.x < position.x - 8.0f);
			if (allowNxt && nxtDist < currDist && nxtDist > 8.0f)
			{
				m_NextCursorPosition = position;

				if (sound.ends_with("focus"))
				{
					if (g_InputData.UiUpPressed || g_InputData.UiRightPressed)
						AudioEngine::PlaySound("misc.focus.fast", 0.1f);
					if (g_InputData.UiDownPressed || g_InputData.UiLeftPressed)
						AudioEngine::PlaySound("misc.focus.slow", 0.1f);
				}
				if (!sound.empty())
				{
					AudioEngine::PlaySound(sound, 0.1f);
				}
			}
		}
	}
}

void ScreenBase::RenderMenuBase(glm::vec2 sz)
{
	float size = 512.0f;
	float offset = fmod(sysTime() * 16.0f, 1024);

	if (offset > -size / 2.0f)
	{
		offset += -size / 2.0f;
	}

	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(size / 2.0f - offset, 0.0f), glm::vec2(size, sz.y), { 0, 0 }, { 1, 180 }, { 1, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Panorama02));
	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(size + (size / 2.0f) - offset, 0.0f), glm::vec2(size, sz.y), { 0, 0 }, { 1, 180 }, { 1, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Panorama01));
	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(-size / 2.0f - offset, 0.0f), glm::vec2(size, sz.y), { 0, 0 }, { 1, 180 }, { 1, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Panorama01));

	DrawCenteredTexturedRect({ sz.x / 2.0f - 68.5f, sz.y / 4.0f }, { 155, 44 }, { 0, 0 }, { 155, 44 }, { 256, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Logo));
	DrawCenteredTexturedRect({ sz.x / 2.0f + 68.5f, sz.y / 4.0f }, { 118, 44 }, { 0, 45 }, { 118, 89 }, { 256, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Logo));

	DrawCenteredTexturedRect({ sz.x / 2.0f + 34.5f - 17.25f, sz.y / 4.0f + 22.0f }, { 107, 23 }, { 1, 90 }, { 107, 112 }, { 256, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Logo));
	DrawCenteredTexturedRect({ sz.x / 2.0f - 34.5f - 17.25f, sz.y / 4.0f + 22.0f }, { 34, 23 }, { 1, 114 }, { 34, 136 }, { 256, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Logo));

}
