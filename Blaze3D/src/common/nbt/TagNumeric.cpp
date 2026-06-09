#include "TagNumeric.h"

TagInt::TagInt(std::string_view name, int value)
{
	m_Value = value;
	m_Name = name;
}

void TagInt::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteI32(m_Value);
}

void TagInt::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadI32();
}

NbtTagType TagInt::GetTagType()
{
	return NBT_TAG_TYPE_INT;
}

TagInt::operator int()
{
	return m_Value;
}

TagShort::TagShort(std::string_view name, int16_t value)
{
	m_Value = value;
	m_Name = name;
}

void TagShort::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteI16(m_Value);
}

void TagShort::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadI16();
}

NbtTagType TagShort::GetTagType()
{
	return NBT_TAG_TYPE_SHORT;
}

TagShort::operator int16_t()
{
	return m_Value;
}

TagByte::TagByte(std::string_view name, int8_t value)
{
	m_Value = value;
	m_Name = name;
}

void TagByte::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteI8(m_Value);
}

void TagByte::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadI8();
}

NbtTagType TagByte::GetTagType()
{
	return NBT_TAG_TYPE_BYTE;
}

TagByte::operator int8_t()
{
	return m_Value;
}

TagFloat::TagFloat(std::string_view name, float value)
{
	m_Value = value;
	m_Name = name;
}

void TagFloat::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteSingle(m_Value);
}

void TagFloat::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadSingle();
}

NbtTagType TagFloat::GetTagType()
{
	return NBT_TAG_TYPE_FLOAT;
}

TagFloat::operator float()
{
	return m_Value;
}

TagDouble::TagDouble(std::string_view name, double value)
{
	m_Value = value;
	m_Name = name;
}

void TagDouble::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteDouble(m_Value);
}

void TagDouble::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadDouble();
}

NbtTagType TagDouble::GetTagType()
{
	return NBT_TAG_TYPE_DOUBLE;
}

TagDouble::operator double()
{
	return m_Value;
}

void TagLong::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteI64(m_Value);
}

void TagLong::ReadContent(BigEndianDataInputStream& in)
{
	m_Value = in.ReadI64();
}

TagLong::TagLong(std::string_view name, int64_t value)
{
	m_Name = name;
	m_Value = value;
}

NbtTagType TagLong::GetTagType()
{
	return NBT_TAG_TYPE_LONG;
}

TagLong::operator int64_t()
{
	return m_Value;
}