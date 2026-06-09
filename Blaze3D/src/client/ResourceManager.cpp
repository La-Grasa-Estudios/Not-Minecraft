#include "ResourceManager.h"

#include <engine/Audio.h>
#include <engine/RenderingInterface.h>

riHandle g_Textures[256];

void ResourceManager::Init(riContext* ctx)
{
	g_Textures[E_TextureResource_Panorama01]	= ctx->LoadTexture("mc/textures/panorama_day_01.rtex");
	g_Textures[E_TextureResource_Panorama02]	= ctx->LoadTexture("mc/textures/panorama_day_02.rtex");
	g_Textures[E_TextureResource_Terrain]		= ctx->LoadTexture("mc/textures/terrain.png");
	g_Textures[E_TextureResource_Particles]		= ctx->LoadTexture("mc/textures/particles.png");
	g_Textures[E_TextureResource_Clouds]		= ctx->LoadTexture("mc/textures/environment/clouds.png");
	g_Textures[E_TextureResource_Player]		= ctx->LoadTexture("mc/textures/mob/char.png");
	g_Textures[E_TextureResource_GuiControls]	= ctx->LoadTexture("mc/textures/gui/gui.png");
	g_Textures[E_TextureResource_GuiInventory]	= ctx->LoadTexture("mc/textures/gui/inventory.png");
	g_Textures[E_TextureResource_GuiCrafting]	= ctx->LoadTexture("mc/textures/gui/crafting.png");
	g_Textures[E_TextureResource_GuiFurnace]	= ctx->LoadTexture("mc/textures/gui/furnace.png");
	g_Textures[E_TextureResource_GuiContainer]	= ctx->LoadTexture("mc/textures/gui/container.png");
	g_Textures[E_TextureResource_GuiIcons]		= ctx->LoadTexture("mc/textures/gui/icons.png");
	g_Textures[E_TextureResource_GuiDirt]		= ctx->LoadTexture("mc/textures/gui/background.png");
	g_Textures[E_TextureResource_Items]			= ctx->LoadTexture("mc/textures/gui/items.png");
	g_Textures[E_TextureResource_Logo]			= ctx->LoadTexture("mc/textures/logo.rtex");
	// g_Textures[E_TextureResource_Mojang]		= ctx->LoadTexture("mc/textures/intro/mojang.rtex");
	// g_Textures[E_TextureResource_Eula]		= ctx->LoadTexture("mc/textures/intro/eula.rtex");
	g_Textures[E_TextureResource_Arrow]			= ctx->LoadTexture("mc/textures/item/arrows.png");
	g_Textures[E_TextureResource_Font]			= ctx->LoadTexture("mc/textures/default.rtex");

	ctx->SetTextureFilter(g_Textures[E_TextureResource_Mojang], RI_FILTER_LINEAR);
	ctx->SetTextureFilter(g_Textures[E_TextureResource_Particles], RI_FILTER_NEAREST);
	ctx->SetTextureFilter(g_Textures[E_TextureResource_Arrow], RI_FILTER_NEAREST);

	AudioEngine::RegisterSound("misc.pop", "mc/sound/random/pop/pop-01.wav");
	AudioEngine::RegisterSound("misc.pop", "mc/sound/random/pop/pop-02.wav");
	AudioEngine::RegisterSound("misc.pop", "mc/sound/random/pop/pop-03.wav");
	AudioEngine::RegisterSound("misc.pop", "mc/sound/random/pop/pop-04.wav");
	AudioEngine::RegisterSound("misc.splash", "mc/sound/random/splash/splash-01.wav");
	AudioEngine::RegisterSound("misc.splash", "mc/sound/random/splash/splash-02.wav");
	AudioEngine::RegisterSound("misc.splash", "mc/sound/random/splash/splash-03.wav");
	AudioEngine::RegisterSound("misc.splash", "mc/sound/random/splash/splash-04.wav");
	AudioEngine::RegisterSound("misc.click", "mc/sound/random/click.wav");
	AudioEngine::RegisterSound("misc.focus.slow", "mc/sound/random/focus_slow.wav");
	AudioEngine::RegisterSound("misc.focus.fast", "mc/sound/random/focus_fast.wav");
	AudioEngine::RegisterSound("misc.hurt", "mc/sound/random/hurt/hurt-01.wav");
	AudioEngine::RegisterSound("misc.hurt", "mc/sound/random/hurt/hurt-02.wav");
	AudioEngine::RegisterSound("misc.hurt", "mc/sound/random/hurt/hurt-03.wav");
	AudioEngine::RegisterSound("misc.break", "mc/sound/random/break.wav");
	AudioEngine::RegisterSound("misc.fizz", "mc/sound/random/fizz.wav");
	AudioEngine::RegisterSound("misc.fire", "mc/sound/fire/fire.wav");
	AudioEngine::RegisterSound("misc.ignite", "mc/sound/fire/ignite.wav");
	AudioEngine::RegisterSound("misc.lava.pop", "mc/sound/liquid/lavapop.wav");
	AudioEngine::RegisterSound("random.drr", "mc/sound/random/drr.wav");
	AudioEngine::RegisterSound("random.bow", "mc/sound/random/bow.wav");
	AudioEngine::RegisterSound("random.explode", "mc/sound/random/explode/explode-01.wav");
	AudioEngine::RegisterSound("random.explode", "mc/sound/random/explode/explode-02.wav");
	AudioEngine::RegisterSound("random.explode", "mc/sound/random/explode/explode-03.wav");

	AudioEngine::RegisterSound("random.fuse", "mc/sound/random/fuse/fuse-01.wav");
	AudioEngine::RegisterSound("random.fuse", "mc/sound/random/fuse/fuse-02.wav");
	AudioEngine::RegisterSound("random.fuse", "mc/sound/random/fuse/fuse-03.wav");

	AudioEngine::RegisterSound("misc.portal", "mc/sound/portal/portal.wav");
	AudioEngine::RegisterSound("misc.portal.travel", "mc/sound/portal/travel.wav");
	AudioEngine::RegisterSound("misc.portal.trigger", "mc/sound/portal/trigger.wav");

	AudioEngine::RegisterSound("block.door.open", "mc/sound/door_open.wav");
	AudioEngine::RegisterSound("block.door.close", "mc/sound/door_close.wav");

	AudioEngine::RegisterSound("mob.cow", "mc/sound/mob/cow1.wav");
	AudioEngine::RegisterSound("mob.cow", "mc/sound/mob/cow2.wav");
	AudioEngine::RegisterSound("mob.cow", "mc/sound/mob/cow3.wav");
	AudioEngine::RegisterSound("mob.cow", "mc/sound/mob/cow4.wav");
	AudioEngine::RegisterSound("mob.cowhurt", "mc/sound/mob/cowhurt1.wav");
	AudioEngine::RegisterSound("mob.cowhurt", "mc/sound/mob/cowhurt2.wav");
	AudioEngine::RegisterSound("mob.cowhurt", "mc/sound/mob/cowhurt3.wav");

	AudioEngine::RegisterSound("mob.pig", "mc/sound/mob/pig1.wav");
	AudioEngine::RegisterSound("mob.pig", "mc/sound/mob/pig2.wav");
	AudioEngine::RegisterSound("mob.pig", "mc/sound/mob/pig3.wav");
	AudioEngine::RegisterSound("mob.pigdeath", "mc/sound/mob/pigdeath.wav");

	AudioEngine::RegisterSound("mob.chicken", "mc/sound/mob/chicken1.wav");
	AudioEngine::RegisterSound("mob.chicken", "mc/sound/mob/chicken2.wav");
	AudioEngine::RegisterSound("mob.chicken", "mc/sound/mob/chicken3.wav");
	AudioEngine::RegisterSound("mob.chickenhurt", "mc/sound/mob/chickenhurt1.wav");
	AudioEngine::RegisterSound("mob.chickenhurt", "mc/sound/mob/chickenhurt2.wav");
	AudioEngine::RegisterSound("mob.chickenplop", "mc/sound/mob/chickenplop.wav");

	AudioEngine::RegisterSound("mob.zombie", "mc/sound/mob/zombie1.wav");
	AudioEngine::RegisterSound("mob.zombie", "mc/sound/mob/zombie2.wav");
	AudioEngine::RegisterSound("mob.zombie", "mc/sound/mob/zombie3.wav");
	AudioEngine::RegisterSound("mob.zombiehurt", "mc/sound/mob/zombiehurt1.wav");
	AudioEngine::RegisterSound("mob.zombiehurt", "mc/sound/mob/zombiehurt2.wav");
	AudioEngine::RegisterSound("mob.zombiedeath", "mc/sound/mob/zombiedeath.wav");

	AudioEngine::RegisterSound("mob.zombiepig", "mc/sound/mob/zombiepig/zpig1.wav");
	AudioEngine::RegisterSound("mob.zombiepig", "mc/sound/mob/zombiepig/zpig2.wav");
	AudioEngine::RegisterSound("mob.zombiepig", "mc/sound/mob/zombiepig/zpig3.wav");
	AudioEngine::RegisterSound("mob.zombiepig", "mc/sound/mob/zombiepig/zpig4.wav");
	AudioEngine::RegisterSound("mob.zombiepig.angry", "mc/sound/mob/zombiepig/zpigangry1.wav");
	AudioEngine::RegisterSound("mob.zombiepig.angry", "mc/sound/mob/zombiepig/zpigangry2.wav");
	AudioEngine::RegisterSound("mob.zombiepig.angry", "mc/sound/mob/zombiepig/zpigangry3.wav");
	AudioEngine::RegisterSound("mob.zombiepig.angry", "mc/sound/mob/zombiepig/zpigangry4.wav");
	AudioEngine::RegisterSound("mob.zombiepighurt", "mc/sound/mob/zombiepig/zpighurt1.wav");
	AudioEngine::RegisterSound("mob.zombiepighurt", "mc/sound/mob/zombiepig/zpighurt2.wav");
	AudioEngine::RegisterSound("mob.zombiepigdeath", "mc/sound/mob/zombiepig/zpigdeath.wav");

	AudioEngine::RegisterSound("mob.creeper.hurt", "mc/sound/mob/creeper/creeper1.wav");
	AudioEngine::RegisterSound("mob.creeper.hurt", "mc/sound/mob/creeper/creeper2.wav");
	AudioEngine::RegisterSound("mob.creeper.hurt", "mc/sound/mob/creeper/creeper3.wav");
	AudioEngine::RegisterSound("mob.creeper.hurt", "mc/sound/mob/creeper/creeper4.wav");
	AudioEngine::RegisterSound("mob.creeper.death", "mc/sound/mob/creeper/creeperdeath.wav");
	AudioEngine::RegisterSound("mob.creeper.fuse", "mc/sound/mob/creeper/fuse.wav");

	AudioEngine::RegisterMusic("title", "mc/sound/title/menu-01.mp3");
	AudioEngine::RegisterMusic("title", "mc/sound/title/menu-02.mp3");
	AudioEngine::RegisterMusic("title", "mc/sound/title/menu-03.mp3");
	AudioEngine::RegisterMusic("title", "mc/sound/title/menu-04.mp3");
}

void ResourceManager::BindTexture(TextureResource textureRsc, riDevice* device)
{
	device->SetTexture(g_Textures[textureRsc], 0);
}

void* ResourceManager::GetHandle(TextureResource textureRsc)
{
	return g_Textures[textureRsc];
}
