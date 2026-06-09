#pragma once

#include "common/block/Block.h"
#include "BaseEntity.h"

BEGIN_ENTITY_DECLARATION(FallingSandEntity)
public:
	FallingSandEntity(World* world);
	FallingSandEntity(World* world, int x, int y, int z);

	void Update() override;

	void WriteToNbt(TagCompound& tag) override;
	void ReadFromNbt(TagCompound& tag) override;

	BlockStorage Block;
END_ENTITY_DECLARATION