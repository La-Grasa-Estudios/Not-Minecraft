#include "World.h"

#include "nbt/NBT.h"
#include "tile/TileEntity.h"

#include <cstring>
#include <thirdparty/glm/ext.hpp>

#define CHUNK_BLOCK_ARRAY_INDEX(x, y, z) (x + 16 * (y + CHUNK_HEIGHT * z))

Chunk::Chunk(World* world, int x, int z) : ChunkWorld(world), PosX(x), PosZ(z)
{
    memset(Blocks, 0, sizeof(Blocks));
    memset(LightMap, 0, sizeof(LightMap));
    memset(HeightMap, 0, sizeof(HeightMap));
    GenStage = TERRAIN_GEN_UNPOPULATED;
}

BlockStorage Chunk::GetBlock(int x, int y, int z) const
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return BLOCK_AIR;

    int xx = x & 0xF;
    int zz = z & 0xF;

    return Blocks[CHUNK_BLOCK_ARRAY_INDEX(xx, y, zz)];
}

BlockStorage Chunk::GetBlockGlobal(int x, int y, int z) const
{
    if (!IsInsideChunk(x, y, z))
    {
        return ChunkWorld->GetBlock(x, y, z);
    }
    return GetBlock(x, y, z);
}

void Chunk::SetBlock(int x, int y, int z, BlockStorage block)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return;

    int xx = x & 0xF;
    int zz = z & 0xF;

    Blocks[CHUNK_BLOCK_ARRAY_INDEX(xx, y, zz)] = block;

    // Dirty mark
    if (SaveTicks <= 0)
        SaveTicks = 900;
}

uint8_t Chunk::GetHeight(int x, int z)
{
     if (!IsInsideChunk(x, 0, z))
    {
        return ChunkWorld->GetHeight(x, z);
    }
    x = x & 0xF;
    z = z & 0xF;
    int index = x + z * CHUNK_WIDTH;
    return HeightMap[index].Height;
}

uint8_t Chunk::GetHeightLocal(int x, int z)
{
    x = x & 0xF;
    z = z & 0xF;
    int index = x + z * CHUNK_WIDTH;
    return HeightMap[index].Height;
}

void Chunk::SetSkyLight(int x, int y, int z, int level)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return;

    if (!IsInsideChunk(x, y, z))
    {
        ChunkWorld->SetSkyLight(x, y, z, level);
    }
    SetSkyLightLocal(x, y, z, level);
}

void Chunk::SetSkyLightLocal(int x, int y, int z, int level)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return;

    x = x & 0xF;
    z = z & 0xF;
    auto data = LightMap[CHUNK_BLOCK_ARRAY_INDEX(x, y, z)] & 0xF0;
    LightMap[CHUNK_BLOCK_ARRAY_INDEX(x, y, z)] = data | (level & 0x0F);

    // Dirty mark
    if (SaveTicks <= 0)
        SaveTicks = 900;
}

int Chunk::GetSkyLight(int x, int y, int z)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return 15;
    if (!IsInsideChunk(x, y, z))
    {
        return ChunkWorld->GetSkyLight(x, y, z);
    }
    return GetSkyLightLocal(x, y, z);
}

int Chunk::GetSkyLightLocal(int x, int y, int z)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return 0;
    x = x & 0xF;
    z = z & 0xF;
    return LightMap[CHUNK_BLOCK_ARRAY_INDEX(x, y, z)] & 0x0F;
}

void Chunk::SetBlockLight(int x, int y, int z, int level)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return;

    if (!IsInsideChunk(x, y, z))
    {
        ChunkWorld->SetBlockLight(x, y, z, level);
    }
    SetBlockLightLocal(x, y, z, level);
}

void Chunk::SetBlockLightLocal(int x, int y, int z, int level)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return;

    x = x & 0xF;
    z = z & 0xF;
    auto data = LightMap[CHUNK_BLOCK_ARRAY_INDEX(x, y, z)] & 0x0F;
    data = data | ((level & 0x0F) << 4);
    LightMap[CHUNK_BLOCK_ARRAY_INDEX(x, y, z)] = data;

    // Dirty mark
    if (SaveTicks <= 0)
        SaveTicks = 900;
}

int Chunk::GetBlockLight(int x, int y, int z)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return 0;
    if (!IsInsideChunk(x, y, z))
    {
        return ChunkWorld->GetBlockLight(x, y, z);
    }
    return GetBlockLightLocal(x, y, z);
}

int Chunk::GetBlockLightLocal(int x, int y, int z)
{
    if (y < 0 || y > CHUNK_HEIGHT - 1)
        return 0;
    x = x & 0xF;
    z = z & 0xF;
    return (LightMap[CHUNK_BLOCK_ARRAY_INDEX(x, y, z)] >> 4) & 0x0F;
}

int Chunk::GetMixedLightLocal(int x, int y, int z)
{
    return glm::max(GetBlockLightLocal(x, y, z), GetSkyLightLocal(x, y, z));
}

void Chunk::SetTileEntity(int x, int y, int z, std::shared_ptr<TileEntity> entity)
{
    if (!entity)
    {
        TileEntities.erase(glm::ivec3(x, y, z));
        return;
    }
    TileEntities[glm::ivec3(x, y, z)] = entity;

    // Dirty mark
    if (SaveTicks <= 0)
        SaveTicks = 900;
}

