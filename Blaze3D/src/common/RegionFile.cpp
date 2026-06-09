#include "RegionFile.h"

#include "common/nbt/BigEndianDataStream.h"

#include "engine/System.h"

#include <filesystem>
#include <cstring>

RegionFile::RegionFile(std::string_view fileName)
{
    memset(m_ChunkOffsets, 0, sizeof(m_ChunkOffsets));
    m_Filename = fileName;

    if (!std::filesystem::exists(fileName))
    {
        std::ofstream out(fileName.data(), std::ios::binary);

        int sector[1024]{};

        out.write(reinterpret_cast<char*>(sector), sizeof(sector));
        out.write(reinterpret_cast<char*>(sector), sizeof(sector));
    }

    m_Stream = std::make_shared<std::fstream>(std::string(fileName), std::ios::binary | std::ios::in | std::ios::out);

    // Get size
    m_Stream->seekg(0, std::ios::end);
    size_t size = m_Stream->tellg();

    // Not multiple of 4, grow
    if ((size & 0xfff) != 0)
    {
        BigEndianDataOutputStream output(*m_Stream);
        for (size_t i = 0; i < (size & 0xfff); i++)
        {
            output.WriteI32(0);
            size++;
        }
    }

    size_t nSectors = size / SECTOR_BYTES;

    for (size_t i = 0; i < nSectors; i++)
    {
        m_SectorsFree.push_back(true);
    }

    // Mark chunk offsets and timestamps has used
    // timestamps are not used in this implementation
    m_SectorsFree[0] = false;
    m_SectorsFree[1] = false;

    m_Stream->seekg(0, std::ios::beg);
    BigEndianDataInputStream input(*m_Stream);

    // Read offset table
    for (uint32_t i = 0; i < SECTOR_INTS; i++)
    {
        int offset = input.ReadI32();
        m_ChunkOffsets[i] = offset;
        if (offset != 0 && (offset >> 8) + (offset & 0xFF) <= (int)m_SectorsFree.size()) {
            for (int sectorNum = 0; sectorNum < (offset & 0xFF); ++sectorNum) {
                m_SectorsFree[(offset >> 8) + sectorNum] = false;
            }
        }
    }
}

void RegionFile::WriteStream(int posX, int posZ, int size, char* data)
{
    int offset = GetChunkSectorIndex(posX, posZ);
    int sectorNumber = offset >> 8;
    int sectorsAllocated = offset & 0xFF;
    int sectorsNeeded = (size + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;

    if (sectorNumber != 0 && sectorsAllocated == sectorsNeeded)
    {
        m_Stream->seekp(sectorNumber * SECTOR_BYTES, std::ios::beg);
        BigEndianDataOutputStream output(*m_Stream);
        output.WriteI32(size + 1);
        output.WriteI8(1); // GZIP
        m_Stream->write(data, size);
        return;
    }

    for (int i = 0; i < sectorsAllocated; i++)
    {
        m_SectorsFree[sectorNumber + i] = true;
    }

    int runStart = FindFirstFreeSector();
    int runLength = 0;
    if (runStart != -1) {
        for (int i = runStart; i < (int)m_SectorsFree.size(); ++i) {
            if (runLength != 0) {
                if (m_SectorsFree[i]) runLength++;
                else runLength = 0;
            }
            else if (m_SectorsFree[i]) {
                runStart = i;
                runLength = 1;
            }
            if (runLength >= sectorsNeeded) {
                break;
            }
        }
    }

    if (runLength >= sectorsNeeded) {
        // We found a free space large enough
        sectorNumber = runStart;
        SetOffset(posX, posZ, (sectorNumber << 8) | sectorsNeeded);
        for (int i = 0; i < sectorsNeeded; ++i) {
            m_SectorsFree[sectorNumber + i] = false;
        }
        m_Stream->seekp(sectorNumber * SECTOR_BYTES, std::ios::beg);

        BigEndianDataOutputStream output(*m_Stream);
        output.WriteI32(size + 1);
        output.WriteI8(1); // GZIP
        m_Stream->write(data, size);
    }
    else
    {
        // No free space, grow file

        sectorNumber = (int)m_SectorsFree.size();

        int sector[1024]{};

#ifdef __wii__
        // Wii specific hack, otherwise file doesn't grow
        m_Stream->close();

        std::ofstream out(m_Filename, std::ios::out | std::ios::app | std::ios::binary);
        for (int i = 0; i < sectorsNeeded; i++)
        {
            out.write(reinterpret_cast<char*>(sector), sizeof(sector));
            m_SectorsFree.push_back(false);
        }
        out.close();

        m_Stream = std::make_shared<std::fstream>(m_Filename, std::ios::binary | std::ios::in | std::ios::out);
#else
        m_Stream->seekp(0, std::ios::end);
        for (int i = 0; i < sectorsNeeded; i++)
        {
            m_Stream->write(reinterpret_cast<char*>(sector), sizeof(sector));
            m_SectorsFree.push_back(false);
        }
#endif

        m_Stream->seekp(sectorNumber * SECTOR_BYTES, std::ios::beg);

        BigEndianDataOutputStream output(*m_Stream);
        output.WriteI32(size + 1);
        output.WriteI8(1); // GZIP
        m_Stream->write(data, size);

        SetOffset(posX, posZ, (sectorNumber << 8) | sectorsNeeded);
    }
}

std::istream* RegionFile::GetChunkReadStream(int posX, int posZ, int* sizePtr)
{
    int offset = GetChunkSectorIndex(posX, posZ);
    if (offset == 0)
    {
        return NULL;
    }

    int sectorNumber = offset >> 8;
    int numSectors = offset & 0xFF;

    if (sectorNumber + numSectors > (int)m_SectorsFree.size()) {
        return NULL;
    }

    m_Stream->seekg(sectorNumber * SECTOR_BYTES, std::ios::beg);

    BigEndianDataInputStream input(*m_Stream);

    auto size = input.ReadI32();
    input.ReadI8(); // Skip type byte

    if (size > (int)SECTOR_BYTES * (int)m_SectorsFree.size())
        return NULL;

    *sizePtr = size;

    return m_Stream.get();
}

inline int RegionFile::GetChunkSectorIndex(int posX, int posZ)
{
    auto index = (uint32_t)(posX + posZ * 32);
    if (index < 0 || index > SECTOR_INTS - 1)
    {
        return 0;
    }
    return m_ChunkOffsets[index];
}

int RegionFile::FindFirstFreeSector()
{
    for (size_t i = 0; i < m_SectorsFree.size(); i++)
    {
        if (m_SectorsFree[i])
        {
            return (int)i;
        }
    }
    return -1;
}

void RegionFile::SetOffset(int posX, int posZ, int val)
{
    int index = (posX + posZ * 32);
    m_ChunkOffsets[index] = val;
    m_Stream->seekp(index * sizeof(int));
    BigEndianDataOutputStream output(*m_Stream);
    output.WriteI32(val);
}
