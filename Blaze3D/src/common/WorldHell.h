#pragma once

#include "World.h"

class WorldHell : public World
{
public:
	WorldHell(const std::string& path);
	virtual ChunkProvider& GetChunkProvider();
	virtual int GetDimensionID();
	virtual bool HasSky();
protected:
	void InitWorld() override;
};