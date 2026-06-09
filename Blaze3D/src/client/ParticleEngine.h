#pragma once

#include <thirdparty/glm/ext.hpp>

enum ParticleRenderMode
{
	PARTICLE_RENDER_MODE_NORMAL,
	PARTICLE_RENDER_MODE_TERRAIN,
	PARTICLE_RENDER_MODE_TORCH,
	PARTICLE_RENDER_MODE_SMOKE,
	PARTICLE_RENDER_MODE_ITEM,
	PARTICLE_RENDER_MODE_EXPLOSION,
	PARTICLE_RENDER_MODE_COUNT
};

class World;
class riDevice;

struct Particle
{
	bool IsAlive;
	bool HasPhysics;
	bool IsGrounded;
	float TimeToLive;
	glm::vec3 Color;
	glm::vec3 Position;
	glm::vec3 LastPosition;
	glm::vec3 Velocity;
	float Size;
	int Age;
	int LifeTimeInTicks;
	ParticleRenderMode RenderMode = PARTICLE_RENDER_MODE_NORMAL;
	union RenderModeUnion
	{
		struct TerrainType
		{
			uint8_t BlockId;
			uint8_t Face;
			float UvFracX;
			float UvFracY;
		} RenderModeTerrain;
		struct ItemType
		{
			uint16_t ItemId;
		} RenderModeItem;
		struct NormalType
		{
			void* Texture;
			uint16_t uvStartX, uvStartY;
			uint16_t uvEndX, uvEndY;
		} RenderModeNormal;
	} RenderData;
};

class ParticleEngine
{
public:
	ParticleEngine(World* world);
	~ParticleEngine();
	void Update();
	void Tick();
	void Render(riDevice* device);
	Particle& InitTerrainParticle(float x, float y, float z, float velX, float velY, float velZ, float size, int blockId);
	Particle& InitTorchFireParticle(float x, float y, float z);
	Particle& InitSmokeParticle(float x, float y, float z);
	Particle& InitExplosionParticle(float x, float y, float z);
private:
	Particle& AddParticle(Particle& particle);
	Particle* m_Particles;
	World* m_World;
	uint16_t m_SearchStartIndex = 0;
};