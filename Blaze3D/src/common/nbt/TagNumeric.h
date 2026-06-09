#pragma once

#include "NBTBase.h"

class TagInt : public NBTBase
{
public:
	TagInt() = default;
	TagInt(std::string_view name, int value);
	NbtTagType GetTagType();

	operator int();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	int m_Value;
};

class TagShort : public NBTBase
{
public:
	TagShort() = default;
	TagShort(std::string_view name, int16_t value);
	NbtTagType GetTagType();

	operator int16_t();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	int16_t m_Value;
};

class TagByte : public NBTBase
{
public:
	TagByte() = default;
	TagByte(std::string_view name, int8_t value);
	NbtTagType GetTagType();

	operator int8_t();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	int8_t m_Value;
};

class TagFloat : public NBTBase
{
public:
	TagFloat() = default;
	TagFloat(std::string_view name, float value);
	NbtTagType GetTagType();

	operator float();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	float m_Value;
};

class TagDouble : public NBTBase
{
public:
	TagDouble() = default;
	TagDouble(std::string_view name, double value);
	NbtTagType GetTagType();

	operator double();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	double m_Value;
};

class TagLong : public NBTBase
{
public:
	TagLong() = default;
	TagLong(std::string_view name, int64_t value);
	NbtTagType GetTagType();

	operator int64_t();

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	int64_t m_Value;
};