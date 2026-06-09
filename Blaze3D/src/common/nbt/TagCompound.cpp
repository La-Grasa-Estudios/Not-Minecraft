#include "TagCompound.h"
#include "TagNumeric.h"
#include "TagString.h"

TagCompound::TagCompound(std::string_view name)
{
	m_Name = name;
}

NbtTagType TagCompound::GetTagType()
{
	return NBT_TAG_TYPE_COMPOUND;
}

std::shared_ptr<NBTBase> TagCompound::GetTag(std::string_view name)
{
	if (!m_Values.contains(name.data()))
	{
		return NULL;
	}
	return m_Values[name.data()];
}

bool TagCompound::Contains(std::string_view name)
{
	return m_Values.contains(name.data());
}

void TagCompound::SetTag(std::shared_ptr<NBTBase> tag)
{
	m_Values[tag->GetName()] = tag;
}

void TagCompound::SetByte(std::string_view name, int8_t value)
{
	m_Values[name.data()] = std::make_shared<TagByte>(name, value);
}

int8_t TagCompound::GetByte(std::string_view name)
{
	if (!Contains(name))
		return 0;
	return GetTagAs<TagByte>(name);
}

void TagCompound::SetShort(std::string_view name, int16_t value)
{
	m_Values[name.data()] = std::make_shared<TagShort>(name, value);
}

int16_t TagCompound::GetShort(std::string_view name)
{
	if (!Contains(name))
		return 0;
	return GetTagAs<TagShort>(name);
}

void TagCompound::SetInt(std::string_view name, int32_t value)
{
	m_Values[name.data()] = std::make_shared<TagInt>(name, value);
}

int32_t TagCompound::GetInt(std::string_view name)
{
	if (!Contains(name))
		return 0;
	return GetTagAs<TagInt>(name);
}

void TagCompound::SetLong(std::string_view name, int64_t value)
{
	m_Values[name.data()] = std::make_shared<TagLong>(name, value);
}

int64_t TagCompound::GetLong(std::string_view name)
{
	if (!Contains(name))
		return 0;
	return GetTagAs<TagLong>(name);
}

void TagCompound::SetFloat(std::string_view name, float value)
{
	m_Values[name.data()] = std::make_shared<TagFloat>(name, value);
}

float TagCompound::GetFloat(std::string_view name)
{
	if (!Contains(name))
		return 0;
	return GetTagAs<TagFloat>(name);
}

void TagCompound::SetDouble(std::string_view name, double value)
{
	m_Values[name.data()] = std::make_shared<TagDouble>(name, value);
}

double TagCompound::GetDouble(std::string_view name)
{
	if (!Contains(name))
		return 0;
	return GetTagAs<TagDouble>(name);
}

void TagCompound::SetBool(std::string_view name, bool value)
{
	m_Values[name.data()] = std::make_shared<TagByte>(name, static_cast<int8_t>(value ? 1 : 0));
}

void TagCompound::SetString(std::string_view name, std::string_view text)
{
	m_Values[name.data()] = std::make_shared<TagString>(name, text);
}

bool TagCompound::GetBool(std::string_view name)
{
	if (!Contains(name))
		return false;
	return GetTagAs<TagByte>(name) != 0;
}

std::string TagCompound::GetString(std::string_view name)
{
	return GetTagAs<TagString>(name);
}

void TagCompound::CombineTag(TagCompound& other)
{
	for (auto& kv : other.m_Values)
	{
		m_Values[kv.first] = kv.second;
	}
}


void TagCompound::WriteContent(BigEndianDataOutputStream& out)
{
	for (auto kv : m_Values)
	{
		kv.second->Write(out);
	}

	out.WriteI8(NBT_TAG_TYPE_END);
}

void TagCompound::ReadContent(BigEndianDataInputStream& in)
{
	while (true)
	{
		auto tag = ReadTag(in);
		if (tag)
		{
			m_Values[tag->GetName()] = tag;
		}
		else
		{
			break;
		}
	}
}
