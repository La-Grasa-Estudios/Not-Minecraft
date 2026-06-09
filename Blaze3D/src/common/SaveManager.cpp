#include "SaveManager.h"

#include "nbt/NBT.h"
#include "common/World.h"
#include "common/entity/BaseEntity.h"
#include "engine/zlib/izlibstream.h"
#include "engine/zlib/ozlibstream.h"

#include "engine/System.h"

#include "RegionFile.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <mutex>

constexpr uint32_t MAX_CACHE_SIZE = 128;
static std::mutex g_WriteMutex;

SaveManager::SaveManager(const std::string& path)
{
	m_Path = path;

	if (path.contains("client"))
		return;

	if (!m_Path.ends_with('/'))
	{
		m_Path.append("/");
	}

	if (!std::filesystem::exists(m_Path))
	{
		std::filesystem::create_directories(m_Path);
	}

	if (!std::filesystem::exists(m_Path + "region/"))
	{
		std::filesystem::create_directories(m_Path + "region/");
	}

	if (!std::filesystem::exists(m_Path + "entities/"))
	{
		std::filesystem::create_directories(m_Path + "entities/");
	}

	if (!std::filesystem::exists(m_Path + "DIM-1/region/"))
	{
		std::filesystem::create_directories(m_Path + "DIM-1/region/");
	}
}

void SaveManager::Save(World* world)
{
	if (m_Path.contains("client"))
		return;

	TagCompound root("");

	auto levelData = std::make_shared<TagCompound>("Data");
	world->WriteToNbt(*levelData);

	root.SetTag(levelData);

	std::ofstream out(m_Path + "level.dat", std::ios::binary);
	BigEndianDataOutputStream output(out);

	root.Write(output);
}

void SaveManager::Save(Chunk* chunk)
{
	if (m_Path.contains("client"))
		return;

	TagCompound tag("");

	chunk->WriteToNbt(tag);

	auto entities = chunk->ChunkWorld->GetEntitiesInChunk(chunk);
	std::shared_ptr<TagList> entitiesNbt = std::make_shared<TagList>("Entities", NBT_TAG_TYPE_COMPOUND);

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i]->IsSerializable())
		{
			entities[i]->ForceRemove = true;
			auto tag = std::make_shared<TagCompound>("");
			entities[i]->WriteToNbt(*tag);
			entitiesNbt->Add(tag);
		}
	}

	std::shared_ptr<TagList> tickList = std::make_shared<TagList>("TickList", NBT_TAG_TYPE_COMPOUND);

	for (auto& tick : chunk->ChunkWorld->m_TickList)
	{
		if (chunk->IsInsideChunk(tick.x, tick.y, tick.z))
		{
			auto tag = std::make_shared<TagCompound>("");
			tag->SetInt("x", tick.x);
			tag->SetInt("y", tick.y);
			tag->SetInt("z", tick.z);
			tag->SetShort("type", tick.type);
			tag->SetShort("delay", tick.delay);
			tickList->Add(tag);
		}
	}

	tag.SetTag(entitiesNbt);
	tag.SetTag(tickList);

	std::stringstream out;
	zlib::ozlibstream outGzip(out, 1);

	BigEndianDataOutputStream output(outGzip);

	tag.Write(output);

	outGzip.close();

	int x = (int)floor(chunk->PosX / 32.0f);
	int z = (int)floor(chunk->PosZ / 32.0f);

#ifndef __wii__
	std::scoped_lock lock(g_WriteMutex);
#endif

	RegionFile* region = NULL;

	if (m_RegionCache.contains(GetRegionCoord(x, z)))
	{
		region = m_RegionCache[GetRegionCoord(x, z)].get();
	}
	else
	{
		if (m_RegionCache.size() >= MAX_CACHE_SIZE)
		{
			m_RegionCache.clear();
		}

		char path[128];

		char dimension[128]{};

		if (chunk->ChunkWorld->GetDimensionID() != 0)
		{
			snprintf(dimension, 128, "/DIM%i", chunk->ChunkWorld->GetDimensionID());
		}

		snprintf(path, 128, "%s%s", m_Path.c_str(), dimension);

		char buffer[128];
		snprintf(buffer, 128, "%s/region/r.%i.%i.mca", path, x, z);
		auto ref = std::make_shared<RegionFile>(buffer);

		m_RegionCache[GetRegionCoord(x, z)] = ref;
		region = ref.get();
	}

	std::string_view rdBuf = out.rdbuf()->view();

	region->WriteStream(chunk->PosX & 31, chunk->PosZ & 31, (int)rdBuf.size(), (char*)rdBuf.data());
}

void SaveManager::Save(BaseEntity* entity, std::string_view name)
{
	if (m_Path.contains("client"))
		return;

	std::ofstream out(m_Path + "entities/" + std::string(name) + ".dat", std::ios::binary);
	BigEndianDataOutputStream output(out);

	TagCompound tag;
	entity->WriteToNbt(tag);
	tag.Write(output);
}

