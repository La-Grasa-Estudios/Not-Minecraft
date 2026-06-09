#pragma once

class Block;
class World;
class VertexProducer;

class BlockPreviewRenderer
{
public:

	static void RenderBlock(VertexProducer* producer, Block* block);

private:

	static void RenderBlockGeneric(VertexProducer* producer, Block* block);
	static void RenderBlockStairs(VertexProducer* producer, Block* block);
	static void RenderBlockPortal(VertexProducer* producer, Block* block);
	static void RenderBlockCrop(VertexProducer* producer, Block* block);

	static void RenderFacePreview(VertexProducer* producer, Block* block, int face);
};