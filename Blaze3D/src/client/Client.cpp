#include "Client.h"

#include "engine/RenderingInterface.h"
#include "engine/memory/MemoryAllocator.h"
#include "engine/ScaledResolution.h"
#include "engine/Font.h"
#include "engine/Time.h"
#include "engine/Audio.h"

#include "common/World.h"
#include "common/WorldHell.h"
#include "common/Random.h"
#include "common/item/Item.h"
#include "common/entity/PlayerEntity.h"
#include "common/SaveManager.h"
#include "common/Teleporter.h"

#include "client/renderer/VertexProducer.h"
#include "client/ui/ScreenPause.h"
#include "client/ui/ScreenInventory.h"
#include "client/ui/ScreenMainMenu.h"

#include "renderer/WorldRenderer.h"
#include "renderer/RenderHelper.h"
#include "renderer/AnimatedWaterTexture.h"
#include "renderer/AnimatedLavaTexture.h"
#include "renderer/AnimatedFireTexture.h"
#include "renderer/AnimatedPortalTexture.h"

#include "SPPlayerHandler.h"
#include "ParticleEngine.h"
#include "MusicManager.h"
#include "ResourceManager.h"
#include "Profiler.h"

#include <fstream>

sysInputData g_InputData;
extern VertexProducer producer;

Random g_Random;

AnimatedWaterTexture g_WaterTexture;
AnimatedWaterFlowTexture g_WaterFlowTexture;
AnimatedLavaTexture g_LavaTexture;
AnimatedLavaFlowTexture g_LavaFlowTexture;
AnimatedFireTexture g_FireTexture;
AnimatedPortalTexture g_PortalTexture;

void Client::Init(riContext* context)
{
	GraphicsContext = context;
	GraphicsDevice = sysNew<riDevice>();
	GraphicsDevice->Init(context);

	pResourceManager = new ResourceManager();
	pResourceManager->Init(GraphicsContext);

	riVertexAttributeTable vat{};

	// Chunks
	vat.Position[0].Enabled = true;
	vat.Position[0].Format = RI_VERTEX_FORMAT_FLOAT3;
	vat.TexCoord[0].Enabled = true;
	vat.TexCoord[0].Format = RI_VERTEX_FORMAT_FLOAT2;
	vat.TexCoord[0].Offset = sizeof(float) * 3;
	vat.Color[0].Enabled = true;
	vat.Color[0].Format = RI_VERTEX_FORMAT_UINT8_4;
	vat.Color[0].Offset = sizeof(float) * 5;

	// Lines
	vat.Position[1].Enabled = true;
	vat.Position[1].Format = RI_VERTEX_FORMAT_FLOAT3;
	vat.Color[1].Enabled = true;
	vat.Color[1].Format = RI_VERTEX_FORMAT_UINT8_4;
	vat.Color[1].Offset = sizeof(float) * 3;

	// With normals
	vat.Position[2].Enabled = true;
	vat.Position[2].Format = RI_VERTEX_FORMAT_FLOAT3;
	vat.TexCoord[2].Enabled = true;
	vat.TexCoord[2].Format = RI_VERTEX_FORMAT_FLOAT2;
	vat.TexCoord[2].Offset = sizeof(float) * 3;
	vat.TexCoord[2].Enabled = true;
	vat.Color[2].Enabled = true;
	vat.Color[2].Format = RI_VERTEX_FORMAT_UINT8_4;
	vat.Color[2].Offset = sizeof(float) * 5;
	vat.Normal[2].Format = RI_VERTEX_FORMAT_UINT8_4;
	vat.Normal[2].Offset = sizeof(float) * 6;
	vat.Normal[2].Enabled = true;

	context->SetVertexAttributeTable(vat);
	Font::Init(GraphicsDevice);

	RenderLoadingScreen("Loading game");	

	MusicManager::Init();

	m_ClientState = CLIENT_STATE_GAME_MENU;
	m_NextClientState = CLIENT_STATE_INGAME;

	m_CurrentScreen = NULL;
	WorldObj = NULL;
	m_WorldRenderer = NULL;
	Player = NULL;
	ViewMatrix = {};
	CurrentFrustum = {};
	ParticleManager = NULL;

	sysSetVMousePosition(GraphicsContext->Width / 2.0f, GraphicsContext->Height / 2.0f);
	AudioEngine::PlayMusic("title", 0.6f);

	GraphicsDevice->SetBlending(true);
}

static float debugTickTime = 0.0f;

void Client::Update()
{
	if (m_NextClientState == CLIENT_STATE_GAME_MENU)
	{
		m_NextClientState = CLIENT_STATE_INGAME;
		m_ClientState = CLIENT_STATE_GAME_MENU;

		m_WorldRenderer->Cleanup();

		WorldObj->Save([this](float progress) {
			RenderLoadingScreenWithBar("Saving", progress);
			});
		WorldObj->Close();

		Player = NULL; // Automatically transfers ownership to World, freed when we delete the world

		delete ParticleManager;
		delete m_WorldRenderer;
		delete WorldObj;
		delete m_WorldAccess;
		delete m_PlayerHandler;

		m_CurrentScreen = NULL;
		WorldObj = NULL;
		m_WorldRenderer = NULL;
		m_WorldAccess = NULL;
		m_PlayerHandler = NULL;
		ViewMatrix = {};
		CurrentFrustum = {};
		ParticleManager = NULL;
		sysSetVMousePosition(GraphicsContext->Width / 2.0f, GraphicsContext->Height / 2.0f);
	}

	Profiler::Clear();
	Profiler::BeginSection("Update");

	if (m_ClientState == CLIENT_STATE_INGAME)
	{
		UpdateGame();
	}
	else if (m_ClientState == CLIENT_STATE_GAME_MENU)
	{
		UpdateMenu();
	}
}

void Client::Render()
{
	ScreenSize = { GraphicsContext->Width, GraphicsContext->Height };
	GraphicsDevice->SetViewport(0.0f, 0.0f, static_cast<float>(GraphicsContext->Width), static_cast<float>(GraphicsContext->Height));

	if (m_ClientState == CLIENT_STATE_INGAME)
	{
		RenderGame();
	} 
	else if (m_ClientState == CLIENT_STATE_GAME_MENU)
	{
		RenderMenu();
	}
}

constexpr int uiScaleFactor = 0;

