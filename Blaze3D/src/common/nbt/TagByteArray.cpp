#include "TagByteArray.h"

TagByteArray::TagByteArray(std::string_view name, const std::vector<int8_t>& array)
{
	Values = std::move(array);
	m_Name = name;
}

NbtTagType TagByteArray::GetTagType()
{
	return NBT_TAG_TYPE_BYTE_ARRAY;
}

size_t TagByteArray::Size()
{
	return Values.size();
}

void TagByteArray::WriteContent(BigEndianDataOutputStream& out)
{
	out.WriteI32(Values.size());
	out.WriteArray(Values.data(), Values.size());
}

void TagByteArray::ReadContent(BigEndianDataInputStream& in)
{
	auto size = in.ReadI32();
	Values.resize(size);
	in.ReadArray(Values.data(), Values.size());
}
