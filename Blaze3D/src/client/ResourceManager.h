#pragma once

enum TextureResource
{
	E_TextureResource_Terrain,
	E_TextureResource_Items,
	E_TextureResource_Clouds,
	E_TextureResource_Particles,
	E_TextureResource_Player,
	E_TextureResource_Arrow,
	E_TextureResource_GuiControls,
	E_TextureResource_GuiDirt,
	E_TextureResource_GuiInventory,
	E_TextureResource_GuiCrafting,
	E_TextureResource_GuiFurnace,
	E_TextureResource_GuiContainer,
	E_TextureResource_Font,
	E_TextureResource_Logo,
	E_TextureResource_Mojang,
	E_TextureResource_Eula,
	E_TextureResource_Panorama01,
	E_TextureResource_Panorama02,
	E_TextureResource_GuiIcons,
};

class riContext;
class riDevice;

class ResourceManager
{
public:
	void Init(riContext* ctx);
	void BindTexture(TextureResource textureRsc, riDevice* device);
	void* GetHandle(TextureResource textureRsc);
};