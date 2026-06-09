#pragma once

#include "NBTBase.h"

#include <vector>

class TagList : public NBTBase
{
public:
	TagList() = default;
	TagList(std::string_view name, NbtTagType type);
	NbtTagType GetTagType();

	void Add(std::shared_ptr<NBTBase> tag);
	std::shared_ptr<NBTBase> Get(size_t index);
	size_t Size();

	template<typename T>
	T& GetAs(size_t index)
	{
		static_assert(std::is_base_of<NBTBase, T>::value, "Invalid Tag provided");
		return *static_cast<T*>(Get(index).get());
	}

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	std::vector<std::shared_ptr<NBTBase>> m_Values;
	NbtTagType m_Type;
};