#pragma once

// Based on https://pastebin.com/niWTqLvk

#include <string_view>
#include <cstdint>
#include <istream>
#include <ostream>
#include <fstream>
#include <memory>
#include <vector>

struct Chunk;

constexpr uint32_t SECTOR_BYTES = 4096;
constexpr uint32_t SECTOR_INTS = SECTOR_BYTES / sizeof(int);
constexpr uint32_t CHUNK_HEADER_SIZE = 5;

class RegionFile
{
public:
	RegionFile(std::string_view fileName);

	void WriteStream(int posX, int posZ, int size, char* data);	
	std::istream* GetChunkReadStream(int posX, int posZ, int* size);

private:

	// Returns -1 when chunk is outside region
	inline int GetChunkSectorIndex(int posX, int posZ);
	int FindFirstFreeSector();
	void SetOffset(int posX, int posZ, int val);

	int32_t m_ChunkOffsets[SECTOR_INTS];

	std::vector<bool> m_SectorsFree;
	std::shared_ptr<std::fstream> m_Stream;
	std::string m_Filename;

};