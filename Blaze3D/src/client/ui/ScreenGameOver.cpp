#include "ScreenGameOver.h"
#include "client/Client.h"

#include <engine/Font.h>

ScreenGameOver::ScreenGameOver(riDevice* device)
{
	m_Device = device;
}

void ScreenGameOver::Render(glm::vec2 sz)
{
	Begin();

	DrawGradient(sz / 2.0f, sz, { 1.0f, 0.0f, 0.0f, 0.65f }, { 1.0f, 0.0f, 0.0f, 0.0f });

	Font::DrawCenteredString("You died!", sz.x / 2.0f, sz.y / 2.0f - 60.0f, 0xFFFFFFFF);

	if (DrawButton(sz / 2.0f - glm::vec2(0.0f, 15.0f), { 200, 20 }, "Respawn"))
	{
		Client::GetInstance().DoPlayerRespawn();
		return;
	}
	if (DrawButton(sz / 2.0f - glm::vec2(0.0f, -15.0f), { 200, 20 }, "Title Screen"))
	{
		Client::GetInstance().CloseLevel();
		return;
	}

	End();
}