TileEntity* Chunk::GetTileEntity(int x, int y, int z)
{
    if (auto entity = TileEntities.find(glm::ivec3(x, y, z)); entity != TileEntities.end())
    {
        return entity->second.get();
    }
    return NULL;
}

bool Chunk::HasAllNeighboursLoaded()
{
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int x = GetStartX() + i * 16;
            int z = GetStartZ() + j * 16;
            if (!ChunkWorld->GetChunk(x, z)) {
                return false;
            }
        }
    }
    return true;
}

bool Chunk::IsInsideChunk(int x, int y, int z) const
{
    auto px = (PosX * CHUNK_WIDTH);
    auto pz = (PosZ * CHUNK_WIDTH);
    return x >= px && x < px + 16 &&
        z >= pz && z < pz + 16 &&
        y >= 0 && y < CHUNK_HEIGHT;
}

void Chunk::ComputeColumnHeight(int localX, int localZ)
{
    localX = localX & 0xF;
    localZ = localZ & 0xF;
    int height = 0;
    for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
    {
        bool blocksLightSides = !GetBlock(localX, y, localZ).GetDef()->BlocksLightToTheSides();

        bool aboveBlocksLightToSides = !GetBlock(localX, y + 1, localZ).GetDef()->BlocksLightToTheSides();

        auto block = GetBlock(localX, y, localZ).GetDef();
        bool isOpaque = block->IsOpaque() || block->GetOpacity() > 0;

        if ((isOpaque && !blocksLightSides) || aboveBlocksLightToSides)
        {
            height = y + 1;
            break;
        }
    }
    int index = localX + localZ * CHUNK_WIDTH;
    if (index >= 0 && index < 256)
    {
        HeightMap[index].Height = height;
    }
}

int Chunk::GetStartX()
{
    return PosX * CHUNK_WIDTH;
}

int Chunk::GetStartZ()
{
    return PosZ * CHUNK_WIDTH;
}

void Chunk::WriteToNbt(TagCompound& root)
{
    std::vector<int8_t> BlockData(sizeof(Blocks) / sizeof(BlockStorage));
    std::vector<int8_t> Metadata(sizeof(Blocks) / sizeof(BlockStorage));
    std::vector<int8_t> LightData(sizeof(Blocks) / sizeof(BlockStorage));
    std::vector<int8_t> Heightmap(sizeof(HeightMap) / sizeof(HeightMapEntry));

    for (int i = 0; i < BlockData.size(); i++)
    {
        BlockData[i] = Blocks[i].Id;
        Metadata[i] = Blocks[i].Metadata;
        LightData[i] = LightMap[i];
    }
    for (int i = 0; i < Heightmap.size(); i++)
    {
        Heightmap[i] = HeightMap[i].Height;
    }

    auto tileEntities = std::make_shared<TagList>("TileEntities", NBT_TAG_TYPE_COMPOUND);

    for (auto& kv : TileEntities)
    {
        auto tag = std::make_shared<TagCompound>("");
        kv.second->WriteToNbt(*tag);
        tileEntities->Add(tag);
    }

    root.SetInt("xPos", PosX);
    root.SetInt("zPos", PosZ);
    root.SetByte("TerrainPopulated", GenStage);
    root.SetTag(std::make_shared<TagByteArray>("Blocks", BlockData));
    root.SetTag(std::make_shared<TagByteArray>("Metadata", Metadata));
    root.SetTag(std::make_shared<TagByteArray>("Lightmap", LightData));
    root.SetTag(std::make_shared<TagByteArray>("Heightmap", Heightmap));
    root.SetTag(tileEntities);
}

void Chunk::LoadFromNbt(TagCompound& root)
{
    auto& BlockData = root.GetTagAs<TagByteArray>("Blocks");
    auto& Metadata = root.GetTagAs<TagByteArray>("Metadata");
    auto& Lightdata = root.GetTagAs<TagByteArray>("Lightmap");
    auto& Heightmap = root.GetTagAs<TagByteArray>("Heightmap");

    for (int i = 0; i < BlockData.Size(); i++)
    {
        Blocks[i].Id = BlockData.Values[i];
        Blocks[i].Metadata = Metadata.Values[i];
        LightMap[i] = Lightdata.Values[i];
    }
    for (int i = 0; i < Heightmap.Size(); i++)
    {
        HeightMap[i].Height = Heightmap.Values[i];
    }

    if (root.Contains("TileEntities"))
    {
        auto& tileEntities = root.GetTagAs<TagList>("TileEntities");
        for (int i = 0; i < tileEntities.Size(); i++)
        {
            auto& tag = tileEntities.GetAs<TagCompound>(i);
            auto tileEntity = TileEntity::ReadTileEntityFromTag(this, tag);
            if (!tileEntity)
            {
                printf("Corrupt tile entity! at chunk %i %i\n", PosX, PosZ);
            }
            else
            {
                tileEntity->ReadFromNbt(tag);
                if (tileEntity)
                {
                    SetTileEntity(tileEntity->posX, tileEntity->posY, tileEntity->posZ, tileEntity);
                }
            }
        }
    }

    GenStage = static_cast<TerrainGenerationStage>(root.GetByte("TerrainPopulated"));
}

size_t HasherVector3Int::operator()(const glm::ivec3& v) const
{
    return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
}