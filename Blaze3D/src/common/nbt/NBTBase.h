#pragma once

#include "BigEndianDataStream.h"

#include <string>
#include <memory>

enum NbtTagType
{
	NBT_TAG_TYPE_END = 0,
	NBT_TAG_TYPE_BYTE = 1,
	NBT_TAG_TYPE_SHORT = 2,
	NBT_TAG_TYPE_INT = 3,
	NBT_TAG_TYPE_LONG = 4,
	NBT_TAG_TYPE_FLOAT = 5,
	NBT_TAG_TYPE_DOUBLE = 6,
	NBT_TAG_TYPE_BYTE_ARRAY = 7,
	NBT_TAG_TYPE_STRING = 8,
	NBT_TAG_TYPE_LIST = 9,
	NBT_TAG_TYPE_COMPOUND = 10
};

class NBTBase
{
public:
	virtual NbtTagType GetTagType() = 0;
	void Write(BigEndianDataOutputStream& out);
	void Read(BigEndianDataInputStream& in);
	std::string GetName();

	static std::shared_ptr<NBTBase> CreateTagFromType(NbtTagType type);
	static std::shared_ptr<NBTBase> ReadTag(BigEndianDataInputStream& in);

	virtual void WriteContent(BigEndianDataOutputStream& out) {};
	virtual void ReadContent(BigEndianDataInputStream& in) {};

	void SetKey(std::string_view key);

protected:
	
	std::string m_Name;
};