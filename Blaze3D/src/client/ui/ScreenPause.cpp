#include "ScreenPause.h"

#include "client/Client.h"

ScreenPause::ScreenPause(riDevice* device)
{
	m_Device = device;
}

void ScreenPause::Render(glm::vec2 sz)
{
	Begin();
	DrawGradient(sz / 2.0f, sz, { 0.5f, 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f, 0.0f });
	if (DrawButton(sz / 2.0f - glm::vec2(0.0f, 15.0f), {200, 20}, "Resume"))
	{
		Client::GetInstance().SetScreen(NULL);
		return;
	}
	if (DrawButton(sz / 2.0f - glm::vec2(0.0f, -15.0f), { 200, 20 }, "Quit"))
	{
		Client::GetInstance().CloseLevel();
		return;
	}
	End();
}

bool ScreenPause::DoesGuiPauseGame()
{
	return true;
}
