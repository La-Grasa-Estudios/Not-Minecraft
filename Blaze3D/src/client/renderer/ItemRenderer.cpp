#include "ItemRenderer.h"

#include "engine/RenderingInterface.h"

#include "client/renderer/VertexProducer.h"
#include "client/renderer/block/BlockPreviewRenderer.h"
#include "common/ItemStack.h"
#include "client/ResourceManager.h"
#include "client/Client.h"

#include "common/item/Item.h"
#include "common/block/Block.h"

#include "engine/System.h"

#include <fstream>

void ItemRenderer::RenderItemStack(const ItemStack& stack, VertexProducer* producer, riDevice* device, const glm::mat4& view, ItemRendererEnviroment env)
{
	auto client = Client::GetInstance();
	if (stack.Id >= 256 || Block::GetBlock(stack.Id)->RendersAsIcon())
	{
		producer->SetColor(1.0f, 1.0f, 1.0f);

		int s = 0, t = 0;

		if (stack.Id >= 256)
		{
			auto item = Item::GetItem(stack.Id);
			s = item->GetIconIndex() % 16;
			t = item->GetIconIndex() / 16;
			client.pResourceManager->BindTexture(E_TextureResource_Items, device);
		}
		else
		{
			auto block = Block::GetBlock(stack.Id);
			s = block->GetIconIndex() % 16;
			t = block->GetIconIndex() / 16;
			client.pResourceManager->BindTexture(E_TextureResource_Terrain, device);
		}

		s *= 16;
		t *= 16;

		float u0 = s / 256.0f;
		float u2 = (s + 16) / 256.0f;
		float v0 = t / 256.0f;
		float v2 = (t + 16) / 256.0f;

		const float itemWidth = 16.0f / 256.0f;
		const float unitSize = 1.0f / 256.0f;

		producer->SetColor(1.0f, 1.0f, 1.0f);
		producer->SetNormal(0.0F, 0.0F, -1.0F);
		producer->AddVertex(0, 1.0f, -itemWidth, u2, v0);
		producer->AddVertex(0, 0, -itemWidth, u2, v2);
		producer->AddVertex(1.0f, 0, -itemWidth, u0, v2);
		producer->AddVertex(1.0f, 1.0f, -itemWidth, u0, v0);

		producer->SetNormal(0.0F, 0.0F, 1.0F);
		producer->AddVertex(0, 1.0f, 0, u2, v0);
		producer->AddVertex(1.0f, 1.0f, 0, u0, v0);
		producer->AddVertex(1.0f, 0, 0, u0, v2);
		producer->AddVertex(0, 0, 0, u2, v2);

		for (int i = 0; i < 16; i++)
		{
			float coordFront = (256.0f - (i + 1) * 16) / 256.0f;
			float coordBack = (256.0f - i * 16) / 256.0f;

			producer->SetNormal(-1.0F, 0.0F, 0.0F);
			producer->AddVertex(coordFront, 1.0f, 0.0f, u0 + ((i + 1) * unitSize), v0);
			producer->AddVertex(coordFront, 0.0f, 0.0f, u0 + ((i + 1) * unitSize), v2);
			producer->AddVertex(coordFront, 0.0f, -itemWidth, u0 + (i * unitSize), v2);
			producer->AddVertex(coordFront, 1.0f, -itemWidth, u0 + (i * unitSize), v0);

			producer->SetNormal(1.0F, 0.0F, 0.0F);
			producer->AddVertex(coordBack, 1.0f, 0.0f, u0 + ((i + 1) * unitSize), v0);
			producer->AddVertex(coordBack, 1.0f, -itemWidth, u0 + (i * unitSize), v0);
			producer->AddVertex(coordBack, 0.0f, -itemWidth, u0 + (i * unitSize), v2);
			producer->AddVertex(coordBack, 0.0f, 0.0f, u0 + (i * unitSize), v2);

			producer->SetNormal(0.0F, 1.0F, 0.0F);
			producer->AddVertex(0.0f, coordBack, 0.0f, u2, v0 + i * unitSize);
			producer->AddVertex(0.0f, coordBack, -itemWidth, u2, v0 + (i + 1) * unitSize);
			producer->AddVertex(1.0f, coordBack, -itemWidth, u0, v0 + (i + 1) * unitSize);
			producer->AddVertex(1.0f, coordBack, 0.0f, u0, v0 + i * unitSize);

			producer->SetNormal(0.0F, -1.0F, 0.0F);
			producer->AddVertex(0.0f, coordFront, 0.0f, u2, v0 + i * unitSize);
			producer->AddVertex(1.0f, coordFront, 0.0f, u0, v0 + i * unitSize);
			producer->AddVertex(1.0f, coordFront, -itemWidth, u0, v0 + (i + 1) * unitSize);
			producer->AddVertex(0.0f, coordFront, -itemWidth, u2, v0 + (i + 1) * unitSize);
		}

		if (env == ITEM_RENDERER_ENV_FIRST_PERSON)
		{
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(0.5f, 0.2f, 0.5f));
			model = glm::scale(model, glm::vec3(1.5f));
			model = glm::rotate(model, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(335.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, glm::vec3(1.0f / 16.0f - 1.0f, -1.0f / 16.0f, 0.0f));

			model = view * model;

			device->SetModelViewMatrix(glm::value_ptr(model));
		}
		else if (env == ITEM_RENDERER_ENV_WORLD)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(1.5f));
			model = glm::translate(model, glm::vec3(-0.125F, 0.0F, 0.25F + 1.0f / 8.0f));
			model = view * model;
			device->SetModelViewMatrix(glm::value_ptr(model));
		}

		device->SetCullMode(RI_CULL_MODE_FRONT);
	}
	else
	{
		auto block = Block::GetBlock(stack.Id);
		BlockPreviewRenderer::RenderBlock(producer, block);
		client.pResourceManager->BindTexture(E_TextureResource_Terrain, device);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.5f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-0.5f));
		model = view * model;
		device->SetModelViewMatrix(glm::value_ptr(model));
	}

	device->SetCullMode(RI_CULL_MODE_NONE);
	device->SetTexturing(true);
	device->Draw(producer->GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer->GetVertexCount());
	device->SetTexturing(false);
	device->SetCullMode(RI_CULL_MODE_BACK);
}