void Client::Close()
{
	if (m_ClientState == CLIENT_STATE_INGAME)
	{
		CloseLevel();
		Update();
	}
}

void Client::SetScreen(std::shared_ptr<ScreenBase> screen)
{
	g_InputData = {};

	if (screen && (!m_CurrentScreen && m_ClientState != CLIENT_STATE_GAME_MENU))
	{
		sysSetVMouseHidden(false);
		sysSetVMousePosition(GraphicsContext->Width / 2.0f, GraphicsContext->Height / 2.0f);
	}

	m_CurrentScreen = screen;
}

void Client::RenderLoadingScreen(std::string_view text)
{
	GraphicsContext->SetSync(false);
	SetupGui();

	GraphicsDevice->SetDepthTest(false, RI_COMPARISON_MODE_LESS_EQUAL);

	producer.Reset();
	producer.SetColor(0.6f, 0.6f, 0.6f, 1.0f);
	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f, UiScreenSize, {}, UiScreenSize / 16.0f, producer);

	GraphicsDevice->SetTexturing(true);
	pResourceManager->BindTexture(E_TextureResource_GuiDirt, GraphicsDevice);
	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	GraphicsDevice->SetTexturing(false);

	Font::DrawCenteredString(text, UiScreenSize.x / 2.0f, UiScreenSize.y / 2.0f - 10, 0xFFFFFFFF);

	RenderDebugScreen();

	GraphicsDevice->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);

	GraphicsContext->SwapBuffers();
	GraphicsContext->SetSync(true);
}

void Client::RenderLoadingScreenWithBar(std::string_view text, float progress)
{
	GraphicsContext->SetSync(false);
	SetupGui();

	GraphicsDevice->SetDepthTest(false, RI_COMPARISON_MODE_LESS_EQUAL);

	producer.Reset();
	producer.SetColor(0.8f, 0.8f, 0.8f, 1.0f);
	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f, UiScreenSize, {}, UiScreenSize / 16.0f, producer);

	GraphicsDevice->SetTexturing(true);
	pResourceManager->BindTexture(E_TextureResource_GuiDirt, GraphicsDevice);
	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	GraphicsDevice->SetTexturing(false);

	producer.Reset();
	producer.SetColor(0.1f, 0.1f, 0.1f, 1.0f);
	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f + glm::vec2(0, 10), { 80, 2 }, {}, {}, producer);
	producer.SetColor(0.1f, 0.8f, 0.1f, 1.0f);
	RenderHelper::RenderTexturedRect(UiScreenSize / 2.0f + glm::vec2(-40, 9), { 80 * progress, 2 }, {}, {}, producer);

	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	Font::DrawCenteredString(text, UiScreenSize.x / 2.0f, UiScreenSize.y / 2.0f - 10, 0xFFFFFFFF);

	RenderDebugScreen();

	GraphicsDevice->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);

	GraphicsContext->SwapBuffers();
	GraphicsContext->SetSync(true);
}

void Client::DoPlayerRespawn()
{
	Player->AddHealth(20);
	Player->RemainingFireTicks = 0;
	Player->SetPosition({ WorldObj->SpawnX + 0.5f, WorldObj->SpawnY + Player->HeightOffset, WorldObj->SpawnZ + 0.5f });
	if (Player->Dimension != 0)
	{
		Player->Dimension = 0;
		SwitchLevel(new World(WorldObj->GetSaveManager().GetSavePath()));
	}
	WorldObj->RequestChunksSurroundingArea(static_cast<int>(Player->Position.x), static_cast<int>(Player->Position.z), 4, true);
	bool validSpawn = false;

	int spawnX = WorldObj->SpawnX;
	int spawnZ = WorldObj->SpawnZ;
	int attempts = 100;

	while (!validSpawn && attempts > 0)
	{
		attempts--;

		int y = WorldObj->GetHeight(spawnX, spawnZ) - 1;
		if (WorldObj->GetBlock(spawnX, y, spawnZ).GetDef()->IsFullyOpaqueBlock())
		{
			validSpawn = true;
			Player->SetPosition({ spawnX + 0.5f, static_cast<float>(y + 1) + Player->HeightOffset, spawnZ + 0.5f });
		}
		else
		{
			spawnX = WorldObj->SpawnX;
			spawnZ = WorldObj->SpawnZ;
			spawnX += g_Random.NextInt(32) - 16;
			spawnZ += g_Random.NextInt(32) - 16;
		}
	}

	SetScreen(nullptr);
}

void Client::Connect()
{
	
}

void Client::LoadLevel(const std::string& path)
{
	AudioEngine::StopMusic();

	m_ClientState = CLIENT_STATE_INGAME;

	auto world = new World(path);

	std::shared_ptr<PlayerEntity> player = std::make_shared<PlayerEntity>(world);
	player->SetPosition({ 0, 129.0f, 0 });

	world->AddEntity(player);

	if (world->GetSaveManager().Load(world) && player->Dimension == -1)
	{
		delete world;
		world = new WorldHell(path);
		player->EntityWorld = world;
		world->AddEntity(player);
	}

	IngameUI = IngameInterface(player.get(), GraphicsDevice);

	Player = player;
	m_WorldAccess = new SPWorldAccess(world, m_WorldRenderer);
	world->WorldAccess = m_WorldAccess;

	bool didLoad = false;

	if (!world->Load([this](float progress) {
		RenderLoadingScreenWithBar("Loading", progress);
		}))
	{
		world->Generate();
	}
	else
	{
		m_Camera.Rotation = Player->Rotation;
		didLoad = true;
	}

	SwitchLevel(world);

	if (!didLoad)
	{
		DoPlayerRespawn();
	}

	m_CurrentScreen = NULL;
}

void Client::CloseLevel()
{
	m_NextClientState = CLIENT_STATE_GAME_MENU;
}

