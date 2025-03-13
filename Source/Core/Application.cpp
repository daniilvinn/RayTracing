#include "Application.h"

#include "Logging.h"

namespace RT {

	void Application::Init()
	{
		Logging::Init();

		LOG_INFO("Info test");
		LOG_WARN("Warning test");
		LOG_ERROR("Error test");
		LOG_CRITICAL("Critical test");
	}

	void Application::Run()
	{

	}

	void Application::Shutdown()
	{

	}

}