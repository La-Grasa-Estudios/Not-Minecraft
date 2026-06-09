#pragma once

#include "engine/Common.h"

#include <vector>

#define NODE_RENDERER_WIDTH 16
#define NODE_RENDERER_HEIGHT 16

class World;
struct Chunk;
class riDevice;
class riContext;

struct WorldRendererSection
{
	int x;
	int y;
	int z;
	float distance;
	Chunk* chunk;

	void SetPosition(int x, int y, int z, Chunk* chunk);

	bool needsUpdate;
	bool IsCulled = false;
	riHandle vertexBuffer[2];
	uint32_t vertexCount[2];
};

class WorldRenderer
{
public:
	WorldRenderer(World* world, riContext* context);
	~WorldRenderer();

	void Cleanup();
	void Render(riDevice* device, float b);
	void RebuildSection(WorldRendererSection* section, uint8_t layer);
	int GetRendererIndex(int x, int y, int z);
	void SetDirty(int x, int y, int z, bool updateChunk = false);
	void SetColumnDirty(int x, int z);
	void SetAllDirty();
	void OnChunkUnload(int x, int z);
	void MarkNewPositionForRender(int x, int z);
private:

	void RenderCelestialBodies(riDevice* device, float partialTick);

	World* m_World;
	WorldRendererSection* m_Renderers;
	std::vector<WorldRendererSection*> m_SortedWorldRenderers;
	uint32_t m_RendererCount;
	uint32_t m_RenderAreaWidth = 0;
	uint32_t m_RenderAreaHeight = 0;
	int32_t m_RenderX = 0;
	int32_t m_RenderZ = 0;
	riContext* m_Context;
};