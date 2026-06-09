#pragma once

#include "Block.h"

#include <thirdparty/glm/ext.hpp>

class BlockLiquid : public Block
{
public:

	friend BlockRenderer;

	BlockLiquid(uint8_t id, BlockLiquidType type);
	bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face) override;
	void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing) override;
	void DisplayTick(World* world, int x, int y, int z) override;
	void RandomTick(World* world, int x, int y, int z) override;
	void Tick(World* world, int x, int y, int z) override;
	void FluidTick(World* world, int x, int y, int z) override;
	int GetLiquidType() override;
	bool BlocksFlow(World* world, int x, int y, int z);
	glm::vec3 GetFlowDirection(World* world, int x, int y, int z);
	static int GetLiquidLevel(World* world, Block* block, int x, int y, int z);
	void InstantUpdate(World* world, int x, int y, int z);
	void InstantUpdateNeighbours(World* world, int x, int y, int z);
	void FlowInto(World* world, int x, int y, int z, BlockStorage newBlock);
	BlockRenderType GetRenderType() override;
private:
	float GetLevel(Chunk* chunk, int x, int y, int z);
	int CalculateFlowCost(World* world, int i, int j, int k, int l, int i1);
	void GetOptimalFlowDirections(bool flowDirection[4], int flowCost[4], World* world, int i, int j, int k);
};