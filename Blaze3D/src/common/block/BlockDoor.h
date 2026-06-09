#pragma once

#include "Block.h"

enum class DoorPart
{
	BOTTOM,
	TOP,
};

class BlockDoor : public Block
{
public:
	friend BlockRenderer;
	BlockDoor(uint8_t id, uint8_t texture, DoorPart part);
	void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing) override;
	void Tick(World* world, int x, int y, int z) override;
	AABB GetWorldBoundingBox(World* world, int x, int y, int z);
	bool UseAt(World* world, BaseEntity* entity, int x, int y, int z) override;
	BlockRenderType GetRenderType() override;
private:
	DoorPart m_DoorPart;
};