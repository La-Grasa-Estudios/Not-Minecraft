#include "BlockDoor.h"

#include "common/World.h"

#include <array>

const uint8_t FACING_MASK = 0b00000011;
const uint8_t FLIP_FLAG_MASK = 0b00000100;
const uint8_t OPEN_FLAG_MASK = 0b00001000;

BlockDoor::BlockDoor(uint8_t id, uint8_t texture, DoorPart part) : Block(id, texture), m_DoorPart(part)
{
	SetOpacity(0);
	SetRenderNeighbours(true);
	SetRendersToItself(true);
	if (part == DoorPart::TOP)
	{
		SetDrop(0);
	}
}

void BlockDoor::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
{
	auto block = world->GetBlock(x, y, z);

	uint8_t meta = facing;

	if (m_DoorPart == DoorPart::BOTTOM)
	{
		if (facing == 0 || facing == 1)
		{
			auto n = world->GetBlock(x + 1, y, z);
			auto n1 = world->GetBlock(x - 1, y, z);

			if (n.Id == GetId())
			{
				auto flipTex = ~(n.Metadata & FLIP_FLAG_MASK) & FLIP_FLAG_MASK;
				meta |= flipTex;
			}
			else if (n1.Id == GetId())
			{
				auto flipTex = ~(n1.Metadata & FLIP_FLAG_MASK) & FLIP_FLAG_MASK;
				meta |= flipTex;
			}
		}
		else if (facing == 2 || facing == 3)
		{
			auto n = world->GetBlock(x, y, z - 1);
			auto n1 = world->GetBlock(x, y, z + 1);
			if (n.Id == GetId())
			{
				auto flip = ~(n.Metadata & FLIP_FLAG_MASK) & FLIP_FLAG_MASK;
				meta |= flip;
			} else if (n1.Id == GetId())
			{
				auto flip = ~(n1.Metadata & FLIP_FLAG_MASK) & FLIP_FLAG_MASK;
				meta |= flip;
			}
		}
	}
	block.Metadata = meta;

	world->SetBlock(x, y, z, block);

	if (m_DoorPart == DoorPart::BOTTOM)
	{
		world->SetBlock(x, y + 1, z, { (uint8_t)(GetId() + 1), meta });
	}
}

void BlockDoor::Tick(World* world, int x, int y, int z)
{
	if (m_DoorPart == DoorPart::BOTTOM)
	{
		if (!world->GetBlock(x, y - 1, z).GetDef()->IsFullyOpaqueBlock() || world->GetBlock(x, y + 1, z).Id != GetId() + 1)
		{
			BreakAt(world, NULL, x, y, z);
			world->NotifyNeighboursWithDelay(x, y, z, 0xFFFF, TICK_TYPE_NORMAL);
		}
	}
	if (m_DoorPart == DoorPart::TOP)
	{
		if (world->GetBlock(x, y - 1, z).Id != GetId() - 1)
		{
			BreakAt(world, NULL, x, y, z, false);
			world->NotifyNeighboursWithDelay(x, y, z, 0xFFFF, TICK_TYPE_NORMAL);
		}
	}
}

AABB BlockDoor::GetWorldBoundingBox(World* world, int x, int y, int z)
{
	auto block = world->GetBlock(x, y, z);
	const float size = 3.0f / 16.0f;

	auto face = block.Metadata & FACING_MASK;
	auto flip = block.Metadata & FLIP_FLAG_MASK;
	auto open = block.Metadata & OPEN_FLAG_MASK;

	if (open)
	{
		std::array<uint8_t, 8> lut =
		{
			3, 2, 0, 1,
			2, 3, 1, 0,
		};
		face = lut[face + (flip > 0 ? 1 : 0) * 4];
	}

	if (face == 0)
	{
		SetShape(0.0f, 0.0f, 1.0f - size, 1.0f, 1.0f, 1.0f);
	}

	if (face == 1)
	{
		SetShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, size);
	}

	if (face == 2)
	{
		SetShape(1.0f - size, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}

	if (face == 3)
	{
		SetShape(0.0f, 0.0f, 0.0f, size, 1.0f, 1.0f);
	}

	return Block::GetWorldBoundingBox(world, x, y, z);
}

bool BlockDoor::UseAt(World* world, BaseEntity* entity, int x, int y, int z)
{
	if (!world->IsServerWorld)
	{
		return true;
	}

	std::array<uint8_t, 8> lut =
	{
		2, 2, 0, 1,
		3, 3, 1, 0
	};

	auto block = world->GetBlock(x, y, z);
	auto flip = block.Metadata & FLIP_FLAG_MASK;
	auto wasOpen = block.Metadata & OPEN_FLAG_MASK;

	auto metadata = block.Metadata & (FACING_MASK | FLIP_FLAG_MASK);
	metadata |= ~(block.Metadata & OPEN_FLAG_MASK) & OPEN_FLAG_MASK;

	auto newOpen = metadata & OPEN_FLAG_MASK;

	block.Metadata = metadata;
	world->SetBlockNoNotify(x, y, z, block);

	if (!wasOpen && newOpen)
	{
		world->PlaySound("block.door.open", { x + 0.5f, y + 1, z + 0.5f }, 0.2f);
	}
	else if (wasOpen && !newOpen)
	{
		world->PlaySound("block.door.close", { x + 0.5f, y + 1, z + 0.5f }, 0.2f);
	}

	if (m_DoorPart == DoorPart::BOTTOM)
	{
		block.Id++;
		if (world->GetBlock(x, y + 1, z).Id == GetId() + 1)
		{
			world->SetBlockNoNotify(x, y + 1, z, block);
		}
	}

	if (m_DoorPart == DoorPart::TOP)
	{
		block.Id--;
		if (world->GetBlock(x, y - 1, z).Id == GetId() - 1)
		{
			world->SetBlockNoNotify(x, y - 1, z, block);
		}
	}

	return true;
}

BlockRenderType BlockDoor::GetRenderType()
{
	return BLOCK_RENDER_TYPE_DOOR;
}
