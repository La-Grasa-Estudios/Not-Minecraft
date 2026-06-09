#include "RenderHelper.h"

#include "engine/RenderingInterface.h"
#include "engine/Font.h"

#include "client/Client.h"
#include "client/ResourceManager.h"
#include "client/renderer/VertexProducer.h"
#include "client/renderer/block/BlockPreviewRenderer.h"
#include "common/ItemStack.h"
#include "common/block/Block.h"
#include "common/item/Item.h"

void RenderHelper::RenderTexturedRect(glm::vec2 Position, glm::vec2 s, glm::vec2 minUv, glm::vec2 maxUv, VertexProducer& producer)
{
	producer.AddVertex(Position.x, Position.y + s.y, -100.0f, minUv.x, maxUv.y);
	producer.AddVertex(Position.x + s.x, Position.y + s.y, -100.0f, maxUv.x, maxUv.y);
	producer.AddVertex(Position.x + s.x, Position.y, -100.0f, maxUv.x, minUv.y);
	producer.AddVertex(Position.x, Position.y, -100.0f, minUv.x, minUv.y);
}

void RenderHelper::RenderBlockOverlay(glm::vec2 Position, glm::vec2 s, int texture, VertexProducer& producer)
{
	int xt = (texture % 16) * 16;
	int yt = (texture / 16) * 16;
	auto minUv = glm::vec2{ xt, yt } / 256.0f;
	auto maxUv = glm::vec2{ xt + 16, yt + 16 } / 256.0f;
	producer.AddVertex(Position.x, Position.y + s.y, -100.0f, minUv.x, maxUv.y);
	producer.AddVertex(Position.x + s.x, Position.y + s.y, -100.0f, maxUv.x, maxUv.y);
	producer.AddVertex(Position.x + s.x, Position.y, -100.0f, maxUv.x, minUv.y);
	producer.AddVertex(Position.x, Position.y, -100.0f, minUv.x, minUv.y);
}

void RenderHelper::RenderCenteredTexturedRect(glm::vec2 Position, glm::vec2 s, glm::vec2 minUv, glm::vec2 maxUv, VertexProducer& producer)
{
	s /= 2.0f;
	producer.AddVertex(Position.x - s.x, Position.y + s.y, -100.0f, minUv.x, maxUv.y);
	producer.AddVertex(Position.x + s.x, Position.y + s.y, -100.0f, maxUv.x, maxUv.y);
	producer.AddVertex(Position.x + s.x, Position.y - s.y, -100.0f, maxUv.x, minUv.y);
	producer.AddVertex(Position.x - s.x, Position.y - s.y, -100.0f, minUv.x, minUv.y);
}

void RenderHelper::RenderItemStack(ItemStack* itemStack, const glm::vec2& pos, VertexProducer& producer, riDevice* device)
{
	if (itemStack->Id <= 0 || itemStack->Amount <= 0)
		return;

	auto& client = Client::GetInstance();

	auto model = glm::mat4(1.0f);

	producer.Reset();

	bool renderWithLighting = false;

	device->SetTexturing(true);
	if (itemStack->Id < 256 && !Block::GetBlock(itemStack->Id)->RendersAsIcon())
	{
		renderWithLighting = true;
		model = glm::translate(model, glm::vec3(pos, -50.0f));
		model = glm::scale(model, glm::vec3(10.0f));
		model = glm::translate(model, glm::vec3(1.5f, -0.5f, 0.0f));
		model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-1.5f, 0.5f, 0.5f));
		model = glm::scale(model, glm::vec3(-1.0f));

		auto block = Block::GetBlock(itemStack->Id);
		BlockPreviewRenderer::RenderBlock(&producer, block);
		client.pResourceManager->BindTexture(E_TextureResource_Terrain, device);
	}
	else
	{
		int xt, yt;

		if (itemStack->Id >= 256)
		{
			auto item = Item::GetItem(itemStack->Id);
			xt = item->GetIconIndex() % 16;
			yt = item->GetIconIndex() / 16;
			client.pResourceManager->BindTexture(E_TextureResource_Items, device);
		}
		else
		{
			auto block = Block::GetBlock(itemStack->Id);
			xt = block->GetIconIndex() % 16;
			yt = block->GetIconIndex() / 16;
			client.pResourceManager->BindTexture(E_TextureResource_Terrain, device);
		}

		glm::vec2 minUv = { xt * 16, yt * 16 };
		glm::vec2 maxUv = minUv + glm::vec2(16.0f);
		producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderCenteredTexturedRect(pos + glm::vec2(0.5f, 2.0f), { 16, 16 }, minUv / 256.0f, maxUv / 256.0f, producer);
	}

	device->PushMatrixStack();
	device->SetModelViewMatrix(glm::value_ptr(model));

	if (renderWithLighting)
	{
		EnableItemLighting();
	}

	device->Draw(producer.GetVertexPointer(), renderWithLighting ? 2 : 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	device->SetTexturing(false);

	if (itemStack->Id >= 256)
	{
		auto item = Item::GetItem(itemStack->Id);
		if (item->GetType() != TOOL_TYPE_NONE && itemStack->Damage > 0)
		{
			float damageProgress = (float)(item->GetMaxDamage() - itemStack->Damage) / (float)item->GetMaxDamage();
			producer.Reset();
			producer.SetColor(0.0f, 0.0f, 0.0f);
			RenderTexturedRect(pos + glm::vec2(-5.5f, 7.0f), { 13.0f, 2.0f }, {}, {}, producer);
			producer.SetColor(1.0f - damageProgress, damageProgress, 0.0f, 1.0f);
			RenderTexturedRect(pos + glm::vec2(-5.5f, 7.0f), { 13.0f * damageProgress, 1.0f }, {}, {}, producer);
			device->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		}
	}

	DisableItemLighting();

	device->PopMatrixStack();

	if (itemStack->Amount > 1)
	{
		char buffer[8]{};
		snprintf(buffer, 8, "%i", itemStack->Amount);
		Font::DrawCenteredString(buffer, pos.x + 4, pos.y + 2, 16777215);
	}
}

void RenderHelper::EnableItemLighting()
{
	auto device = Client::GetInstance().GraphicsDevice;

	auto zero     = glm::vec3(0.0F);
	auto ambient  = glm::vec3(0.2F);
	auto diffuse  = glm::vec3(0.5F);
	auto specular = glm::vec3(0.0F);

	auto dir1 = glm::vec3(0.2f, 1.0f, -0.7f);	
	auto dir2 = glm::vec3(-0.2f, 1.0f, 0.7f);

	riLight light1 = {};
	light1.LightType = RI_LIGHT_TYPE_DIRECTIONAL;
	light1.Data.Directional.Ambient = ambient;
	light1.Data.Directional.Diffuse = diffuse;
	light1.Data.Directional.Specular = specular;
	light1.Data.Directional.Direction = dir1;

	riLight light2 = {};
	light2.LightType = RI_LIGHT_TYPE_DIRECTIONAL;
	light2.Data.Directional.Ambient = ambient;
	light2.Data.Directional.Diffuse = diffuse;
	light2.Data.Directional.Specular = specular;
	light2.Data.Directional.Direction = dir2;

	device->SetLighting(true);
	device->SetLight(&light1, true, 0);
	device->SetLight(&light2, true, 1);
	device->SetSmoothShade(false);
}

void RenderHelper::DisableItemLighting()
{
	auto device = Client::GetInstance().GraphicsDevice;

	device->SetLighting(false);
	device->SetLight(nullptr, false, 0);
	device->SetLight(nullptr, false, 1);
	device->SetSmoothShade(true);
}