bool SaveManager::Load(World* world)
{
	if (m_Path.contains("client"))
		return false;

	std::ifstream in(m_Path + "level.dat", std::ios::binary);

	if (!in.is_open() || !in.good())
	{
		return false;
	}

	BigEndianDataInputStream input(in);
	auto root = NBTBase::ReadTag(input);

	auto& levelData = ((TagCompound*)root.get())->GetTagAs<TagCompound>("Data");
	world->LoadFromNbt(levelData);

	return true;
}

bool SaveManager::Load(Chunk* chunk)
{
	if (m_Path.contains("client"))
		return false;

#ifndef __wii__
	std::scoped_lock lock(g_WriteMutex);
#endif

	int x = (int)floor(chunk->PosX / 32.0f);
	int z = (int)floor(chunk->PosZ / 32.0f);

	RegionFile* region = NULL;

	if (m_RegionCache.contains(GetRegionCoord(x, z)))
	{
		region = m_RegionCache[GetRegionCoord(x, z)].get();
	}
	else
	{
		if (m_RegionCache.size() >= MAX_CACHE_SIZE)
		{
			m_RegionCache.clear();
		}

		char path[64];

		char dimension[64]{};

		if (chunk->ChunkWorld->GetDimensionID() != 0)
		{
			snprintf(dimension, 64, "/DIM%i", chunk->ChunkWorld->GetDimensionID());
		}

		snprintf(path, 64, "%s%s", m_Path.c_str(), dimension);

		char buffer[64];
		snprintf(buffer, 64, "%s/region/r.%i.%i.mca", path, x, z);
		auto ref = std::make_shared<RegionFile>(buffer);

		m_RegionCache[GetRegionCoord(x, z)] = ref;
		region = ref.get();
	}

	int sz;
	auto stream = region->GetChunkReadStream(chunk->PosX & 31, chunk->PosZ & 31, &sz);

	if (!stream)
	{
		return false;
	}

	std::vector<char> buff(sz);
	stream->read(buff.data(), sz);

	std::stringstream mem;
	mem.write(buff.data(), sz);
	buff = {};

	auto gzipIn = zlib::izlibstream{ mem };

	BigEndianDataInputStream input(gzipIn);
	auto root = NBTBase::ReadTag(input);
	if (!root)
	{
		return false;
	}
	auto& compound = *((TagCompound*)root.get());

	if (!compound.Contains("Heightmap") ||
		!compound.Contains("Blocks") ||
		!compound.Contains("Lightmap") ||
		!compound.Contains("Metadata") ||
		!compound.Contains("TerrainPopulated"))
	{
		printf("[Warning] chunk %i, %i got corrupted, resetting it!\n", chunk->PosX, chunk->PosZ);
		return false;
	}

	chunk->LoadFromNbt(compound);

	if (compound.Contains("Entities"))
	{
		auto& entities = compound.GetTagAs<TagList>("Entities");

		for (size_t i = 0; i < entities.Size(); i++)
		{
			auto& tag = entities.GetAs<TagCompound>(i);
			auto entity = BaseEntity::ReadEntity(tag, chunk->ChunkWorld);
			if (entity)
			{
				entity->ReadFromNbt(tag);
				chunk->ChunkWorld->AddEntity(entity);
			}
		}
	}

	if (compound.Contains("TickList"))
	{
		auto& tickList = compound.GetTagAs<TagList>("TickList");
		for (size_t i = 0; i < tickList.Size(); i++)
		{
			auto& tag = tickList.GetAs<TagCompound>(i);
			chunk->ChunkWorld->ScheduleUpdate(
				tag.GetInt("x"), tag.GetInt("y"), tag.GetInt("z"),
				tag.GetShort("delay"), tag.GetShort("type")
			);
		}
	}

	return true;
}

bool SaveManager::Load(BaseEntity* entity, std::string_view name)
{
	if (m_Path.contains("client"))
		return false;

	std::ifstream in(m_Path + "entities/" + std::string(name) + ".dat", std::ios::binary);

	if (!in.is_open() || !in.good())
	{
		return false;
	}

	BigEndianDataInputStream input(in);
	auto root = NBTBase::ReadTag(input);

	auto& entityData = *((TagCompound*)root.get());
	entity->ReadFromNbt(entityData);

	return true;
}

size_t SaveManager::GetCacheSize()
{
	return m_RegionCache.size();
}

std::string SaveManager::GetSavePath()
{
	return m_Path;
}

uint64_t SaveManager::GetRegionCoord(int x, int z)
{
	int64_t l = x;
	int64_t l1 = z;
	return (l & 0xffffffffL) | ((l1 & 0xffffffffL) << 32);
}
