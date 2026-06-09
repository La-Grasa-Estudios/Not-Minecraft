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

#define VAL_TYPE long double

VAL_TYPE factorial(VAL_TYPE n)
{
	VAL_TYPE r = 1;
	for (VAL_TYPE i = 1; i <= n; i++)
	{
		r *= i;
	}
	return r;
}

uint64_t permutacion(VAL_TYPE n, VAL_TYPE k)
{
	VAL_TYPE nfac = factorial(n);
	VAL_TYPE div = factorial(n - k);
	return static_cast<uint64_t>(nfac / div);
}

uint64_t combinacion(VAL_TYPE n, VAL_TYPE k)
{
	VAL_TYPE nfac = factorial(n);
	VAL_TYPE div = factorial(n - k) * factorial(k);
	return static_cast<uint64_t>(nfac / div);
}

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
