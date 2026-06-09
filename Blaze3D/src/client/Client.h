#pragma once

#include "engine/Common.h"
#include "engine/Frustum.h"

#include "common/WorldCommon.h"
#include "common/Timer.h"
#include "client/ui/IngameInterface.h"
#include "SPWorldAccess.h"
#include "PlayerHandler.h"

#include "thirdparty/glm/ext.hpp"

#include <memory>
#include <string>

struct Camera
{
	glm::vec3 Position;
	glm::vec3 Rotation;
	float Fov = 90.0f;
};

class PlayerEntity;
class riContext;
class riDevice;
class World;
class WorldRenderer;
class ParticleEngine;
class ScreenBase;
class ResourceManager;

enum ClientState
{
	CLIENT_STATE_GAME_MENU,
	CLIENT_STATE_INGAME,
};

class Client
{
public:
	static inline Client& GetInstance()
	{
		static Client instance;
		return instance;
	}
	void Init(riContext* context);

	void Update();
	void Render();
	void Close();

	ResourceManager* pResourceManager;

	glm::mat4 ViewMatrix;
	Frustum CurrentFrustum;

	ParticleEngine* ParticleManager;

	Timer UpdateTimer{ 20.0f };
	std::shared_ptr<PlayerEntity> Player;

	glm::vec2 UiScreenSize;
	glm::vec2 ScreenSize;
	glm::vec2 CursorPosition;

	void RenderBlockOutline();
	void RenderBlockCracks(int phase);
	void SetScreen(std::shared_ptr<ScreenBase> screen);

	void RenderLoadingScreen(std::string_view text);
	void RenderLoadingScreenWithBar(std::string_view text, float progress);

	void DoPlayerRespawn();

	void Connect();
	void LoadLevel(const std::string& path);
	void CloseLevel();

	void SwitchLevel(World* newWorld);

	ScreenBase* GetScreen();

	bool CloseFlag = false;

	riDevice* GraphicsDevice;
	riContext* GraphicsContext;

	int Ticks = 0;

	RayCastHit RayHit;
	bool DidHitBlock = false;

	IngameInterface IngameUI{ nullptr, nullptr };
	World* WorldObj;

private:

	void RenderDebugScreen();

	void UpdateGame();
	void RenderGame();

	std::shared_ptr<ScreenBase> m_CurrentScreen;

	void RenderSkybox(float b);
	void SetupGui();
	void SetupCamera();

	void UpdateMenu();
	void RenderMenu();


	WorldRenderer* m_WorldRenderer;
	SPWorldAccess* m_WorldAccess;
	PlayerHandler* m_PlayerHandler;

	Camera m_Camera;
	ClientState m_ClientState;
	ClientState m_NextClientState;

};