// Only used to switch levels, to exit use CloseLevel otherwise may cause memory leaks or data loss
void Client::SwitchLevel(World* newWorld)
{
	if (WorldObj)
	{
		m_WorldRenderer->Cleanup();

		delete ParticleManager;
		delete m_WorldRenderer;
		delete WorldObj;
		delete m_WorldAccess;
		delete m_PlayerHandler;

		m_CurrentScreen = NULL;
		WorldObj = NULL;
		m_WorldRenderer = NULL;
		m_WorldAccess = NULL;
		m_PlayerHandler = NULL;
		ViewMatrix = {};
		CurrentFrustum = {};
		ParticleManager = NULL;
	}
	else // We probably come from LoadLevel, return
	{
		m_WorldRenderer = new WorldRenderer(newWorld, GraphicsContext);
		ParticleManager = new ParticleEngine(newWorld);
		if (m_WorldAccess)
		{
			delete m_WorldAccess;
		}
		m_WorldAccess = new SPWorldAccess(newWorld, m_WorldRenderer);
		m_PlayerHandler = new SPPlayerHandler(newWorld);
		newWorld->WorldAccess = m_WorldAccess;
		WorldObj = newWorld;
		return;
	}

	if (newWorld)
	{
		m_WorldRenderer = new WorldRenderer(newWorld, GraphicsContext);
		ParticleManager = new ParticleEngine(newWorld);
		m_WorldAccess = new SPWorldAccess(newWorld, m_WorldRenderer);
		m_PlayerHandler = new SPPlayerHandler(newWorld);
		newWorld->WorldAccess = m_WorldAccess;
		Player->EntityWorld = newWorld;
		WorldObj = newWorld;
		WorldObj->AddEntity(Player);
	}
	else
	{
		m_ClientState = CLIENT_STATE_GAME_MENU;
	}
}

ScreenBase* Client::GetScreen()
{
	if (m_CurrentScreen)
	{
		return m_CurrentScreen.get();
	}
	return nullptr;
}

void Client::RenderDebugScreen()
{
	static int fpsCounter = 0;
	static float fpsTimer = 0;
	static int fps = 0;
	int py = -8;
	char debugBuffer[255];

	fpsCounter++;
	if ((fpsTimer += Time::UnscaledDeltaTime) > 1.0f)
	{
		fps = fpsCounter;
		fpsTimer = 0.0f;
		fpsCounter = 0;
	}

	snprintf(debugBuffer, 255, "Not Minecraft - %i FPS", fps);
	Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);

	if (Player && WorldObj)
	{
		auto forward = Player->GetForward();
		auto angle = glm::atan(forward.x, -forward.z) * 180.0f / glm::pi<float>();
		if (angle < 0.0f)
			angle += 360.0f;

		int meta = -1;
		if (DidHitBlock)
		{
			meta = WorldObj->GetBlock(RayHit.HitBlock.x, RayHit.HitBlock.y, RayHit.HitBlock.z).Metadata;
		}

		int px = (int)floor(Player->Position.x);
		int pz = (int)floor(Player->Position.z);

		snprintf(debugBuffer, 255, "%.3f MS RegionCache: %i", Time::UnscaledDeltaTime * 1000.0f, (int)WorldObj->GetSaveManager().GetCacheSize());
		Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);

		snprintf(debugBuffer, 255, "E: %i X: %.2f Y: %.2f Z: %.2f", (int)WorldObj->GetEntityCount(), Player->Position.x, Player->Position.y - Player->HeightOffset, Player->Position.z);
		Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);

		snprintf(debugBuffer, 255, "%.2fms per tick A: %.2f F: %i M: %i", debugTickTime * 1000.0f, angle, Player->GetEntityFacingDirection(), meta);
		Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);
	}

	//auto& sections = Profiler::GetSections();
	//
	//for (int i = 0; i < sections.size(); i++)
	//{
	//	snprintf(debugBuffer, 255, "%s - %.2fms", sections[i].Name, sections[i].Duration * 1000.0f);
	//	Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);
	//}
	
}

