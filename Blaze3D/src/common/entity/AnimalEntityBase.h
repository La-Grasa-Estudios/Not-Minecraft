#pragma once

#include "CreatureEntity.h"

class World;

class AnimalEntityBase : public CreatureEntity
{
public:
	AnimalEntityBase(World* world);
};