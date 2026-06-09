#include "BlockLiquid.h"

#include "common/World.h"

#include "engine/Audio.h"
#include "common/Random.h"

#include <cmath>

constexpr uint8_t WATER_SOURCE_LEVEL = 8;
constexpr uint8_t WATER_FALLING = 16;

BlockLiquid::BlockLiquid(uint8_t id, BlockLiquidType type) : Block(id, 0)
{
	SetLayer(1);
	SetTexture(13, 12);

	if (type == LIQUID_TYPE_LAVA)
	{
		SetTexture(13, 14);
		SetLightEmitted(15);
	}

	SetShape(0.0f, 0.0f, 0.0f, 1.0f, 0.9f, 1.0f);
	SetIsSolid(false);
	SetIsSolidToRaycast(false);
	SetHardness(99999.0f);
	SetDrop(BLOCK_AIR);
	SetRenderNeighbours(true);
	SetMakesSound(false);
	SetIsReplaceable(true);
	SetOpacity(2);
	SetCreatesBreakParticles(false);
	SetIsOpaque(false);
	SetIsReplaceableByPlayer(true);
	SetResistance(100000.0f);
}

bool BlockLiquid::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	auto block = chunk->GetBlockGlobal(x, y, z).GetDef();
	return (face == 1 && block != this) || ((block->GetId() == 0 || block->GetRenderNeighbours()) && block != this);
}

void BlockLiquid::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
{
	auto b = world->GetBlock(x, y, z);
	b.Metadata = WATER_SOURCE_LEVEL;
	world->SetBlockNoNotifyNoDirty(x, y, z, b);
}

void BlockLiquid::DisplayTick(World* world, int x, int y, int z)
{
	if (GetLiquidType() == LIQUID_TYPE_LAVA)
	{
		if (world->GetBlock(x, y + 1, z).Id == 0 && Random::GlobalRand.NextInt(100) == 0)
		{
			world->PlaySound("misc.lava.pop", glm::vec3(x, y, z) + glm::vec3(0.5f), 0.1f);
		}
	}
}

void BlockLiquid::RandomTick(World* world, int x, int y, int z)
{
}

void BlockLiquid::FluidTick(World* world, int x, int y, int z)
{
	int delayTicks = (m_Id == BLOCK_LAVA) ? 30 : 5;
	auto self = world->GetBlock(x, y, z);

	bool keepAlive = self.Metadata >= WATER_SOURCE_LEVEL;

	uint8_t highestNeighbour = 0;
	int nbNeighborSources = 0;

	int falloff = m_Id == BLOCK_LAVA ? 2 : 1;

	if (world->GetDimensionID() == -1)
	{
		delayTicks = 5;
		falloff = 1;
	}

	for (int i = 0; i < 4 && self.Metadata < WATER_SOURCE_LEVEL; i++)
	{
		int xx = x;
		int zz = z;
		if (i == 0)
			zz -= 1;
		if (i == 1)
			zz += 1;
		if (i == 2)
			xx -= 1;
		if (i == 3)
			xx += 1;
		auto neighbour = world->GetBlock(xx, y, zz);
		if (neighbour.GetDef() == this)
		{
			if (neighbour.Metadata > highestNeighbour)
			{
				highestNeighbour = neighbour.Metadata;
			}
			if (neighbour.Metadata > self.Metadata)
			{
				keepAlive = true;
			}
			if (neighbour.Metadata == WATER_SOURCE_LEVEL)
			{
				nbNeighborSources++;
			}
		}
	}

	if (highestNeighbour > WATER_SOURCE_LEVEL - falloff)
	{
		highestNeighbour = WATER_SOURCE_LEVEL - falloff;
	}

	if (self.Metadata == WATER_FALLING)
	{
		keepAlive = world->GetBlock(x, y + 1, z).GetDef() == this;
	}

	if (nbNeighborSources >= 2 && self.Metadata < WATER_SOURCE_LEVEL)
	{
		self.Metadata = WATER_SOURCE_LEVEL;
		world->SetBlockNoNotify(x, y, z, self);
		world->NotifyNeighboursWithDelay(x, y, z, delayTicks, TICK_TYPE_LIQUID);
		return;
	}

	if (!keepAlive)
	{
		if (highestNeighbour > falloff)
		{
			self.Metadata = highestNeighbour - falloff;
			world->SetBlockNoNotify(x, y, z, self);
			world->NotifyNeighboursWithDelay(x, y, z, delayTicks, TICK_TYPE_LIQUID);
		}
		else
		{
			world->SetBlockNoNotify(x, y, z, BLOCK_AIR);
			world->NotifyNeighboursWithDelay(x, y, z, delayTicks, TICK_TYPE_LIQUID);
			return;
		}
	}

	if (self.Metadata > 0)
	{
		auto below = world->GetBlock(x, y - 1, z);
		auto belowCannotFlow = BlocksFlow(world, x, y - 1, z);
		uint8_t nextLevel = self.Metadata - falloff;
		if (!belowCannotFlow)
		{
			if (!belowCannotFlow)
			{
				if (below.GetDef() != this || below.Metadata < WATER_SOURCE_LEVEL)
				{
					FlowInto(world, x, y - 1, z, { GetId(), WATER_FALLING });
					world->ScheduleUpdate(x, y, z, delayTicks, TICK_TYPE_LIQUID);
				}
			}
		}
		else
		{
			if (nextLevel == 0)
			{
				return;
			}
			if (self.Metadata == WATER_FALLING)
			{
				nextLevel = WATER_SOURCE_LEVEL - falloff;

				if (!belowCannotFlow)
				{
					return;
				}
			}
			bool optimalFlowPath[4];
			int flowCosts[4];
			GetOptimalFlowDirections(optimalFlowPath, flowCosts, world, x, y, z);

			for (int i = 0; i < 4; i++)
			{
				int xx = x;
				int zz = z;
				if (i == 0)
					xx -= 1;
				if (i == 1)
					xx += 1;
				if (i == 2)
					zz -= 1;
				if (i == 3)
					zz += 1;
				if (optimalFlowPath[i])
				{
					auto neighbour = world->GetBlock(xx, y, zz);
					auto neighbourBlocksFlow = BlocksFlow(world, xx, y, zz);
					if (neighbour.GetDef() == this)
					{
						if (neighbour.Metadata < nextLevel)
						{
							neighbour.Metadata = nextLevel;
							world->SetBlockNoNotify(xx, y, zz, neighbour);
							world->ScheduleUpdate(xx, y, zz, delayTicks, 1);
						}
					}
					else if (!neighbourBlocksFlow)
					{
						FlowInto(world, xx, y, zz, { GetId(), nextLevel });
						world->ScheduleUpdate(xx, y, zz, delayTicks, 1);
					}
				}
			}
		}
	}
}

