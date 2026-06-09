#include "WorldRenderer.h"
#include "RenderHelper.h"

#include "common/World.h"
#include "common/entity/PlayerEntity.h"

#include "client/renderer/entity/EntityRendererDispatcher.h"
#include "client/renderer/VertexProducer.h"
#include "client/ResourceManager.h"
#include "client/Client.h"
#include "client/Profiler.h"

#include "engine/memory/MemoryAllocator.h"
#include "engine/RenderingInterface.h"
#include "engine/Frustum.h"

#include "block/BlockRenderer.h"

#include <algorithm>

#ifdef __wii__
#define RENDER_AREA_SIZE 64
#else
#define RENDER_AREA_SIZE 512
#endif

VertexProducer* gfx_vp = nullptr;
const float RENDER_DISTANCE = RENDER_AREA_SIZE;

static int bucketInt(int i, int j)
{
	if (i < 0)
	{
		return -((-i - 1) / j) - 1;
	}
	else
	{
		return i / j;
	}
}

void WorldRendererSection::SetPosition(int x, int y, int z, Chunk* chunk)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->chunk = chunk;
}

WorldRenderer::WorldRenderer(World* world, riContext* context)
{
	if (!gfx_vp)
	{
		gfx_vp = new VertexProducer();
	}

	static riHandle forceField = Client::GetInstance().GraphicsContext->LoadTexture("mc/textures/forcefield.png");

	m_World = world;
	m_Context = context;

	int wx = RENDER_AREA_SIZE / NODE_RENDERER_WIDTH;
	int wy = CHUNK_HEIGHT / NODE_RENDERER_HEIGHT;
	auto nbRenderers = wx * wx * wy;

	m_RenderAreaWidth = wx;
	m_RenderAreaHeight = wy;

	m_RendererCount = nbRenderers;
	m_SortedWorldRenderers.resize(nbRenderers);
	m_Renderers = new WorldRendererSection[nbRenderers];
	for (int i = 0; i < nbRenderers; i++)
	{
		m_Renderers[i].vertexBuffer[0] = NULL;
		m_Renderers[i].vertexBuffer[1] = NULL;
		m_Renderers[i].vertexCount[0] = NULL;
		m_Renderers[i].vertexCount[1] = NULL;
		m_SortedWorldRenderers[i] = &m_Renderers[i];
	}

	m_RenderX = 38921038291;
	m_RenderZ = 20984023990;

	EntityRendererDispatcher::Init();
}

WorldRenderer::~WorldRenderer()
{
	delete[] m_Renderers;
}

static WorldRendererSection* g_PendingSection = NULL;
static int g_SectionStep;

void WorldRenderer::Cleanup()
{
	for (int i = 0; i < m_SortedWorldRenderers.size(); i++) {
		auto section = m_SortedWorldRenderers[i];
		section->chunk = NULL;
		for (int layer = 0; layer < 2; layer++)
		{
			if (section->vertexCount[layer] > 0)
			{
				m_Context->DestroyBuffer(section->vertexBuffer[layer]);
			}
		}
	}
	m_SortedWorldRenderers.clear();
}

