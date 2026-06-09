#include "EntityRendererDispatcher.h"
#include "EntityRenderer.h"

#include "ItemEntityRenderer.h"
#include "FallingSandEntityRenderer.h"
#include "ArrowEntityRenderer.h"
#include "CreatureEntityRenderer.h"
#include "CreeperEntityRenderer.h"
#include "PigZombieEntityRenderer.h"
#include "TNTPrimedEntityRenderer.h"

#include <common/entity/BaseEntity.h>
#include <common/entity/ItemEntity.h>
#include <common/entity/ArrowEntity.h>
#include <common/entity/PigEntity.h>
#include <common/entity/CowEntity.h>
#include <common/entity/ChickenEntity.h>
#include <common/entity/CreeperEntity.h>
#include <common/entity/ZombieEntity.h>
#include <common/entity/PigZombieEntity.h>
#include <common/entity/FallingSandEntity.h>
#include <common/entity/TNTPrimedEntity.h>

void EntityRendererDispatcher::Init()
{
	if (!s_Renderers.empty())
		return;

	RegisterEntityRenderer<ItemEntity, ItemEntityRenderer>();
	RegisterEntityRenderer<FallingSandEntity, FallingSandEntityRenderer>();
	RegisterEntityRenderer<ArrowEntity, ArrowEntityRenderer>();
	RegisterEntityRenderer<TNTPrimedEntity, TNTPrimedEntityRenderer>();

	// Creatures
	RegisterEntityRenderer<PigEntity, CreatureEntityRenderer>();
	RegisterEntityRenderer<CowEntity, CreatureEntityRenderer>();
	RegisterEntityRenderer<ChickenEntity, CreatureEntityRenderer>();
	RegisterEntityRenderer<ZombieEntity, CreatureEntityRenderer>();
	RegisterEntityRenderer<CreeperEntity, CreeperEntityRenderer>();
	RegisterEntityRenderer<PigZombieEntity, PigZombieEntityRenderer>();
}

void EntityRendererDispatcher::DispatchEntityRender(BaseEntity* pEntity, riDevice* pDevice)
{
	std::type_index idx = typeid(*pEntity);

	if (auto it = s_Renderers.find(idx); it != s_Renderers.end())
	{
		// pEntity->InterpolateNetworkUpdate();
		it->second->RenderType(pEntity, pDevice);
	}
	else
	{
		printf("Couldn't find entity renderer for %s\n", typeid(*pEntity).name());
	}
}
