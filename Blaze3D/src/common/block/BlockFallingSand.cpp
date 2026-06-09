#include "BlockFallingSand.h"

#include <common/World.h>
#include "common/entity/FallingSandEntity.h"

BlockFallingSand::BlockFallingSand(uint8_t id, uint8_t texture) : Block(id, texture)
{
}

void BlockFallingSand::Tick(World* world, int x, int y, int z)
{
    auto below = world->GetBlock(x, y - 1, z).GetDef();

    if (!below->IsSolid() && world->CanAddEntity())
    {
        auto entity = std::make_shared<FallingSandEntity>(world, x, y, z);
        world->AddEntity(entity);
    }
}

void BlockFallingSand::RandomTick(World* world, int x, int y, int z)
{

}