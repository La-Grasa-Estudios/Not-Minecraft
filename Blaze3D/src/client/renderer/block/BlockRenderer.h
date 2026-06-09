#pragma once

class Block;
class World;
struct Chunk;
class VertexProducer;
class BlockPreviewRenderer;

class BlockRenderer
{
public:

	friend BlockPreviewRenderer;

	static void RenderBlock(Block* block, Chunk* chunk, int x, int y, int z);

private:

	static void RenderBlockGeneric(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockCross(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockTorch(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockStairs(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockDoor(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockFire(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockPortal(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockLiquid(Block* block, Chunk* chunk, int x, int y, int z);
	static void RenderBlockCrop(Block* block, Chunk* chunk, int x, int y, int z);

	static void RenderFaceWorld(VertexProducer* producer, Block* block, Chunk* chunk, int x, int y, int z, int face);
	static void RenderFaceDoorWorld(VertexProducer* producer, Block* block, Chunk* chunk, int x, int y, int z, int face);
	static void RenderFaceLiquidWorld(VertexProducer* producer, Block* block, Chunk* chunk, int x, int y, int z, int face);
	static void RenderFacePreview(VertexProducer* producer, int face);
};