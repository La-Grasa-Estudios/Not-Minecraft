#include "TileEntity.h"

#include "common/nbt/NBT.h"
#include "common/Chunk.h"

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

void TileEntity::RegisterEntityConstructor(const char* type, std::function<std::shared_ptr<TileEntity>(Chunk*, TagCompound&)> fn)
{
	s_EntityIDToFnMap[type] = fn;
}

std::shared_ptr<TileEntity> TileEntity::ReadTileEntityFromTag(Chunk* chunk, TagCompound& tag)
{
	if (!tag.Contains("EntityID"))
		return NULL;

	if (auto fn = s_EntityIDToFnMap.find(tag.GetTagAs<TagString>("EntityID")); fn != s_EntityIDToFnMap.end())
	{
		return fn->second(chunk, tag);
	}

	return NULL;
}

void TileEntity::ReadFromNbt(TagCompound& tag)
{
	posX = tag.GetInt("x");
	posY = tag.GetInt("y");
	posZ = tag.GetInt("z");
}

void TileEntity::WriteToNbt(TagCompound& tag)
{
	// TO DO: Merge the BaseEntity.cpp version and this to a Util file
#if defined(__GNUC__) || defined(__clang__)
	int status;
	char* realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
	std::string result(realname ? realname : typeid(*this).name());
	free(realname);
	tag.SetTag(std::make_shared<TagString>("EntityID", result));
#pragma message "Using CLANG|GCC demangling"
#else
	std::string className = typeid(*this).name();
	className = className.substr(className.find_first_of(' ') + 1);
	tag.SetTag(std::make_shared<TagString>("EntityID", className));
#endif

	tag.SetInt("x", posX);
	tag.SetInt("y", posY);
	tag.SetInt("z", posZ);
}

void TileEntity::Update()
{
}

void TileEntity::Render(riDevice* device)
{
}