int BlockLiquid::GetLiquidType()
{
	return GetId() == BLOCK_WATER ? 1 : (GetId() == BLOCK_LAVA ? 2 : 0);
}

void BlockLiquid::Tick(World* world, int x, int y, int z)
{
	int ticks = m_Id == BLOCK_LAVA ? 30 : 5;
	if (world->GetDimensionID() == -1)
	{
		ticks = 5;
	}
	world->ScheduleUpdate(x, y, z, ticks, TICK_TYPE_LIQUID);
}

bool BlockLiquid::BlocksFlow(World* world, int x, int y, int z)
{
	auto b = world->GetBlock(x, y, z).GetDef();
	return !b->IsReplaceable();
}

glm::vec3 BlockLiquid::GetFlowDirection(World* world, int x, int y, int z)
{
	float vx = 0.0;
	float vy = 0.0;
	float vz = 0.0;
	const int effectiveFlowDecay = glm::min(GetLiquidLevel(world, this, x, y, z), 8);
	if (effectiveFlowDecay == -999) {
		return glm::vec3(vx, vy, vz);
	}
	for (int i = 0; i < 4; ++i) {
		int n = x;
		int n2 = z;
		if (i == 0) {
			--n;
		}
		if (i == 1) {
			--n2;
		}
		if (i == 2) {
			++n;
		}
		if (i == 3) {
			++n2;
		}
		int n3 = glm::min(GetLiquidLevel(world, this, n, y, n2), 8);
		if (n3 == -999) continue;
		if (n3 < 0) {
			n3 = glm::min(GetLiquidLevel(world, this, n, y - 1, n2), 8);
			if (n3 >= 0) {
				const int n4 = n3 - (effectiveFlowDecay - 9);
				vx -= (n - x) * n4;
				vz -= (n2 - z) * n4;
			}
		}
		else if (n3 >= 0) {
			const int n4 = n3 - effectiveFlowDecay;
			vx += (n - x) * n4;
			vz += (n2 - z) * n4;
		}
	}
	const float length2 = glm::sqrt(vx * vx + vy * vy + vz * vz);
	vx /= length2;
	vy /= length2;
	vz /= length2;
	if ((vx == 0.0 && vz == 0.0) || vx > 2.0 || vz > 2.0 || std::isnan(vx) || std::isnan(vz)) {
		return glm::vec3(0.0, 0.0, 0.0);
	}
	return glm::vec3(-vx, vy, -vz);
}

