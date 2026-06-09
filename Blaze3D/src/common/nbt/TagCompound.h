#pragma once

#include "NBTBase.h"

#include <unordered_map>

class TagCompound : public NBTBase
{
public:
	TagCompound() = default;
	TagCompound(std::string_view name);
	NbtTagType GetTagType();

	template<typename T>
	T& GetTagAs(std::string_view name)
	{
		static_assert(std::is_base_of<NBTBase, T>::value, "Invalid Tag provided");
		return *static_cast<T*>(GetTag(name).get());
	}
	std::shared_ptr<NBTBase> GetTag(std::string_view name);
	bool Contains(std::string_view name);
	void SetTag(std::shared_ptr<NBTBase> tag);

	void SetByte(std::string_view name, int8_t value);
	void SetShort(std::string_view name, int16_t value);
	void SetInt(std::string_view name, int32_t value);
	void SetLong(std::string_view name, int64_t value);
	void SetFloat(std::string_view name, float value);
	void SetDouble(std::string_view name, double value);
	void SetBool(std::string_view name, bool value);
	void SetString(std::string_view name, std::string_view text);

	int8_t GetByte(std::string_view name);
	int16_t GetShort(std::string_view name);
	int32_t GetInt(std::string_view name);
	int64_t GetLong(std::string_view name);
	float GetFloat(std::string_view name);
	double GetDouble(std::string_view name);
	bool GetBool(std::string_view name);
	std::string GetString(std::string_view name);

	void CombineTag(TagCompound& other);

private:
	void WriteContent(BigEndianDataOutputStream& out);
	void ReadContent(BigEndianDataInputStream& in);

	std::unordered_map<std::string, std::shared_ptr<NBTBase>> m_Values;
};