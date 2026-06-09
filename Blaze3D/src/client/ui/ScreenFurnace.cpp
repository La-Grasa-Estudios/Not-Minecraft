#include "ScreenFurnace.h"
#include "SlotIds.h"

#include "common/World.h"
#include "client/Client.h"
#include "client/ResourceManager.h"
#include "common/entity/PlayerEntity.h"
#include "common/tile/FurnaceTileEntity.h"

#include "engine/System.h"

extern sysInputData g_InputData;

ScreenFurnace::ScreenFurnace(riDevice* device, int x, int y, int z)
{
	m_Device = device;
	m_PosX = x;
	m_PosY = y;
	m_PosZ = z;
}

void ScreenFurnace::Render(glm::vec2 sz)
{
	auto& client = Client::GetInstance();
	auto tileEntity = client.GetInstance().Player->EntityWorld->GetTileEntity(m_PosX, m_PosY, m_PosZ);

	if (!tileEntity || !dynamic_cast<FurnaceTileEntity*>(tileEntity))
	{
		client.SetScreen(NULL);
		return;
	}

	auto furnace = static_cast<FurnaceTileEntity*>(tileEntity);

	Begin();
	DrawGradient(sz / 2.0f, sz, { 0.5f, 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f, 0.0f });
	DrawCenteredTexturedRect(sz / 2.0f, { 176, 166 }, {}, { 176, 166 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiFurnace));

	int size = (int)(14.0f * furnace->GetRemainingBurnTime());
	int progress = (int)(27.0f * furnace->GetProgress());

	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(5.25f - (13.5f - progress / 2.0f), -40), { progress, 16 }, { 176, 14 }, { 176 + progress, 14 + 17 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiFurnace));
	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(-24, -41 + (8.0f - size / 2.0f)), { 14, size }, { 176, (14 - size) }, { 190, 14 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiFurnace));

	DrawItemSlot(glm::vec2(sz.x / 2.0f - 24, sz.y / 2.0f - 22), 1.0f, furnace->FuelItemStack, 0);
	DrawItemSlot(glm::vec2(sz.x / 2.0f - 24, sz.y / 2.0f - 58), 1.0f, furnace->CookItemStack, 0);
	if (DrawItemSlot(glm::vec2(sz.x / 2.0f + 36, sz.y / 2.0f - 40), 1.0f, furnace->ResultItemStack, false) == ITEM_SLOT_MAIN_CLICK)
	{
		auto& stack = client.Player->CursorStack;
		bool tookItems = false;

		if (stack.Id == 0)
		{
			stack.Take(furnace->ResultItemStack);
			tookItems = true;
		}
		else if (stack.Id == furnace->ResultItemStack.Id)
		{
			auto before = stack.Amount;
			auto left = stack.Expand(furnace->ResultItemStack.Amount);
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
			furnace->ResultItemStack.Shrink(1);
		}
	}

	for (int x = 0; x < 9; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			glm::vec2 pos = { sz.x / 2.0f - 72 + x * 18, sz.y / 2.0f + 9 + y * 18 };
			DrawItemSlot(pos, 1.0f, client.Player->Inventory.InventoryResources[y * 9 + x], 0);
		}
	}
	for (int i = 0; i < 9; i++)
	{
		glm::vec2 pos = { sz.x / 2.0f - 72 + i * 18, sz.y / 2.0f + 67 };
		DrawItemSlot(pos, 1.0f, client.Player->Inventory.Resources[i], 0);
	}
	DrawItemSlot(client.CursorPosition, 1.0f, client.Player->CursorStack, 0, false, false);
	End();

	if (g_InputData.BackActionDown || g_InputData.InventoryActionDown)
	{
		client.SetScreen(NULL);
	}
}

void ScreenFurnace::ReceiveSlotUpdate(int slotId, ItemStack& newItemStack)
{
	
}
