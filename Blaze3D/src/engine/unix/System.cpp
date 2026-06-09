#include <chrono>

#include <thread>

#include <SDL3/SDL.h>

#include "../System.h"

static long long start = 0.0f;

static float vMouseX = 0.0f;
static float vMouseY = 0.0f;
static float vLastMouseX = 0.0f;
static float vLastMouseY = 0.0f;

static bool Focused = false;
static bool MouseGrabbed = false;
static bool MousePrimary = false;
static bool MouseSecondary = false;

int scrollY = 0;

void SDL_EventCB(SDL_Event& e)
{
	if (e.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
		Focused = true;
	if (e.type == SDL_EVENT_WINDOW_FOCUS_LOST)
		Focused = false;
	if (e.type == SDL_EVENT_MOUSE_MOTION)
	{
		vMouseX = e.motion.x;
		vMouseY = e.motion.y;
		vLastMouseX += e.motion.xrel;
		vLastMouseY += e.motion.yrel;
	}
	if (e.type == SDL_EVENT_MOUSE_BUTTON_UP || e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	{
		if (e.button.button == 1)
			MousePrimary = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
		if (e.button.button == 3)
			MouseSecondary = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
	}
	if (e.type == SDL_EVENT_MOUSE_WHEEL)
	{
		scrollY = -e.wheel.y;
	}
}

void sysInit()
{
	
}

void sysClose()
{

}

void sysCreateThread(void* (*fn)(void*))
{
	std::thread t([fn]
		{
			fn(NULL);
		});
	t.detach();
}

void sysYield()
{
	std::this_thread::yield();
}

float sysTime()
{
	std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();
	auto since_epoch = begin.time_since_epoch();
	auto time = std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count();
	if (start == 0.0)
	{
		start = time;
	}
	return ((float)(time - start) / 1000.0f / 1000.0f);
}

sysInputData sysGetInputData()
{
	static sysInputData lastData{};
	sysInputData data{};

	data.VDeltaX = vLastMouseX;
	data.VDeltaY = vLastMouseY;

	vLastMouseX = 0.0f;
	vLastMouseY = 0.0f;

	if (!Focused)
	{
		data.VDeltaX = 0.0f;
		data.VDeltaY = 0.0f;
	}

	data.VMouseX = vMouseX;
	data.VMouseY = vMouseY;

	int numKeys;
	auto state = SDL_GetKeyboardState(&numKeys);

	if (state[SDL_SCANCODE_W])
	{
		data.MovementAxisY -= 1.0f;
	}
	if (state[SDL_SCANCODE_S])
	{
		data.MovementAxisY += 1.0f;
	}
	if (state[SDL_SCANCODE_A])
	{
		data.MovementAxisX -= 1.0f;
	}
	if (state[SDL_SCANCODE_D])
	{
		data.MovementAxisX += 1.0f;
	}

	data.MovementAxisJump = state[SDL_SCANCODE_SPACE];
	data.MainAction = MousePrimary;
	data.SecundaryAction = MouseSecondary;

	data.MainActionDown = data.MainAction && !lastData.MainAction;
	data.SecundaryActionDown = data.SecundaryAction && !lastData.SecundaryAction;
	data.UiUpPressed = data.MovementAxisY < -0.5f && !(lastData.MovementAxisY < -0.5f);
	data.UiDownPressed = data.MovementAxisY > 0.5f && !(lastData.MovementAxisY > 0.5f);
	data.UiLeftPressed = data.MovementAxisX < -0.5f && !(lastData.MovementAxisX < -0.5f);
	data.UiRightPressed = data.MovementAxisX > 0.5f && !(lastData.MovementAxisX > 0.5f);
	data.UiMainPressed = data.MainActionDown;
	data.UiSecondaryPressed = data.SecundaryActionDown;

	data.ChangeItemLeftDown = state[SDL_SCANCODE_V] && !lastData.ChangeItemLeftDown;
	data.ChangeItemRightDown = state[SDL_SCANCODE_B] && !lastData.ChangeItemRightDown;
	data.BackActionDown = state[SDL_SCANCODE_ESCAPE] && !lastData.BackActionDown;
	data.InventoryActionDown = state[SDL_SCANCODE_E] && !lastData.InventoryActionDown;

	data.ShiftAction = state[SDL_SCANCODE_LSHIFT];

	lastData = data;

	lastData.ChangeItemLeftDown = state[SDL_SCANCODE_V];
	lastData.ChangeItemRightDown = state[SDL_SCANCODE_B];
	lastData.BackActionDown = state[SDL_SCANCODE_ESCAPE];
	lastData.InventoryActionDown = state[SDL_SCANCODE_E];

	if (scrollY > 0)
	{
		data.ChangeItemRightDown = true;
	}
	else if (scrollY < 0)
	{
		data.ChangeItemLeftDown = true;
	}

	scrollY = 0;

	return data;
}

void sysSetVMousePosition(float x, float y)
{
	SDL_WarpMouseInWindow(SDL_GL_GetCurrentWindow(), x, y);
	// glfwSetCursorPos(glfwGetCurrentContext(), x, y);
}

void sysSetVMouseHidden(bool hidden)
{
	MouseGrabbed = hidden;
	if (hidden)
	{
		SDL_HideCursor();
	}
	else
	{
		SDL_ShowCursor();
	}
	SDL_SetWindowRelativeMouseMode(SDL_GL_GetCurrentWindow(), hidden);
	//SDL_SetWindowMouseGrab(SDL_GL_GetCurrentWindow(), (SDL_bool)hidden);
	// glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_HIDDEN);
}

std::string sysPath(const std::string& path)
{
	return path;
}

uint32_t bswap32(uint32_t v)
{
	return v;
}
