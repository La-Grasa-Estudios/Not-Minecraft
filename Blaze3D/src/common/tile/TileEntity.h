#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <memory>

struct Chunk;
class TagCompound;
class riDevice;

#define DECLARE_TILE_ENTITY_CONSTRUCTOR(type, fn) \
namespace Internal { \
    struct internalTileEntityConstructorFor##type { \
        internalTileEntityConstructorFor##type() { \
             TileEntity::RegisterEntityConstructor(#type, fn); \
        } \
    }; \
    static internalTileEntityConstructorFor##type objReg##type; \
}

class TileEntity
{
public:

    int posX, posY, posZ;
    Chunk* chunk;

    static void RegisterEntityConstructor(const char* type, std::function<std::shared_ptr<TileEntity>(Chunk*, TagCompound&)> fn);

    static std::shared_ptr<TileEntity> ReadTileEntityFromTag(Chunk* chunk, TagCompound& tag);

    virtual void ReadFromNbt(TagCompound& tag);
    virtual void WriteToNbt(TagCompound& tag);

    virtual void Update();
    virtual void Render(riDevice* device);

private:
    static inline std::unordered_map<std::string, std::function<std::shared_ptr<TileEntity>(Chunk*, TagCompound&)>> s_EntityIDToFnMap;
};