#if defined(_WIN32) && !defined(CMAKE_BUILD)

#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#ifdef _DEBUG
#pragma comment(lib, "SDL3d.lib")
#else
#pragma comment(lib, "SDL3.lib")
#endif

#endif