int BlockLiquid::GetLiquidLevel(World* world, Block* block, int x, int y, int z)
{
	auto b = world->GetBlock(x, y, z);
	if (b.GetDef() != block)
		return -999;
	auto level = b.Metadata;
	if (level == WATER_FALLING) level = 9;
	return level;
}

void BlockLiquid::InstantUpdate(World* world, int x, int y, int z)
{
	auto self = world->GetBlock(x, y, z);

	bool keepAlive = self.Metadata >= WATER_SOURCE_LEVEL;

	uint8_t highestNeighbour = 0;
	int nbNeighborSources = 0;

	int delayTicks = m_Id == BLOCK_LAVA ? 30 : 5;
	int falloff = m_Id == BLOCK_LAVA ? 2 : 1;

	if (world->GetDimensionID() == -1)
	{
		delayTicks = 5;
		falloff = 1;
	}

	for (int i = 0; i < 4 && self.Metadata < WATER_SOURCE_LEVEL; i++)
	{
		int xx = x;
		int zz = z;
		if (i == 0)
			zz -= 1;
		if (i == 1)
			zz += 1;
		if (i == 2)
			xx -= 1;
		if (i == 3)
			xx += 1;
		auto neighbour = world->GetBlock(xx, y, zz);
		if (neighbour.GetDef() == this)
		{
			if (neighbour.Metadata > highestNeighbour)
			{
				highestNeighbour = neighbour.Metadata;
			}
			if (neighbour.Metadata > self.Metadata)
			{
				keepAlive = true;
			}
			if (neighbour.Metadata == WATER_SOURCE_LEVEL)
			{
				nbNeighborSources++;
			}
		}
	}

	if (highestNeighbour > WATER_SOURCE_LEVEL - falloff)
	{
		highestNeighbour = WATER_SOURCE_LEVEL - falloff;
	}

	if (self.Metadata == WATER_FALLING)
	{
		keepAlive = world->GetBlock(x, y + 1, z).GetDef() == this;
	}

	if (nbNeighborSources >= 2 && self.Metadata < WATER_SOURCE_LEVEL)
	{
		self.Metadata = WATER_SOURCE_LEVEL;
		world->SetBlockNoNotify(x, y, z, self);
		InstantUpdateNeighbours(world, x, y, z);
		return;
	}

	if (!keepAlive)
	{
		if (highestNeighbour > falloff)
		{
			self.Metadata = highestNeighbour - falloff;
			world->SetBlockNoNotify(x, y, z, self);
			InstantUpdateNeighbours(world, x, y, z);
		}
		else
		{
			world->SetBlockNoNotify(x, y, z, BLOCK_AIR);
			InstantUpdateNeighbours(world, x, y, z);
			return;
		}
	}

	if (self.Metadata > 0)
	{
		auto below = world->GetBlock(x, y - 1, z);
		auto belowCannotFlow = BlocksFlow(world, x, y - 1, z);
		uint8_t nextLevel = self.Metadata - falloff;
		if (!belowCannotFlow)
		{
			if (!belowCannotFlow)
			{
				if (below.GetDef() != this || below.Metadata < WATER_SOURCE_LEVEL)
				{
					below.GetDef()->BreakAt(world, NULL, x, y - 1, z, true);
					FlowInto(world, x, y - 1, z, { GetId(), WATER_FALLING });
					InstantUpdate(world, x, y - 1, z);
				}
			}
		}
		else
		{
			if (nextLevel == 0)
			{
				return;
			}
			if (self.Metadata == WATER_FALLING)
			{
				nextLevel = WATER_SOURCE_LEVEL - falloff;

				if (!belowCannotFlow)
				{
					return;
				}
			}
			bool optimalFlowPath[4];
			int flowCosts[4];
			GetOptimalFlowDirections(optimalFlowPath, flowCosts, world, x, y, z);

			for (int i = 0; i < 4; i++)
			{
				int xx = x;
				int zz = z;
				if (i == 0)
					xx -= 1;
				if (i == 1)
					xx += 1;
				if (i == 2)
					zz -= 1;
				if (i == 3)
					zz += 1;
				if (optimalFlowPath[i])
				{
					auto neighbour = world->GetBlock(xx, y, zz);
					auto neighbourBlocksFlow = BlocksFlow(world, xx, y, zz);
					if (neighbour.GetDef() == this)
					{
						if (neighbour.Metadata < nextLevel)
						{
							neighbour.Metadata = nextLevel;
							world->SetBlockNoNotify(xx, y, zz, neighbour);
							InstantUpdate(world, xx, y, zz);
						}
					}
					else if (!neighbourBlocksFlow)
					{
						neighbour.GetDef()->BreakAt(world, NULL, xx, y, zz, true);
						FlowInto(world, xx, y, zz, { GetId(), nextLevel });
						InstantUpdate(world, xx, y, zz);
					}
				}
			}
		}
	}
}

