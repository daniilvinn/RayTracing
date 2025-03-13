#pragma once

#include "Application.h"

#include <Windows.h>

using namespace RT;

#ifdef TEMP_RT_RELEASE
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
#else
int main()
#endif
{
	
	Application RTApplication;

	RTApplication.Init();
	RTApplication.Run();
	RTApplication.Shutdown();

}
