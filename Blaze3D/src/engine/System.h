#pragma once

#include <cstdint>
#include <string>

struct sysInputData
{
	float VMouseX;
	float VMouseY;
	float VDeltaX;
	float VDeltaY;

	float MovementAxisX;
	float MovementAxisY;
	bool MovementAxisJump;

	bool ShiftAction;

	bool MainAction;
	bool SecundaryAction;

	bool MainActionDown;
	bool SecundaryActionDown;

	bool ChangeItemLeftDown;
	bool ChangeItemRightDown;

	bool BackActionDown;
	bool InventoryActionDown;

	bool UiUpPressed;
	bool UiDownPressed;
	bool UiLeftPressed;
	bool UiRightPressed;
	bool UiMainPressed;
	bool UiSecondaryPressed;
};

void sysInit();
void sysClose();
void sysCreateThread(void* (*fn)(void*));
void sysYield();
float sysTime();
sysInputData sysGetInputData();
void sysSetVMousePosition(float x, float y);
void sysSetVMouseHidden(bool hidden);
std::string sysPath(const std::string& p);

uint32_t bswap32(uint32_t v);