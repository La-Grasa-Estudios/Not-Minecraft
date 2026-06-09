#include "ScreenInventory.h"

#include "client/ResourceManager.h"
#include "client/Client.h"
#include "common/item/Item.h"
#include "common/entity/PlayerEntity.h"
#include "common/Recipe.h"
#include "SlotIds.h"

#include "engine/System.h"

extern sysInputData g_InputData;

ScreenInventory::ScreenInventory(riDevice* device)
{
	m_Device = device;
	memset(CraftingGrid, 0, sizeof(CraftingGrid));
}

ScreenInventory::~ScreenInventory()
{
	
}

void ScreenInventory::Render(glm::vec2 sz)
{
	Begin();
	DrawGradient(sz / 2.0f, sz, { 0.5f, 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f, 0.0f });
	DrawCenteredTexturedRect(sz / 2.0f, { 176, 166 }, {}, { 176, 166 }, { 256, 256 }, Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_GuiInventory));
	auto& client = Client::GetInstance();
	for (int i = 0; i < 4; i++)
	{
		int x = i % 2;
		int y = i / 2;
		glm::vec2 pos = { sz.x / 2.0f + 8 + x * 18, sz.y / 2.0f - 49 + y * 18 };
		DrawItemSlot(pos, 1.0f, CraftingGrid[i], C_CRAFT_SLOT_START + i);
	}
	{

		glm::vec2 pos = { sz.x / 2.0f + 64, sz.y / 2.0f - 39 };
		auto recipe = Recipe::GetMatchingRecipe(CraftingGrid, 2, 2);
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
					for (int i = 0; i < 4; i++)
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
			DrawItemSlot(pos, 1.0f, client.Player->Inventory.InventoryResources[id], C_INVENTORY_SLOT_START + id);
		}
	}
	for (int i = 0; i < 9; i++)
	{
		glm::vec2 pos = { sz.x / 2.0f - 72 + i * 18, sz.y / 2.0f + 67 };
		DrawItemSlot(pos, 1.0f, client.Player->Inventory.Resources[i], C_HOTBAR_SLOT_START + i);
	}

	for (int i = 0; i < 4; i++)
	{
		int x = 0;
		int y = i;
		glm::vec2 pos = { sz.x / 2.0f - 72.0f + x * 18, sz.y / 2.0f - 67 + y * 18 };
		if (DrawItemSlot(pos, 1.0f, client.Player->Inventory.Armor[i], C_ARMOR_SLOT_START + i, false, true) != SCREEN_CLICK_INVALID)
		{
			auto item = Item::GetItem(client.Player->CursorStack.Id);
			if (item && item->GetType() == i + TOOL_TYPE_ARMOR_HEAD)
			{
				client.Player->Inventory.Armor[i].Take(client.Player->CursorStack);
			}
			else if (client.Player->CursorStack.Id == 0)
			{
				client.Player->CursorStack.Take(client.Player->Inventory.Armor[i]);
			}
		}
	}

	DrawItemSlot(client.CursorPosition, 1.0f, client.Player->CursorStack, 0, false, false);
	End();

	if (g_InputData.BackActionDown || g_InputData.InventoryActionDown)
	{
		client.SetScreen(NULL);
	}
}

ScreenType ScreenInventory::GetScreenType()
{
	return E_ScreenType_Inventory;
}

void ScreenInventory::ReceiveSlotUpdate(int slotId, ItemStack& newItemStack)
{
	if (slotId >= C_CRAFT_SLOT_START && slotId < C_CRAFT_SLOT_START + 4)
	{
		CraftingGrid[slotId - C_CRAFT_SLOT_START] = newItemStack;
	}
	if (slotId == C_CRAFT_RESULT_SLOT_ID)
	{
		Result = newItemStack;
	}
}