void WorldRenderer::Render(riDevice* device, float b)
{
	int allowUpdates = 2;
#ifndef _DEBUG
	allowUpdates = 8;
#endif

	Profiler::EndAndBeginSection("RenderCelestial");
	if (m_World->HasSky())
		RenderCelestialBodies(device, Client::GetInstance().UpdateTimer.a);

	device->SetFogStart(RENDER_DISTANCE / 2.0f * 0.25f);
	device->SetFogEnd(RENDER_DISTANCE / 2.0f * 0.75f);
	device->SetFog(true);
	device->SetTexturing(true);
	Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Terrain, device);

	auto nbRenderers = m_SortedWorldRenderers.size();

	Profiler::EndAndBeginSection("RenderOpaque");
	for (int i = nbRenderers - 1; i >= 0; i--) {
		WorldRendererSection* section = m_SortedWorldRenderers[i];

		section->IsCulled = true;
		section->distance = glm::distance(glm::vec2(Client::GetInstance().Player->Position.x, Client::GetInstance().Player->Position.z), glm::vec2(section->x + 8.0f, section->z + 8.0f));

		if (!section->chunk || section->chunk->GenStage != TERRAIN_GEN_COMPLETE)
		{
			continue;
		}

		if (section->distance > RENDER_DISTANCE / 2.0f) {
			continue;
		}

		section->IsCulled = Client::GetInstance().CurrentFrustum.IsBoxVisible(
			glm::vec3(section->x, section->y, section->z),
			glm::vec3(section->x + NODE_RENDERER_WIDTH, section->y + NODE_RENDERER_HEIGHT, section->z + NODE_RENDERER_WIDTH)) == false;

		if (section->needsUpdate && allowUpdates > 0 && section->chunk->GenStage == TERRAIN_GEN_COMPLETE) {
			RebuildSection(section, 0);
			RebuildSection(section, 1);
			section->needsUpdate = false;
			allowUpdates--;
		}

		if (section->vertexBuffer[0] && section->vertexCount[0] > 0 && !section->IsCulled) {
			device->Draw(section->vertexBuffer[0], 0, RI_PRIMITIVE_TYPE_QUAD, section->vertexCount[0]);
		}
	}

	auto identity = glm::mat4(1.0f);

	Profiler::EndAndBeginSection("RenderEntities");
	device->SetModelViewMatrix(glm::value_ptr(Client::GetInstance().ViewMatrix));
	RenderHelper::EnableItemLighting();

	for (int i = 0; i < m_World->m_Entities.size(); i++)
	{
		auto entity = m_World->m_Entities[i].get();
		auto minP = glm::vec3{ entity->BoundingBox.minX, entity->BoundingBox.minY, entity->BoundingBox.minZ };
		auto maxP = glm::vec3{ entity->BoundingBox.maxX, entity->BoundingBox.maxY, entity->BoundingBox.maxZ };

		if (glm::distance(Client::GetInstance().Player->Position, entity->Position) < RENDER_DISTANCE * 14.0f &&
			Client::GetInstance().CurrentFrustum.IsBoxVisible(minP, maxP))
		{
			EntityRendererDispatcher::DispatchEntityRender(entity, device);
		}
	}

	RenderHelper::DisableItemLighting();

	device->SetModelViewMatrix(glm::value_ptr(Client::GetInstance().ViewMatrix));

	Profiler::EndAndBeginSection("Sort");
	std::sort(m_SortedWorldRenderers.begin(), m_SortedWorldRenderers.end(), [](WorldRendererSection* a, WorldRendererSection* b) {
		return a->distance > b->distance;
	});

	if (m_World->HasSky())
	{
		Profiler::EndAndBeginSection("RenderClouds");
		device->SetCullMode(RI_CULL_MODE_NONE);
		device->SetTexturing(true);

		Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Clouds, device);

		float uvOffsetX = Client::GetInstance().Player->Position.x;
		float uvOffsetZ = Client::GetInstance().Player->Position.z;

		gfx_vp->Reset();

		b = glm::clamp(1 - b, 0.2f, 1.0f);

		gfx_vp->SetColor(1.0f - b, 1.0f - b, 1.0f - b * 0.95f, 0.75f);
		gfx_vp->SetTranslation(uvOffsetX, 0.0f, uvOffsetZ);

		uvOffsetX += sysTime() * 0.4f;

		uvOffsetX = uvOffsetX / 8.0f / 256.0f;
		uvOffsetZ = uvOffsetZ / 8.0f / 256.0f;

		uvOffsetX = fmod(uvOffsetX, 1.0f);
		uvOffsetZ = fmod(uvOffsetZ, 1.0f);

		gfx_vp->AddVertex(1024.0f, 128.0f, 1024.0f, 1.0f + uvOffsetX, 1.0f + uvOffsetZ);
		gfx_vp->AddVertex(1024.0f, 128.0f, -1024.0f, 1.0f + uvOffsetX, 0.0f + uvOffsetZ);
		gfx_vp->AddVertex(-1024.0f, 128.0f, -1024.0f, 0.0f + uvOffsetX, 0.0f + uvOffsetZ);
		gfx_vp->AddVertex(-1024.0f, 128.0f, 1024.0f, 0.0f + uvOffsetX, 1.0f + uvOffsetZ);

		device->SetFogEnd(400.0f);
		device->Draw(gfx_vp->GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, gfx_vp->GetVertexCount());
	}

	device->SetTexturing(true);
	Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Terrain, device);
	device->SetFogEnd(RENDER_DISTANCE / 2.0f * 0.75f);

	Profiler::EndAndBeginSection("RenderTranslucent");
	for (int i = 0; i < nbRenderers; i++) {
		WorldRendererSection* section = m_SortedWorldRenderers[i];

		if (section->IsCulled)
		{
			continue;
		}

		if (section->vertexBuffer[1] && section->vertexCount[1] > 0) {
			device->Draw(section->vertexBuffer[1], 0, RI_PRIMITIVE_TYPE_QUAD, section->vertexCount[1]);
		}
	}

	device->SetCullMode(RI_CULL_MODE_BACK);

	device->SetFog(false);
}

