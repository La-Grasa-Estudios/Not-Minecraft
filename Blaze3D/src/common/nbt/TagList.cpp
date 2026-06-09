#include "TagList.h"

TagList::TagList(std::string_view name, NbtTagType type)
{
	m_Name = name;
	m_Type = type;
}

NbtTagType TagList::GetTagType()
{
	return NBT_TAG_TYPE_LIST;
}

void TagList::Add(std::shared_ptr<NBTBase> tag)
{
	m_Values.push_back(tag);
}

std::shared_ptr<NBTBase> TagList::Get(size_t index)
{
	return m_Values[index];
}

size_t TagList::Size()
{
	return m_Values.size();
}

void TagList::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteI8(m_Type);
	out.WriteI32(static_cast<int32_t>(m_Values.size()));

	for (int i = 0; i < m_Values.size(); i++)
	{
		m_Values[i]->WriteContent(out);
	}
}

void TagList::ReadContent(BigEndianDataInputStream& in)
{
	m_Type = static_cast<NbtTagType>(in.ReadI8());
	auto size = in.ReadI32();
	for (int i = 0; i < size; i++)
	{
		auto tag = CreateTagFromType(m_Type);
		tag->ReadContent(in);
		m_Values.push_back(tag);
	}
}
