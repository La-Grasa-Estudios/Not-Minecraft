#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class World;
class RegionFile;
class BaseEntity;
struct Chunk;

class SaveManager
{
public:
	SaveManager() = default;
	SaveManager(const std::string& path);

	void Save(World* world);
	void Save(Chunk* chunk);
	void Save(BaseEntity* entity, std::string_view name);

	bool Load(World* world);
	bool Load(Chunk* chunk);
	bool Load(BaseEntity* entity, std::string_view name);

	size_t GetCacheSize();

	std::string GetSavePath();

private:

	uint64_t GetRegionCoord(int x, int z);

	std::unordered_map<uint64_t, std::shared_ptr<RegionFile>> m_RegionCache;

	std::string m_Path;
};