void BlockLiquid::InstantUpdateNeighbours(World* world, int x, int y, int z)
{
	InstantUpdate(world, x - 1, y, z);
	InstantUpdate(world, x + 1, y, z);
	InstantUpdate(world, x, y - 1, z);
	InstantUpdate(world, x, y + 1, z);
	InstantUpdate(world, x, y, z - 1);
	InstantUpdate(world, x, y, z + 1);
}

void BlockLiquid::FlowInto(World* world, int x, int y, int z, BlockStorage newBlock)
{
	auto blockAt = world->GetBlock(x, y, z).GetDef();
	if (blockAt->GetLiquidType() != 0 && blockAt->GetLiquidType() != GetLiquidType())
	{
		newBlock.Id = BLOCK_OBSIDIAN;
		newBlock.Metadata = 0;
	}
	else
	{
		blockAt->BreakAt(world, NULL, x, y, z, true);
	}
	world->SetBlockNoNotify(x, y, z, newBlock);
}

BlockRenderType BlockLiquid::GetRenderType()
{
	return BLOCK_RENDER_TYPE_LIQUID;
}

float BlockLiquid::GetLevel(Chunk* chunk, int x, int y, int z)
{
	int n = 0;
	float ans = 0.0;

	for (int i = 0; i < 4; ++i) {
		const int xx = x - (i & 0x1);
		const int zz = z - (i >> 1 & 0x1);
		if (this == chunk->GetBlockGlobal(xx, y - 1, zz).GetDef()) {
			const int b = GetLiquidLevel(chunk->ChunkWorld, this, xx, y - 1, zz);
			if (b == 9) {
				n++;
			}
		}
		if (this == chunk->GetBlockGlobal(xx, y + 1, zz).GetDef()) {
			return 1.0;
		}

		if (this == chunk->GetBlockGlobal(xx, y, zz).GetDef()) {
			const int b = GetLiquidLevel(chunk->ChunkWorld, this, xx, y, zz);
			float anso = b / 9.0f;
			int no = 1;
			if (b >= 8 || b == 0) {
				anso *= 11.0;
				no *= 11;
			}
			ans += anso;
			n += no;
		}
	}
	return ans / n;
}

int BlockLiquid::CalculateFlowCost(World* world, int i, int j, int k, int l, int i1)
{
	int j1 = 1000;
	for (int k1 = 0; k1 < 4; k1++)
	{
		if (k1 == 0 && i1 == 1 || k1 == 1 && i1 == 0 || k1 == 2 && i1 == 3 || k1 == 3 && i1 == 2)
		{
			continue;
		}
		int l1 = i;
		int i2 = j;
		int j2 = k;
		if (k1 == 0)
		{
			l1--;
		}
		if (k1 == 1)
		{
			l1++;
		}
		if (k1 == 2)
		{
			j2--;
		}
		if (k1 == 3)
		{
			j2++;
		}
		if (BlocksFlow(world, l1, i2, j2))
		{
			continue;
		}
		if (!BlocksFlow(world, l1, i2 - 1, j2))
		{
			return l;
		}
		if (l >= 4)
		{
			continue;
		}
		int k2 = CalculateFlowCost(world, l1, i2, j2, l + 1, k1);
		if (k2 < j1)
		{
			j1 = k2;
		}
	}

	return j1;
}

void BlockLiquid::GetOptimalFlowDirections(bool flowDirection[4], int flowCost[4], World* world, int i, int j, int k)
{
	for (int l = 0; l < 4; l++)
	{
		flowCost[l] = 1000;
		int j1 = i;
		int i2 = j;
		int j2 = k;
		if (l == 0)
		{
			j1--;
		}
		if (l == 1)
		{
			j1++;
		}
		if (l == 2)
		{
			j2--;
		}
		if (l == 3)
		{
			j2++;
		}
		auto neigh = world->GetBlock(j1, i2, j2);
		if (BlocksFlow(world, j1, i2, j2) || (neigh.GetDef() == this && neigh.Metadata == WATER_SOURCE_LEVEL))
		{
			continue;
		}
		if (!BlocksFlow(world, j1, i2 - 1, j2))
		{
			flowCost[l] = 0;
		}
		else
		{
			flowCost[l] = CalculateFlowCost(world, j1, i2, j2, 1, l);
		}
	}

	int i1 = flowCost[0];
	for (int k1 = 1; k1 < 4; k1++)
	{
		if (flowCost[k1] < i1)
		{
			i1 = flowCost[k1];
		}
	}

	for (int l1 = 0; l1 < 4; l1++)
	{
		flowDirection[l1] = flowCost[l1] == i1;
	}

	return;
}