void Client::UpdateGame()
{
	MusicManager::Update();

	static auto lastInput = sysGetInputData();
	auto input = sysGetInputData();

	static uint8_t mineCooldown = 0;
	static BaseEntity* pickedEntity = nullptr;

	DidHitBlock = WorldObj->RayCast(Player->Position, Player->GetForward(), 5.0f, RayHit);

	g_InputData = input;
	
	Time::TimeScale = 1.0f;

	if (!m_CurrentScreen)
	{
		m_Camera.Rotation.y += input.VDeltaX * 0.1f;
		m_Camera.Rotation.x += input.VDeltaY * 0.1f;

		m_Camera.Rotation.x = glm::clamp(m_Camera.Rotation.x, -89.0f, 89.0f);

		while (m_Camera.Rotation.y < 0.0f)
		{
			m_Camera.Rotation.y += 360.0f;
		}

		while (m_Camera.Rotation.y > 360.0f)
		{
			m_Camera.Rotation.y -= 360.0f;
		}

		Player->inputState = input;
	}
	else
	{
		Player->inputState = {};
		input.MainAction = false;
		input.MainActionDown = false;
		input.SecundaryAction = false;
		input.SecundaryActionDown = false;
		input.ChangeItemLeftDown = false;
		input.ChangeItemRightDown = false;
	}

	lastInput = input;

	Player->LastRotation = Player->Rotation;
	Player->Rotation = m_Camera.Rotation;
	AudioEngine::SetListener(Player->Position, -Player->GetForward());
	AudioEngine::Update();

	UpdateTimer.advanceTime();

	pickedEntity = WorldObj->GetEntityWithRayCast(Player.get(), Player->Position, Player->GetForward(), 5.0f);

	if (pickedEntity)
	{
		DidHitBlock = false;
		RayHit.Face = -1;
	}

	Profiler::EndAndBeginSection("Tick");
	for (int i = 0; i < UpdateTimer.ticks; i++)
	{
		Ticks++;

		float tickStart = sysTime();
		WorldObj->Update();

		int renderX = ((int)glm::floor(Player->Position.x) >> 4) << 4;
		int renderZ = ((int)glm::floor(Player->Position.z) >> 4) << 4;
		m_WorldRenderer->MarkNewPositionForRender(renderX, renderZ);

		ParticleManager->Tick();

		if (input.MainAction)
		{
			m_PlayerHandler->MainAction(RayHit);
		}

		auto& chunkMap = WorldObj->GetChunkMap();

		Random rng;
		for (auto& kv : chunkMap)
		{
			Chunk* chunk = kv.second.get();
			
			float distance = glm::distance(glm::vec3(chunk->GetStartX() + 8.0f, Client::GetInstance().Player->Position.y, chunk->GetStartZ() + 8.0f), Client::GetInstance().Player->Position);

			const int updates = 768;

			for (int i = 0; i < updates && chunk->GenStage == TERRAIN_GEN_COMPLETE && distance < 40.0f; i++) {
				int x = rng.NextInt(CHUNK_WIDTH) + chunk->GetStartX();
				int y = rng.NextInt(32) - 16;
				int z = rng.NextInt(CHUNK_WIDTH) + chunk->GetStartZ();

				y += (int)Client::GetInstance().Player->Position.y;

				Block* block = chunk->GetBlockGlobal(x, y, z).GetDef();
				if (block) {
					block->DisplayTick(WorldObj, x, y, z);
				}
			}
		}

		g_WaterTexture.Animate();
		g_WaterFlowTexture.Animate();
		g_LavaTexture.Animate();
		g_LavaFlowTexture.Animate();
		g_FireTexture.Animate();
		g_PortalTexture.Animate();

		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 208, 192, 16, 16, g_WaterTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 224, 192, 16, 16, g_WaterFlowTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 240, 192, 16, 16, g_WaterFlowTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 224, 208, 16, 16, g_WaterFlowTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 240, 208, 16, 16, g_WaterFlowTexture.Buffer);

		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 208, 224, 16, 16, g_LavaTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 224, 224, 16, 16, g_LavaFlowTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 240, 224, 16, 16, g_LavaFlowTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 224, 240, 16, 16, g_LavaFlowTexture.Buffer);
		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 240, 240, 16, 16, g_LavaFlowTexture.Buffer);

		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 240, 16, 16, 16, g_FireTexture.Buffer);

		GraphicsContext->UpdateTexturePart(pResourceManager->GetHandle(E_TextureResource_Terrain), 224, 0, 16, 16, g_PortalTexture.GpuBuffer);

		m_PlayerHandler->Tick();

		if (Player->NetherPortalTicks == 80)
		{
			Teleporter().TeleportEntity(Player.get());
			Player->NetherPortalTicks = 100;
			AudioEngine::PlaySound("misc.portal.travel", 0.1f);
		}

		debugTickTime = sysTime() - tickStart;
	}

	if (input.MainActionDown && pickedEntity)
	{
		Player->Attack(pickedEntity, 1);
	}

	if (input.SecundaryAction)
	{
		m_PlayerHandler->SecondaryAction(RayHit);
	}
	else
	{
		m_PlayerHandler->SecondaryActionUnPressed();
	}
	if (!input.MainAction)
	{
		m_PlayerHandler->MainActionUnPressed();
	}

	auto& inv = Player->Inventory;
	if (g_InputData.SecundaryActionDown)
	{
		auto item = Item::GetItem(inv.Resources[Player->HotbarSlot].Id);
		if (item)
		{
			if (item->TryUseOnEntity(Player.get(), WorldObj, (int)floor(Player->Position.x), (int)floor(Player->Position.y), (int)floor(Player->Position.z)))
			{
				inv.Resources[Player->HotbarSlot].Shrink(1);
			}
		}
	}

	{
		Profiler::EndAndBeginSection("Particles");
		ParticleManager->Update();
	}

	m_Camera.Position = Player->Interpolate(Player->LastPosition, Player->Position, UpdateTimer.a);

	if (input.ChangeItemLeftDown)
		Player->HotbarSlot--;
	if (input.ChangeItemRightDown)
		Player->HotbarSlot++;

	if (Player->HotbarSlot >= 9)
		Player->HotbarSlot = 0;
	if (Player->HotbarSlot < 0)
		Player->HotbarSlot = 8;

	if (!m_CurrentScreen)
	{
		if (g_InputData.InventoryActionDown)
		{
			SetScreen(std::make_shared<ScreenInventory>(GraphicsDevice));
		}
		if (g_InputData.BackActionDown)
		{
			SetScreen(std::make_shared<ScreenPause>(GraphicsDevice));

			WorldObj->GetSaveManager().Save(WorldObj);
		}
	}
}

