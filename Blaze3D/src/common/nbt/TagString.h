#pragma once

#include "NBTBase.h"

#include <string>

class TagString : public NBTBase
{
public:
	TagString() = default;
	TagString(std::string_view name, std::string_view string);
	NbtTagType GetTagType();

	operator std::string();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	std::string m_Value;

};