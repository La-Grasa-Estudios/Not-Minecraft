#include "TagString.h"

TagString::TagString(std::string_view name, std::string_view string)
{
	m_Value = string;
	m_Name = name;
}

void TagString::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteString(m_Value);
}

void TagString::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadString();
}

NbtTagType TagString::GetTagType()
{
	return NBT_TAG_TYPE_STRING;
}

TagString::operator std::string()
{
	return m_Value;
}
