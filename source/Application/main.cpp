#include "D3DGame.h"
#include "D3DAppException.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace Library;
using namespace Application;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	/*
	Force the main thread to always run on CPU 0.
	This is done because on some systems QueryPerformanceCounter returns a bit different counter values
	on the different CPUs (contrary to what it's supposed to do), which can cause negative frame times
	if the thread is scheduled on the other CPU in the next frame. This can cause very jerky behavior and
	appear as if frames return out of order.
	*/
	SetThreadAffinityMask(GetCurrentThread(), 1);

	// Create a console window so that we can use it as our logging output
	AllocConsole();

	// Redirect all standard output to console (i.e. printf, cout etc).
	freopen("CONOUT$", "a", stdout);

	D3DGame app(instance, L"D3D Application", L"D3D Engine", showCommand);

	try
	{
		app.Initialize();
		app.Run();
	}
	catch (D3DAppException ex)
	{
		MessageBox(app.WindowHandle(), ex.whatw().c_str(), app.WindowTitle().c_str(), MB_ABORTRETRYIGNORE);
	}

	return 0;
}