void Client::RenderGame()
{
	SetupCamera();

	float fr = 0.5f;
	float fg = 0.8f;
	float fb = 1.0f;

	float celestialAngle = WorldObj->GetCelestialAngle(UpdateTimer.a);
	float brightness = glm::cos(celestialAngle * glm::pi<float>() * 2.0f) * 2.0f + 0.5f;
	brightness = glm::clamp(brightness, 0.0f, 1.0f);

	if (!WorldObj->HasSky())
	{
		brightness = 0.0f;
		fr = 1.0f;
		fg = 0.5f;
		fb = 0.4f;
	}

	glm::vec3 dayColor = { fr, fg, fb };
	glm::vec3 nightColor = { 0.05f, 0.05f, 0.1f };

	if (!WorldObj->HasSky())
	{
		nightColor = { 0.1f, 0.05f, 0.05f };
	}

	glm::vec3 finalColor = glm::mix(dayColor, nightColor, 1.0f - brightness);

	GraphicsDevice->ClearSurface(GraphicsContext->GetSurfaceHandle(), finalColor.r, finalColor.g, finalColor.b, 0.0f, true, true);
	GraphicsDevice->SetFogColor(finalColor.r, finalColor.g, finalColor.b);

	RenderSkybox(brightness);

	GraphicsDevice->SetModelViewMatrix(glm::value_ptr(ViewMatrix));

	pResourceManager->BindTexture(E_TextureResource_Terrain, GraphicsDevice);
	m_WorldRenderer->Render(GraphicsDevice, brightness);

	auto model = glm::mat4(1.0f);
	GraphicsDevice->SetModelViewMatrix(glm::value_ptr(ViewMatrix));

	Profiler::EndAndBeginSection("RenderParticles");
	ParticleManager->Render(GraphicsDevice);

	Profiler::EndAndBeginSection("RenderCracks");
	if (DidHitBlock)
	{
		if (m_PlayerHandler->GetDigProgress() > 0.0f)
		{
			float progress = m_PlayerHandler->GetDigProgress();
			progress = 1.0f - progress;
			progress = (float)(int)(progress * 9.0f);

			RenderBlockCracks(static_cast<int>(progress));
		}
	}

	Profiler::EndAndBeginSection("RenderUI");
	IngameUI.Render();

	SetupGui();

	// Fire overlay
	if (Player->IsBurning())
	{
		auto minUv = glm::vec2(240, 16) / 256.0f;
		auto maxUv = glm::vec2(256, 32) / 256.0f;
		producer.Reset();
		producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

		RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f + glm::vec2(-UiScreenSize.x / 2.0f, UiScreenSize.y / 4.0f), UiScreenSize, minUv, maxUv, producer);
		RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f + glm::vec2(UiScreenSize.x / 2.0f, UiScreenSize.y / 4.0f), UiScreenSize, minUv, maxUv, producer);

		pResourceManager->BindTexture(E_TextureResource_Terrain, GraphicsDevice);
		GraphicsDevice->SetTexturing(true);
		GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		GraphicsDevice->SetTexturing(false);
	}

	if (Player->NetherPortalTicks > 0)
	{
		float a = Player->NetherPortalTicks / 70.0f;
		a = glm::clamp(a, 0.0f, 1.0f);
		a *= a;
		producer.Reset();
		producer.SetColor(1.0f, 1.0f, 1.0f, a);

		RenderHelper::RenderBlockOverlay({}, UiScreenSize, Block::GetBlock(BLOCK_PORTAL)->GetTextureForFace(0, NULL, 0, 0, 0), producer);

		pResourceManager->BindTexture(E_TextureResource_Terrain, GraphicsDevice);
		GraphicsDevice->SetTexturing(true);
		GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		GraphicsDevice->SetTexturing(false);
	}

	if (Block::GetBlock(Player->GetBlockAtEyePos())->IsFullyOpaqueBlock())
	{
		producer.Reset();
		producer.SetColor(0.2f, 0.2f, 0.2f, 1.0f);

		RenderHelper::RenderBlockOverlay({}, UiScreenSize, Block::GetBlock(Player->GetBlockAtEyePos())->GetTextureForFace(0, NULL, 0, 0, 0), producer);

		pResourceManager->BindTexture(E_TextureResource_Terrain, GraphicsDevice);
		GraphicsDevice->SetTexturing(true);
		GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		GraphicsDevice->SetTexturing(false);
	}

	producer.Reset();

	auto maxUv = glm::vec2(182, 22) / 256.0f;
	auto selectBoxMinUv = glm::vec2(0, 22) / 256.0f;
	auto selectBoxMaxUv = glm::vec2(23, 45) / 256.0f;

	producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	const float scale = 1.0f;

	RenderHelper::RenderCenteredTexturedRect({ UiScreenSize.x / 2.0f, UiScreenSize.y - 11 }, { 182 * scale, 22 * scale }, {}, maxUv, producer);
	RenderHelper::RenderCenteredTexturedRect({ UiScreenSize.x / 2.0f - (80 + (-Player->HotbarSlot * 20)) * scale, UiScreenSize.y - 11 }, { 24 * scale, 24 * scale }, selectBoxMinUv, selectBoxMaxUv, producer);

	pResourceManager->BindTexture(E_TextureResource_GuiControls, GraphicsDevice);
	GraphicsDevice->SetTexturing(true);
	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	for (int i = 0; i < 9; i++)
	{
		glm::vec2 pos = { UiScreenSize.x / 2.0f - (80 + (-i * 20)) * scale - 0.5f, UiScreenSize.y - 13 };
		RenderHelper::RenderItemStack(&Player->Inventory.Resources[i], pos, producer, GraphicsDevice);
	}

	producer.Reset();
	producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	GraphicsDevice->SetBlendMode(BLEND_MODE_INV_DST_COLOR, BLEND_MODE_INV_SRC_COLOR);
	pResourceManager->BindTexture(E_TextureResource_GuiControls, GraphicsDevice);
	GraphicsDevice->SetTexturing(true);
	RenderHelper::RenderCenteredTexturedRect({ UiScreenSize.x / 2.0f, UiScreenSize.y / 2.0f }, { 8, 8 }, glm::vec2{ 243, 3 } / 256.0f, glm::vec2{ 252, 12 } / 256.0f, producer);
	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	GraphicsDevice->SetBlendMode(BLEND_MODE_SRC_ALPHA, BLEND_MODE_INV_SRC_ALPHA);

	producer.Reset();
	producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	bool damageOutline = Player->ImmunityTicks % 4;

	for (int i = 0; i < 10; i++)
	{
		float x = UiScreenSize.x / 2.0f - 86.0f + i * 8.0f;
		float y = UiScreenSize.y - 27.5f;

		if (Player->Health <= 6)
		{
			y -= g_Random.NextInt(2);
		}

		// Full hearth
		RenderHelper::RenderCenteredTexturedRect({ x, y }, { 9, 9 }, glm::vec2{ 16 + damageOutline * 9, 0 } / 256.0f, glm::vec2{ 16 + damageOutline * 9 + 9, 9 } / 256.0f, producer);

		if (i * 2 + 1 < Player->Health)
		{
			RenderHelper::RenderCenteredTexturedRect({ x, y }, { 9, 9 }, glm::vec2{ 52, 0 } / 256.0f, glm::vec2{ 61, 9 } / 256.0f, producer);
		}
		if (i * 2 + 1 == Player->Health)
		{
			RenderHelper::RenderCenteredTexturedRect({ x, y }, { 9, 9 }, glm::vec2{ 61, 0 } / 256.0f, glm::vec2{ 70, 9 } / 256.0f, producer);
		}

	}

	int totalArmor = Player->Inventory.GetArmorValue();

	for (int i = 0; i < 10 && totalArmor > 0; i++)
	{
		float x = UiScreenSize.x / 2.0f - 86.0f + i * 8.0f;
		float y = UiScreenSize.y - 27.5f - 10;
		// Full armor icon
		RenderHelper::RenderCenteredTexturedRect({ x, y }, { 9, 9 }, glm::vec2{ 16, 9 } / 256.0f, glm::vec2{ 16 + 9, 18 } / 256.0f, producer);

		if (i * 2 + 1 < totalArmor)
		{
			RenderHelper::RenderCenteredTexturedRect({ x, y }, { 9, 9 }, glm::vec2{ 34, 9 } / 256.0f, glm::vec2{ 43, 18 } / 256.0f, producer);
		}
		if (i * 2 + 1 == totalArmor)
		{
			RenderHelper::RenderCenteredTexturedRect({ x, y }, { 9, 9 }, glm::vec2{ 34, 9 } / 256.0f, glm::vec2{ 25, 18 } / 256.0f, producer);
		}

	}

	pResourceManager->BindTexture(E_TextureResource_GuiIcons, GraphicsDevice);
	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	RenderDebugScreen();

	if (!m_CurrentScreen)
	{
		sysSetVMouseHidden(true);
	}
	else
	{
		sysSetVMouseHidden(false);
		m_CurrentScreen->Render(UiScreenSize);

		// glm::vec2 minUv = glm::vec2{ 220, 16 } / 256.0f;
		// glm::vec2 maxUv = glm::vec2{ 256, 52 } / 256.0f;
		// producer.Reset();
		// producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		// RenderHelper::RenderCenteredTexturedRect(CursorPosition, { 9.f, 9.f }, minUv, maxUv, producer);
		// 
		// pResourceManager->BindTexture(E_TextureResource_GuiControls, GraphicsDevice);
		// GraphicsDevice->SetTexturing(true);
		// GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		// GraphicsDevice->SetTexturing(false);
	}

	GraphicsDevice->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);
	Profiler::EndSection();
}

