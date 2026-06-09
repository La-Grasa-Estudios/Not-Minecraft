#include "ScreenMainMenu.h"

#include "engine/System.h"
#include "engine/Font.h"
#include "engine/RenderingInterface.h"

#include "client/Client.h"

#include "ScreenWorldSelection.h"

ScreenMainMenu::ScreenMainMenu(riDevice* device)
{
	m_Device = device;
}

void ScreenMainMenu::Render(glm::vec2 sz)
{
	Begin();
	
	RenderMenuBase(sz);
	
	glm::mat4 model = glm::mat4(1.0f);
	
	float scale = 1.8f - glm::abs(glm::sin(sysTime() * glm::pi<float>() * 2.0f) * 0.1f);
	scale = scale * 100.0f / (Font::Width("Now in C++ 23!") + 48.0f);

	model = glm::translate(model, glm::vec3(sz.x / 2.0f + 100.0f, sz.y / 4.0f, -100.0f));
	model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(scale));

	m_Device->PushMatrixStack();
	m_Device->SetModelViewMatrix(glm::value_ptr(model));

	Font::DrawCenteredString("Now in C++ 23!", 0, 0, 0xFF00FFFF);
	m_Device->PopMatrixStack();

	if (DrawButton(sz / 2.0f + glm::vec2(0.0f, 10.0f), { 200, 20 }, "Play"))
	{
		Client::GetInstance().SetScreen(std::make_shared<ScreenWorldSelection>(m_Device));
	}
	if (DrawButton(sz / 2.0f + glm::vec2(0.0f, 35.0f), { 200, 20 }, "Quit"))
	{
		Client::GetInstance().CloseFlag = true;
	}
	End();
}
