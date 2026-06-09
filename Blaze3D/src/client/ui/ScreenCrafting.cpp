#include "ScreenCrafting.h"
#include "SlotIds.h"

#include "client/Client.h"
#include "client/ResourceManager.h"

#include "common/entity/PlayerEntity.h"
#include "common/Recipe.h"

#include "engine/System.h"

extern sysInputData g_InputData;

ScreenCrafting::ScreenCrafting(riDevice* device)
{
	m_Device = device;
	memset(CraftingGrid, 0, sizeof(CraftingGrid));
}

ScreenCrafting::~ScreenCrafting()
{

}

void ScreenCrafting::Render(glm::vec2 sz)
{
	Begin();
	DrawGradient(sz / 2.0f, sz, { 0.5f, 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f, 0.0f });
	DrawCenteredTexturedRect(sz / 2.0f, { 176, 166 }, {}, { 176, 166 }, { 256, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_GuiCrafting));
	auto& client = Client::GetInstance();
	for (int i = 0; i < 9; i++)
	{
		int x = i % 3;
		int y = i / 3;
		glm::vec2 pos = { sz.x / 2.0f - 50 + x * 18, sz.y / 2.0f - 58 + y * 18 };
		DrawItemSlot(pos, 1.0f, CraftingGrid[i], C_CRAFT_SLOT_START + i);
	}
	{

		glm::vec2 pos = { sz.x / 2.0f + 44, sz.y / 2.0f - 40 };
		auto recipe = Recipe::GetMatchingRecipe(CraftingGrid, 3, 3);
		ItemStack result{};
		if (recipe)
		{
			result = recipe->GetResult();
		}
		if (DrawItemSlot(pos, 1.0f, result, 0, false) == ITEM_SLOT_MAIN_CLICK)
		{
			if (result.Id != 0)
			{
				auto& stack = client.Player->CursorStack;
				bool tookItems = false;

				if (stack.Id == 0)
				{
					stack.Take(result);
					tookItems = true;
				}
				else if (stack.Id == result.Id)
				{
					auto before = stack.Amount;
					auto left = stack.Expand(result.Amount);
					if (left != 0)
					{
						stack.Amount = before;
					}
					else
					{
						tookItems = true;
					}
				}

				if (tookItems)
				{
					for (int i = 0; i < 9; i++)
					{
						CraftingGrid[i].Shrink(1);
					}
				}
			}
		}
	}
	for (int x = 0; x < 9; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			int id = y * 9 + x;
			glm::vec2 pos = { sz.x / 2.0f - 72 + x * 18, sz.y / 2.0f + 9 + y * 18 };
			DrawItemSlot(pos, 1.0f, client.Player->Inventory.InventoryResources[y * 9 + x], C_INVENTORY_SLOT_START + id);
		}
	}
	for (int i = 0; i < 9; i++)
	{
		glm::vec2 pos = { sz.x / 2.0f - 72 + i * 18, sz.y / 2.0f + 67 };
		DrawItemSlot(pos, 1.0f, client.Player->Inventory.Resources[i], C_HOTBAR_SLOT_START + i);
	}
	DrawItemSlot(client.CursorPosition, 1.0f, client.Player->CursorStack, 0, false, false);
	End();

	if (g_InputData.BackActionDown || g_InputData.InventoryActionDown)
	{
		client.SetScreen(NULL);
	}
}

ScreenType ScreenCrafting::GetScreenType()
{
	return E_ScreenType_Crafting;
}

void ScreenCrafting::ReceiveSlotUpdate(int slotId, ItemStack& newItemStack)
{
	if (slotId >= C_CRAFT_SLOT_START && slotId < C_CRAFT_SLOT_END)
	{
		int i = slotId - C_CRAFT_SLOT_START;
		CraftingGrid[i] = newItemStack;
	}
	if (slotId == C_CRAFT_RESULT_SLOT_ID)
	{
		Result = newItemStack;
	}
}
