#pragma once

#include "NBTBase.h"

#include <vector>

class TagByteArray : public NBTBase
{
public:
	TagByteArray() = default;
	TagByteArray(std::string_view name, const std::vector<int8_t>& array);
	NbtTagType GetTagType();

	size_t Size();
	std::vector<int8_t> Values;

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);
};