void WorldRenderer::RebuildSection(WorldRendererSection* section, uint8_t layer)
{
	gfx_vp->Reset();

	for (int zz = 0; zz < NODE_RENDERER_WIDTH; zz++) {
		for (int xx = 0; xx < NODE_RENDERER_WIDTH; xx++) {
			for (int yy = 0; yy < NODE_RENDERER_HEIGHT; yy++) {
				int x = section->x + xx;
				int y = section->y + yy;
				int z = section->z + zz;

				BlockStorage block = section->chunk->GetBlock(x, y, z);

				if (block.Id != 0 && block.GetDef()->GetRenderLayer() == layer) {
					// Moving all render code from common game to client
					BlockRenderer::RenderBlock(block.GetDef(), section->chunk, x, y, z);
					// block.GetDef()->RenderWorld(section->chunk, x, y, z);
				}
			}
		}
	}

	if (section->vertexBuffer[layer])
	{
		m_Context->DestroyBuffer(section->vertexBuffer[layer]);
		section->vertexBuffer[layer] = nullptr;
	}

	if (gfx_vp->GetVertexCount() == 0)
	{
		section->vertexCount[layer] = 0;
		section->vertexBuffer[layer] = nullptr;
		return;
	}

	section->vertexCount[layer] = gfx_vp->GetVertexCount();
	section->vertexBuffer[layer] = gfx_vp->CreateVertexBuffer(m_Context);
}

int WorldRenderer::GetRendererIndex(int x, int y, int z)
{
	return (y * m_RenderAreaHeight + z) * m_RenderAreaWidth + x;
}

void WorldRenderer::SetDirty(int blockX, int blockY, int blockZ, bool forceChunkReload) {
	int minX = bucketInt(blockX, 16);
	int minY = bucketInt(blockY, 16);
	int minZ = bucketInt(blockZ, 16);

	int maxX = bucketInt(blockX + 1, 16);
	int maxY = bucketInt(blockY + 1, 16);
	int maxZ = bucketInt(blockZ + 1, 16);

	for (int xIdx = minX; xIdx <= maxX; ++xIdx) {

		int wrapX = xIdx % m_RenderAreaWidth;
		if (wrapX < 0) {
			wrapX += m_RenderAreaWidth;
		}

		for (int yIdx = minY; yIdx <= maxY; ++yIdx) {

			int wrapY = yIdx % m_RenderAreaHeight;
			if (wrapY < 0) {
				wrapY += m_RenderAreaHeight;
			}

			for (int zIdx = minZ; zIdx <= maxZ; ++zIdx) {

				int wrapZ = zIdx % m_RenderAreaWidth;
				if (wrapZ < 0) {
					wrapZ += m_RenderAreaWidth;
				}

				int idx = (wrapZ * m_RenderAreaHeight + wrapY) * m_RenderAreaWidth + wrapX;
				auto& renderer = m_Renderers[idx];

				renderer.needsUpdate = true;

				if (forceChunkReload) {
					renderer.chunk = m_World->GetChunk(renderer.x, renderer.z);
				}
			}
		}
	}
}
	
void WorldRenderer::SetColumnDirty(int x, int z)
{
	for (int y = 0; y < m_RenderAreaHeight; y++)
	{
		SetDirty(x, y * NODE_RENDERER_HEIGHT, z, true);
	}
}

void WorldRenderer::SetAllDirty()
{
	for (int i = 0; i < m_RendererCount; i++)
	{
		m_Renderers[i].needsUpdate = true;
	}
}

void WorldRenderer::OnChunkUnload(int x, int z)
{

}

