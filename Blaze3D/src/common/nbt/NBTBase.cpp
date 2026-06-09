#include "NBT.h"

void NBTBase::Write(BigEndianDataOutputStream& out)
{
	out.WriteI8(GetTagType());
	out.WriteString(m_Name);
	WriteContent(out);
}

void NBTBase::Read(BigEndianDataInputStream& in)
{
	NbtTagType type = static_cast<NbtTagType>(in.ReadI8());
	if (type != NBT_TAG_TYPE_END)
	{
		auto tag = CreateTagFromType(type);

		tag->m_Name = in.ReadString();
		tag->ReadContent(in);
	}
}

std::string NBTBase::GetName()
{
	return m_Name;
}

std::shared_ptr<NBTBase> NBTBase::CreateTagFromType(NbtTagType type)
{
    switch (type)
    {
    case NBT_TAG_TYPE_END:
        return nullptr;
    case NBT_TAG_TYPE_BYTE:
        return std::make_shared<TagByte>();
    case NBT_TAG_TYPE_SHORT:
        return std::make_shared<TagShort>();
    case NBT_TAG_TYPE_INT:
        return std::make_shared<TagInt>();
    case NBT_TAG_TYPE_LONG:
        return std::make_shared<TagLong>();
    case NBT_TAG_TYPE_FLOAT:
        return std::make_shared<TagFloat>();
    case NBT_TAG_TYPE_DOUBLE:
        return std::make_shared<TagDouble>();
    case NBT_TAG_TYPE_BYTE_ARRAY:
        return std::make_shared<TagByteArray>();
    case NBT_TAG_TYPE_STRING:
        return std::make_shared<TagString>();
    case NBT_TAG_TYPE_LIST:
        return std::make_shared<TagList>();
    case NBT_TAG_TYPE_COMPOUND:
        return std::make_shared<TagCompound>();
    default:
        return nullptr;
    }
}

std::shared_ptr<NBTBase> NBTBase::ReadTag(BigEndianDataInputStream& in)
{
	NbtTagType type = static_cast<NbtTagType>(in.ReadI8());

	if (type != NBT_TAG_TYPE_END)
	{
		auto tag = CreateTagFromType(type);
        if (!tag)
            return NULL;
		tag->m_Name = in.ReadString();
		tag->ReadContent(in);
		return tag;
	}
	return NULL;
}

void NBTBase::SetKey(std::string_view key)
{
    m_Name = key;
}
