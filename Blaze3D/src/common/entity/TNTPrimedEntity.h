#pragma once

#include "common/block/Block.h"
#include "BaseEntity.h"

class TNTPrimedEntity : public BaseEntity
{
public:
	TNTPrimedEntity(World* world);
	TNTPrimedEntity(World* world, int x, int y, int z);

	void Update() override;

	void WriteToNbt(TagCompound& tag) override;
	void ReadFromNbt(TagCompound& tag) override;

	int Timer = 0;
};