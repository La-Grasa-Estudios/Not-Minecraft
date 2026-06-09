#include "ItemBow.h"

#include "engine/Audio.h"

#include "common/World.h"
#include "common/entity/ArrowEntity.h"

ItemBow::ItemBow(int id) : Item(id)
{

}

bool ItemBow::TryUseOnEntity(BaseEntity* entity, World* world, int x, int y, int z)
{
	if (entity->Inventory.ConsumeItemWithId(Item::Arrow->GetId()))
	{
		auto arrow = std::make_shared<ArrowEntity>(world, entity, entity->Position);
		auto forward = entity->GetForward();
		arrow->IsImportant = true;
		arrow->SetHeading(forward.x, forward.y, forward.z, 1.5f, 1.0f);
		world->AddEntity(arrow);
		world->PlaySound("random.bow", entity->Position, 0.3f);
	}

	// return true; shit, it removes the item if i return true
    return false;
}
