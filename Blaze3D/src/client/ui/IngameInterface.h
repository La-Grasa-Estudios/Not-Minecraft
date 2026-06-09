#pragma once

class PlayerEntity;
class riDevice;

class IngameInterface
{
public:
	IngameInterface(PlayerEntity* player, riDevice* device);
	void Render();
	bool DoPlaceSwing = false;
	bool DoDigSwing = false;
	bool IsAnimating = false;
private:
	PlayerEntity* m_Player;
	riDevice* m_Device;
};