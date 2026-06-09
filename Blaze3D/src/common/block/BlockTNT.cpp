#include "BlockTNT.h"

#include "common/Random.h"

#include "common/World.h"
#include "common/entity/TNTPrimedEntity.h"

BlockTNT::BlockTNT(uint8_t id) : Block(id, 0)
{
}

uint8_t BlockTNT::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	return face == 0 ? 10 : (face == 1 ? 9 : 8);
}

void BlockTNT::OnBlockExploded(World* world, int x, int y, int z)
{
	auto entity = std::make_shared<TNTPrimedEntity>(world, x, y, z);
	entity->Timer = Random::GlobalRand.NextInt(entity->Timer / 4) + entity->Timer / 8;
	world->AddEntity(entity);
}