void WorldRenderer::MarkNewPositionForRender(int playerBlockX, int playerBlockZ)
{
	if (!m_World || ((m_RenderX == playerBlockX) && (m_RenderZ == playerBlockZ))) {
		return;
	}

	m_RenderX = playerBlockX;
	m_RenderZ = playerBlockZ;

	const int renderSize = m_RenderAreaWidth * 16;
	const int halfRenderSize = renderSize / 2;

	for (int xIdx = 0; xIdx < m_RenderAreaWidth; ++xIdx) {

		int targetX = xIdx * 16;

		int distanceToPlayerX = (targetX + halfRenderSize) - m_RenderX;
		if (distanceToPlayerX < 0) {
			distanceToPlayerX -= renderSize - 1;
		}

		int xWrap = distanceToPlayerX / renderSize;

		targetX -= xWrap * renderSize;

		for (int zIdx = 0; zIdx < m_RenderAreaWidth; ++zIdx) {

			int targetZ = zIdx * 16;

			int playerDistZ = (targetZ + halfRenderSize) - m_RenderZ;
			if (playerDistZ < 0) {
				playerDistZ -= renderSize - 1;
			}

			int zWrap = playerDistZ / renderSize;

			targetZ -= zWrap * renderSize;

			auto chunk = m_World->GetChunk(targetX, targetZ);

			for (int yIdx = 0; yIdx < m_RenderAreaHeight; ++yIdx) {

				int targetY = yIdx * 16;

				int idx = (zIdx * m_RenderAreaHeight + yIdx) * m_RenderAreaWidth + xIdx;
				auto& renderer = m_Renderers[idx];

				if (renderer.x == targetX &&
					renderer.y == targetY &&
					renderer.z == targetZ) {
					continue;
				}

				renderer.SetPosition(targetX, targetY, targetZ, chunk);
				renderer.needsUpdate = true;
			}
		}
	}
}

void WorldRenderer::RenderCelestialBodies(riDevice* device, float partialTick)
{
	static riHandle sunTexture = m_Context->LoadTexture("mc/textures/terrain/sun.png");
	static riHandle moonTexture = m_Context->LoadTexture("mc/textures/terrain/moon.png");

	m_Context->SetTextureFilter(sunTexture, RI_FILTER_NEAREST);
	m_Context->SetTextureFilter(moonTexture, RI_FILTER_NEAREST);

	device->SetDepthTest(false, RI_COMPARISON_MODE_LESS_EQUAL);

	auto& client = Client::GetInstance();

	float celestialAngle = m_World->GetCelestialAngle(partialTick);
	float rot = celestialAngle * 360.0f;
	auto view = glm::mat4(glm::mat3(client.ViewMatrix)); // Remove translation

	auto model = glm::rotate(glm::mat4(1.0f), glm::radians(rot), glm::vec3(1.0f, 0.0f, 0.0f));

	view = view * model;

	device->PushMatrixStack();
	device->SetModelViewMatrix(glm::value_ptr(view));
	device->SetFog(false);
	device->SetCullMode(RI_CULL_MODE_NONE);

	float size = 30.0f;

	gfx_vp->Reset();
	gfx_vp->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	gfx_vp->AddVertex(-size, 100.0f, -size, 0.0f, 0.0f);
	gfx_vp->AddVertex(size, 100.0f, -size, 1.0f, 0.0f);
	gfx_vp->AddVertex(size, 100.0f, size, 1.0f, 1.0f);
	gfx_vp->AddVertex(-size, 100.0f, size, 0.0f, 1.0f);

	device->SetBlendMode(BLEND_MODE_ONE, BLEND_MODE_ONE);
	device->SetTexturing(true);
	device->SetTexture(sunTexture, 0);
	device->Draw(gfx_vp->GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, gfx_vp->GetVertexCount());

	gfx_vp->Reset();
	gfx_vp->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	size = 20.0f;

	gfx_vp->AddVertex(-size, -100.0f, -size, 0.0f, 0.0f);
	gfx_vp->AddVertex(size, -100.0f, -size, 1.0f, 0.0f);
	gfx_vp->AddVertex(size, -100.0f, size, 1.0f, 1.0f);
	gfx_vp->AddVertex(-size, -100.0f, size, 0.0f, 1.0f);

	device->SetTexture(moonTexture, 0);
	device->Draw(gfx_vp->GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, gfx_vp->GetVertexCount());
	device->SetTexturing(false);
	device->SetBlendMode(BLEND_MODE_SRC_ALPHA, BLEND_MODE_INV_SRC_ALPHA);

	device->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);
	device->SetCullMode(RI_CULL_MODE_BACK);
	device->PopMatrixStack();
}
