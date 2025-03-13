#include "Application.h"

#include "Logging.h"

namespace RT {

	void Application::Init()
	{
		Logging::Init();

		Logging::Info("Info test");
		Logging::Warning("Warning test");
		Logging::Error("Error test");
		Logging::Critical("Critical test");
	}

	void Application::Run()
	{

	}

	void Application::Shutdown()
	{

	}

}