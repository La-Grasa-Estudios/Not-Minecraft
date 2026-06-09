#include "ItemStack.h"
#include "item/Item.h"

#include "engine/Audio.h"

#include "nbt/NBT.h"

void ItemStack::Shrink(int32_t i)
{
    IsDirty = true;
    if (Amount < 0) {
        Amount = 0;
    }
    while (i > 0 && Amount > 0) {
        --Amount;
        --i;
    }
    if (Amount < 1) {
        Id = 0;
        Damage = 0;
    }
}

int32_t ItemStack::Expand(int32_t i)
{
    IsDirty = true;
    if (Amount <= 0) {
        Amount = 0;
    }
    if (Id == 0) {
        Damage = 0;
        return 0;
    }
    int MaxStack = 64;
    if (auto item = Item::GetItem(Id); item)
    {
        MaxStack = item->GetMaxStackSize();
    }
    while (Amount < MaxStack && i > 0)
    {
        Amount += 1;
        i -= 1;
    }
    return i;
}

bool ItemStack::HasSpace(int32_t i)
{
    if (Id == 0)
        return true;
    auto start = i;
    auto amt = Amount;
    int MaxStack = 64;
    if (auto item = Item::GetItem(Id); item)
    {
        MaxStack = item->GetMaxStackSize();
    }
    while (amt < MaxStack && i > 0)
    {
        amt += 1;
        i -= 1;
    }
    return i != start;
}

void ItemStack::Take(ItemStack& other)
{
    IsDirty = true;
    other.IsDirty = true;
    ItemStack temp = other;
    other = *this;
    *this = temp;
}

void ItemStack::TakeHalf(ItemStack& other)
{
    IsDirty = true;
    other.IsDirty = true;
    *this = other;
    auto amt = other.Amount / 2;
    Amount = amt;
    other.Shrink(Amount);
    if (amt == 0)
    {
        other = {};
        Amount = 1;
    }
}

void ItemStack::IncreaseDamage()
{
    IsDirty = true;
    auto item = Item::GetItem(Id);
    if (item)
    {
        Damage += 1;
        if (Damage >= item->GetMaxDamage())
        {
            Shrink(1);
            Damage = 0;
        }
	}
}

std::shared_ptr<TagCompound> ItemStack::WriteToTag()
{
    auto tag = std::make_shared<TagCompound>("");
    tag->SetTag(std::make_shared<TagShort>("id", static_cast<int16_t>(Id)));
    tag->SetTag(std::make_shared<TagShort>("Damage", static_cast<int16_t>(Damage)));
    tag->SetTag(std::make_shared<TagShort>("Count", static_cast<int16_t>(Amount)));
    return tag;
}

void ItemStack::ReadFromTag(TagCompound& tag)
{
    Id = tag.GetTagAs<TagShort>("id");
    Damage = tag.GetTagAs<TagShort>("Damage");
    Amount = tag.GetTagAs<TagShort>("Count");
    IsDirty = Id != 0 || Amount != 0 || Damage != 0;
}

bool ItemStack::operator==(const ItemStack& other)
{
    return other.Id == Id && other.Amount == Amount && other.Damage == Damage;
}
