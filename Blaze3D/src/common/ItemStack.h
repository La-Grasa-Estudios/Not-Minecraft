#pragma once

#include <cstdint>
#include <memory>

class TagCompound;

class ItemStack
{
public:
	uint32_t Id;
	int32_t Amount;
	int32_t Damage;
	bool IsDirty;
	void Shrink(int32_t amount);
	// Returns the amount left if the stack does not have the capacity
	int32_t Expand(int32_t amount);
	bool HasSpace(int32_t amount);
	void Take(ItemStack& other);
	void TakeHalf(ItemStack& other);
	void IncreaseDamage();
	std::shared_ptr<TagCompound> WriteToTag();
	void ReadFromTag(TagCompound& tag);
	bool operator ==(const ItemStack& other);
};