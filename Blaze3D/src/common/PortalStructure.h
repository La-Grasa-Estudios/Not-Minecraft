#pragma once

#include "World.h"

class PortalStructure
{
public:
	static bool IsPortalValid(World* world, int x, int y, int z);
	static void FillPortal(World* world, int x, int y, int z);
	static int GetPortalDirection(World* world, int x, int y, int z, int& outOriginX, int& outOriginY, int& outOriginZ);
};