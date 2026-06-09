#include "ScreenContainer.h"
#include "SlotIds.h"

#include "client/ResourceManager.h"
#include "client/Client.h"

#include "common/entity/PlayerEntity.h"
#include "common/tile/ChestTileEntity.h"
#include "common/World.h"

#include "engine/System.h"

extern sysInputData g_InputData;

ScreenContainer::ScreenContainer(riDevice* device, int x, int y, int z)
{
	m_Device = device;
	m_PosX = x;
	m_PosY = y;
	m_PosZ = z;
}

void ScreenContainer::Render(glm::vec2 sz)
{
	auto& client = Client::GetInstance();
	auto tileEntity = client.GetInstance().Player->EntityWorld->GetTileEntity(m_PosX, m_PosY, m_PosZ);

	if (!tileEntity || !dynamic_cast<ChestTileEntity*>(tileEntity))
	{
		client.SetScreen(NULL);
		return;
	}

	auto chest = static_cast<ChestTileEntity*>(tileEntity);
	auto neighbour = dynamic_cast<ChestTileEntity*>(chest->GetNeighbour());

	float offsetY = 0.0f;

	if (neighbour)
	{
		offsetY = 27;

		if (chest->posX > neighbour->posX || chest->posZ > neighbour->posZ)
		{
			auto s = neighbour;
			neighbour = chest;
			chest = s;
		}
	}

	Begin();
	DrawGradient(sz / 2.0f, sz, { 0.5f, 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f, 0.0f });
	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(0, 34.5f + offsetY), { 176, 97 }, { 0, 125 }, { 176, 222 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiContainer));
	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(0, 34.5f - 75 + offsetY), { 176, 54 }, { 0, 71 }, { 176, 125 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiContainer));

	for (int x = 0; x < 9; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			glm::vec2 pos = { sz.x / 2.0f - 72 + x * 18, sz.y / 2.0f + offsetY - 58.5f + y * 18 };
			DrawItemSlot(pos, 1.0f, chest->Inventory[y * 9 + x], 0);
		}
	}

	if (neighbour)
	{
		DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(0, 34.5f - 75 + offsetY - 54), { 176, 54 }, { 0, 71 }, { 176, 125 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiContainer));
		for (int x = 0; x < 9; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				int yy = y - 3;
				glm::vec2 pos = { sz.x / 2.0f - 72 + x * 18, sz.y / 2.0f + offsetY - 58.5f + yy * 18 };
				DrawItemSlot(pos, 1.0f, neighbour->Inventory[y * 9 + x], 0);
			}
		}
	}
	float o = 57.0f;
	if (neighbour)
	{
		o = 111.f;
	}
	DrawCenteredTexturedRect(sz / 2.0f + glm::vec2(0, 35.f - o + offsetY - 54), { 176, 17 }, { 0, 0 }, { 176, 17 }, { 256, 256 }, client.pResourceManager->GetHandle(E_TextureResource_GuiContainer));

	for (int x = 0; x < 9; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			glm::vec2 pos = { sz.x / 2.0f - 72 + x * 18, sz.y / 2.0f + offsetY + 9 + y * 18 };
			DrawItemSlot(pos, 1.0f, client.Player->Inventory.InventoryResources[y * 9 + x], 0);
		}
	}
	for (int i = 0; i < 9; i++)
	{
		glm::vec2 pos = { sz.x / 2.0f - 72 + i * 18, sz.y / 2.0f + offsetY + 67 };
		DrawItemSlot(pos, 1.0f, client.Player->Inventory.Resources[i], 0);
	}
	DrawItemSlot(client.CursorPosition, 1.0f, client.Player->CursorStack, 0, false, false);
	End();

	if (g_InputData.BackActionDown || g_InputData.InventoryActionDown)
	{
		client.SetScreen(NULL);
	}
}

void ScreenContainer::ReceiveSlotUpdate(int slotId, ItemStack& newItemStack)
{
	
}
