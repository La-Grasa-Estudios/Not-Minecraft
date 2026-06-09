#include <iostream>
#include "engine/memory/MemoryAllocator.h"
#include "engine/RenderingInterface.h"
#include "engine/Time.h"
#include "engine/System.h"
#include "engine/Audio.h"
#include "thirdparty/glm/ext.hpp"

#include "client/Client.h"
#include "common/item/Item.h"

#include <cstdint>
#include <thread>

#include <Util.h>

int main(int argc, const char* argv[])
{
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++)
	{
		args.push_back(std::string(argv[i]));
	}

	auto context = sysNew<riContext>();

	context->Init();
	sysInit();
	AudioEngine::Init();

	Block::InitBlocks();
	Item::Init();

	Client::GetInstance().Init(context);

	while (!context->ShouldClose() && !Client::GetInstance().CloseFlag)
	{
		Time::BeginProfile();
		Client::GetInstance().Update();
		Client::GetInstance().Render();

		context->SwapBuffers();

		Time::EndProfile();
	}
	Client::GetInstance().Close();

	sysClose();
}
