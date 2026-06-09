#include "ItemFlintAndSteel.h"

#include "common/World.h"

ItemFlintAndSteel::ItemFlintAndSteel(int id) : Item(id)
{
}

bool ItemFlintAndSteel::TryPlace(BaseEntity* entity, World* world, int x, int y, int z)
{
	return false;
}