void Client::RenderSkybox(float b)
{
	Profiler::EndAndBeginSection("RenderSkybox");
	auto view = glm::mat4(1.0f);
	view = glm::rotate(view, glm::radians(m_Camera.Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(m_Camera.Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(m_Camera.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	GraphicsDevice->SetModelViewMatrix(glm::value_ptr(view));

	float voidColor[6]{
		// sky plane
		0.4f,
		0.7f,
		1.0f,
		// void plane
		0.1f,
		0.3f,
		0.9f,
	};

	float netherColor[6]{
		// sky plane
		0.0f,
		0.0f,
		0.0f,
		// void plane
		0.0f,
		0.0f,
		0.0f,
	};

	auto colors = voidColor;
	if (!WorldObj->HasSky())
	{
		colors = netherColor;
	}

	const float ySize = 16;

	producer.Reset();
	producer.ConcatenateTexCoord = false;
	producer.SetLightMultiplier(b);

	for (int xx = -1; xx <= 1; xx++) {
		for (int zz = -1; zz <= 1; zz++) {
			float x2 = -256 + xx * 256;
			float x3 = 256 + xx * 256;
			float y2 = -ySize;
			float y3 = y2 + ySize * 2;
			float z2 = -256 + zz * 256;
			float z3 = 256 + zz * 256;

			producer.SetColor(colors[0], colors[1], colors[2]);
			producer.AddVertex(x2, y3, z3);
			producer.AddVertex(x2, y3, z2);
			producer.AddVertex(x3, y3, z2);
			producer.AddVertex(x3, y3, z3);
			producer.SetColor(colors[3], colors[4], colors[5]);
			producer.AddVertex(x3, y2, z3);
			producer.AddVertex(x3, y2, z2);
			producer.AddVertex(x2, y2, z2);
			producer.AddVertex(x2, y2, z3);
		}
	}

	GraphicsDevice->SetDepthTest(false, RI_COMPARISON_MODE_LESS_EQUAL);
	GraphicsDevice->SetTexturing(false);
	GraphicsDevice->SetFog(true);
	GraphicsDevice->SetFogStart(0.0f);
	GraphicsDevice->SetFogEnd(256.0f);

	GraphicsDevice->Draw(producer.GetVertexPointer(), 1, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	GraphicsDevice->SetTexturing(true);
	GraphicsDevice->SetFog(false);
	GraphicsDevice->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);
}

void Client::SetupGui()
{
	ScaledResolution scaledRes(GraphicsContext->Width, GraphicsContext->Height, uiScaleFactor);

	UiScreenSize = { (float)scaledRes.getOrthoWidth(), (float)scaledRes.getOrthoHeight() };
	CursorPosition = { g_InputData.VMouseX, g_InputData.VMouseY };
	CursorPosition.x = (int)CursorPosition.x * (float)scaledRes.getOrthoWidth() / (float)GraphicsContext->Width;
	CursorPosition.y = (int)CursorPosition.y * (float)scaledRes.getOrthoHeight() / (float)GraphicsContext->Height - 1;

	auto proj = glm::ortho(0.0f, (float)scaledRes.getOrthoWidth(), (float)scaledRes.getOrthoHeight(), 0.0f, -1500.0f, 1500.0f);

	GraphicsDevice->SetProjectionMatrix(glm::value_ptr(proj));
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -100.0f));
	GraphicsDevice->SetModelViewMatrix(glm::value_ptr(model));
	GraphicsDevice->SetDepthTest(false, RI_COMPARISON_MODE_LESS_EQUAL);
}

void Client::RenderBlockOutline()
{
	if (RayHit.Face == -1)
		return;

	struct Vertex
	{
		glm::vec3 pos;
		uint32_t color;
	};

	const float epsilon = 0.002f; // small expansion

	AABB bb = WorldObj->GetBlock(RayHit.HitBlock.x, RayHit.HitBlock.y, RayHit.HitBlock.z).GetDef()->GetRaycastBoundingBox(WorldObj, RayHit.HitBlock.x, RayHit.HitBlock.y, RayHit.HitBlock.z);

	float minX = bb.minX - epsilon;
	float minY = bb.minY - epsilon;
	float minZ = bb.minZ - epsilon;
	float maxX = bb.maxX + epsilon;
	float maxY = bb.maxY + epsilon;
	float maxZ = bb.maxZ + epsilon;

	Vertex vertices[] = {
		// Bottom face
		{ { minX, minY, minZ }, bswap32(0xFF202020) }, // A
		{ { maxX, minY, minZ }, bswap32(0xFF202020) }, // B

		{ { maxX, minY, minZ }, bswap32(0xFF202020) }, // B
		{ { maxX, maxY, minZ }, bswap32(0xFF202020) }, // C

		{ { maxX, maxY, minZ }, bswap32(0xFF202020) }, // C
		{ { minX, maxY, minZ }, bswap32(0xFF202020) }, // D

		{ { minX, maxY, minZ }, bswap32(0xFF202020) }, // D
		{ { minX, minY, minZ }, bswap32(0xFF202020) }, // A

		// Top face
		{ { minX, minY, maxZ }, bswap32(0xFF202020) }, // E
		{ { maxX, minY, maxZ }, bswap32(0xFF202020) }, // F

		{ { maxX, minY, maxZ }, bswap32(0xFF202020) }, // F
		{ { maxX, maxY, maxZ }, bswap32(0xFF202020) }, // G

		{ { maxX, maxY, maxZ }, bswap32(0xFF202020) }, // G
		{ { minX, maxY, maxZ }, bswap32(0xFF202020) }, // H

		{ { minX, maxY, maxZ }, bswap32(0xFF202020) }, // H
		{ { minX, minY, maxZ }, bswap32(0xFF202020) }, // E

		// Vertical edges
		{ { minX, minY, minZ }, bswap32(0xFF202020) }, // A
		{ { minX, minY, maxZ }, bswap32(0xFF202020) }, // E

		{ { maxX, minY, minZ }, bswap32(0xFF202020) }, // B
		{ { maxX, minY, maxZ }, bswap32(0xFF202020) }, // F

		{ { maxX, maxY, minZ }, bswap32(0xFF202020) }, // C
		{ { maxX, maxY, maxZ }, bswap32(0xFF202020) }, // G

		{ { minX, maxY, minZ }, bswap32(0xFF202020) }, // D
		{ { minX, maxY, maxZ }, bswap32(0xFF202020) }, // H
	};

	const size_t count = sizeof(vertices) / sizeof(Vertex);

	GraphicsDevice->Draw(vertices, 1, RI_PRIMITIVE_TYPE_LINE_LIST, static_cast<uint32_t>(count));
}

void Client::RenderBlockCracks(int phase)
{
	if (RayHit.Face == -1)
		return;

	AABB bb = WorldObj->GetBlockBoundingBox(RayHit.HitBlock.x, RayHit.HitBlock.y, RayHit.HitBlock.z);

	float height = bb.maxY - bb.minY;

	float x3 = bb.minX - 0.003f;
	float y3 = bb.minY - 0.003f;
	float z3 = bb.minZ - 0.003f;
	float x4 = bb.maxX + 0.003f;
	float y4 = bb.maxY + 0.003f;
	float z4 = bb.maxZ + 0.003f;
	float tUnit = 1.0f / 16.0f;
	int texX = phase;
	int texY = 15;
	float crackTexY = texY * tUnit;
	float crackTexY2 = texY * tUnit;
	float crackTexX = texX * tUnit;

	producer.Reset();
	// face 1 (original first quad group)
	producer.SetColor(1.0f, 1.0f, 1.0f);
	producer.AddVertex(x3, y3, z3, crackTexX, crackTexY2);
	producer.AddVertex(x4, y3, z3, crackTexX + tUnit, crackTexY2);
	producer.AddVertex(x4, y3, z4, crackTexX + tUnit, crackTexY2 + tUnit);
	producer.AddVertex(x3, y3, z4, crackTexX, crackTexY2 + tUnit);

	// face 2 (next quad group)
	producer.AddVertex(x3, y4, z3, crackTexX, crackTexY2);
	producer.AddVertex(x4, y4, z3, crackTexX + tUnit, crackTexY2);
	producer.AddVertex(x4, y4, z4, crackTexX + tUnit, crackTexY2 + tUnit);
	producer.AddVertex(x3, y4, z4, crackTexX, crackTexY2 + tUnit);

	float tUnitY = tUnit * height;

	// face 3 (third quad group)
	producer.AddVertex(x3, y3, z3, crackTexX, crackTexY + tUnitY);
	producer.AddVertex(x4, y3, z3, crackTexX + tUnit, crackTexY + tUnitY);
	producer.AddVertex(x4, y4, z3, crackTexX + tUnit, crackTexY);
	producer.AddVertex(x3, y4, z3, crackTexX, crackTexY);

	// face 4 (fourth quad group)
	producer.AddVertex(x3, y3, z4, crackTexX, crackTexY + tUnitY);
	producer.AddVertex(x4, y3, z4, crackTexX + tUnit, crackTexY + tUnitY);
	producer.AddVertex(x4, y4, z4, crackTexX + tUnit, crackTexY);
	producer.AddVertex(x3, y4, z4, crackTexX, crackTexY);

	// face 5 (fifth quad group)
	producer.AddVertex(x3, y3, z3, crackTexX, crackTexY + tUnitY);
	producer.AddVertex(x3, y3, z4, crackTexX + tUnit, crackTexY + tUnitY);
	producer.AddVertex(x3, y4, z4, crackTexX + tUnit, crackTexY);
	producer.AddVertex(x3, y4, z3, crackTexX, crackTexY);

	// face 6 (sixth quad group)
	producer.AddVertex(x4, y3, z3, crackTexX, crackTexY + tUnitY);
	producer.AddVertex(x4, y3, z4, crackTexX + tUnit, crackTexY + tUnitY);
	producer.AddVertex(x4, y4, z4, crackTexX + tUnit, crackTexY);
	producer.AddVertex(x4, y4, z3, crackTexX, crackTexY);

	GraphicsDevice->SetBlendMode(BLEND_MODE_DST_COLOR, BLEND_MODE_SRC_COLOR);
	GraphicsDevice->SetCullMode(RI_CULL_MODE_NONE);
	GraphicsDevice->SetTexturing(true);
	pResourceManager->BindTexture(E_TextureResource_Terrain, GraphicsDevice);
	GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	GraphicsDevice->SetCullMode(RI_CULL_MODE_BACK);
	GraphicsDevice->SetBlendMode(BLEND_MODE_SRC_ALPHA, BLEND_MODE_INV_SRC_ALPHA);
}

void Client::SetupCamera()
{
	// TO DO: Adjustable FOV and game settings
	auto proj = glm::perspective(glm::radians(70.0f), GraphicsContext->Width / (float)GraphicsContext->Height, 0.1f, 1000.0f);
	auto view = glm::mat4(1.0f);

	float portalWarp = Player->Interpolate((float)Player->LastNetherPortalTicks, (float)Player->NetherPortalTicks, UpdateTimer.a);
	float walkDistance = Player->Interpolate(Player->LastWalkDist, Player->WalkDist, UpdateTimer.a)*0.5f;
	float bobFactor = Player->Interpolate(Player->LastSwingFactor, Player->SwingFactor, UpdateTimer.a)*0.1f;

	if (portalWarp > 0.0f)
	{
		portalWarp /= 81.0f;
		portalWarp *= portalWarp;
		float factor = 5.0f / (portalWarp * portalWarp + 5.0f) - portalWarp * 0.04f;
		factor *= factor;
		float ticks = (Ticks + UpdateTimer.a) * 20;
		proj = glm::rotate(proj, glm::radians(ticks), glm::vec3(0.0f, 1.0f, 1.0f));
		proj = glm::scale(proj, { 1.0f / factor, 1.0f, 1.0f });
		proj = glm::rotate(proj, glm::radians(-ticks), glm::vec3(0.0f, 1.0f, 1.0f));
	}

	view = glm::translate(view, glm::vec3(glm::sin(walkDistance * glm::pi<float>()) * bobFactor * 0.5f, -glm::abs(glm::cos(walkDistance * glm::pi<float>()) * bobFactor), 0.0f));
	view = glm::rotate(view, glm::radians(glm::sin(walkDistance * glm::pi<float>()) * bobFactor * 3.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(glm::abs(glm::cos(walkDistance * glm::pi<float>() + 0.2f) * bobFactor) * 5.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	view = glm::rotate(view, glm::radians(-Player->GetCameraTilt(UpdateTimer.a)), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::rotate(view, glm::radians(m_Camera.Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(m_Camera.Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(m_Camera.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::translate(view, -m_Camera.Position);

	GraphicsDevice->SetProjectionMatrix(glm::value_ptr(proj));
	ViewMatrix = view;

	CurrentFrustum = Frustum(proj * view);
}

void Client::UpdateMenu()
{
	static auto lastInput = sysGetInputData();
	auto input = sysGetInputData();

	g_InputData = input;
}

void Client::RenderMenu()
{
	static float time = sysTime();

	static ScreenMainMenu mainMenu(GraphicsDevice);
	producer.Reset();

	GraphicsDevice->ClearSurface(GraphicsContext->GetSurfaceHandle(), 1.0f, 1.0f, 1.0f, 1.0f, true, true);

	GraphicsDevice->SetFog(false);
	GraphicsDevice->SetFogColor(0.0f, 0.0f, 0.0f);
	GraphicsDevice->SetFogStart(99999.0f); // For some reason wii renders with fog enabled
	GraphicsDevice->SetFogEnd(99999.0f);
	GraphicsDevice->SetCullMode(RI_CULL_MODE_NONE);
	GraphicsDevice->SetTexturing(true);
	GraphicsDevice->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);

	SetupGui();

	GraphicsDevice->SetBlendMode(BLEND_MODE_SRC_ALPHA, BLEND_MODE_INV_SRC_ALPHA);
	GraphicsDevice->SetAlphaTest(false);

	sysSetVMouseHidden(false);

	if (false && sysTime() < time + 6.0f)
	{
		// if (g_InputData.MainActionDown)
		// {
		// 	time -= 10.0f;
		// }
		// glm::vec2 minUv = glm::vec2{ 0, 0 };
		// glm::vec2 maxUv = glm::vec2{ 1, 1 };
		// producer.Reset();
		// producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		// glm::vec2 size = {};
		// 
		// riHandle texture = 0;
		// 
		// if (sysTime() < time + 2.0f) // Eula
		// {
		// 	texture = pResourceManager->GetHandle(E_TextureResource_Eula);
		// 	size = { 512, 218 };
		// 	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f, size / 2.5f, minUv, maxUv, producer);
		// } else if (sysTime() < time + 4.0f) // Studio
		// {
		// 	texture = pResourceManager->GetHandle(E_TextureResource_Mojang);
		// 	size = { 232, 224 };
		// 	minUv = { 280 / 512.0f, 32 / 256.0f };
		// 	maxUv = { 511.0f / 512.0f, 255.0f / 256.0f };
		// 	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f, size / 2.5f, minUv, maxUv, producer);
		// } else if (sysTime() < time + 6.0f) // Mojang
		// {
		// 	const float offset = 105.0f / 2.0f - 0.1f;
		// 	texture = pResourceManager->GetHandle(E_TextureResource_Mojang);
		// 	size = { 210, 119 };
		// 	minUv = { 0, 121 / 256.0f };
		// 	maxUv = { 210 / 512.0f, 240 / 256.0f };
		// 	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f + glm::vec2(offset, 0), size / 2.0f, minUv, maxUv, producer);
		// 	minUv = { 0, 0 };
		// 	maxUv = { 210 / 512.0f, 119 / 256.0f };
		// 	RenderHelper::RenderCenteredTexturedRect(UiScreenSize / 2.0f + glm::vec2(-offset, 0), size / 2.0f, minUv, maxUv, producer);
		// }
		// 
		// GraphicsDevice->SetTexture(texture, 0);
		// GraphicsDevice->SetTexturing(true);
		// GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		// GraphicsDevice->SetTexturing(false);
	}
	else
	{
		if (!m_CurrentScreen)
		{
			mainMenu.Render(UiScreenSize);
		}
		else
		{
			m_CurrentScreen->Render(UiScreenSize);
		}
	}

	char debugBuffer[255];
	int py = -8;

	snprintf(debugBuffer, 255, "Not Minecraft");
	Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);

	snprintf(debugBuffer, 255, "%.3f MS RAM: %.2fMb", Time::UnscaledDeltaTime * 1000.0f, sysMemoryUsage() / 1024.0f / 1024.0f);
	Font::DrawShadow(debugBuffer, 2, py += 10, 16777215);

	{
		// glm::vec2 minUv = glm::vec2{ 220, 16 } / 256.0f;
		// glm::vec2 maxUv = glm::vec2{ 256, 52 } / 256.0f;
		// producer.Reset();
		// producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		// RenderHelper::RenderCenteredTexturedRect(CursorPosition, { 9.f, 9.f }, minUv, maxUv, producer);
		// 
		// pResourceManager->BindTexture(E_TextureResource_GuiControls, GraphicsDevice);
		// GraphicsDevice->SetTexturing(true);
		// GraphicsDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		// GraphicsDevice->SetTexturing(false);
	}

	GraphicsDevice->SetDepthTest(true, RI_COMPARISON_MODE_LESS_EQUAL);
	GraphicsDevice->SetTexturing(false);
	GraphicsDevice->SetFog(false);
	GraphicsDevice->SetAlphaTest(true);
	GraphicsDevice->SetBlendMode(BLEND_MODE_SRC_ALPHA, BLEND_MODE_INV_SRC_ALPHA);
}