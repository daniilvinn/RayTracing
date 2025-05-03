#include "Application.h"

#include "Window.h"
#include "Logging.h"
#include "RHI/RHI.h"

namespace RT {

	Application* const Application::Instance = new Application;

	void Application::Init()
	{
		Logging::Init();
		LOG_INFO("Launching application");
		LOG_INFO("Entering initialization phase");

		WindowConfig AppWindowConfig = {};
		AppWindowConfig.Width = 1600u;
		AppWindowConfig.Height = 900u;
		AppWindowConfig.Title = "Editor";


		if (IN_DEBUG_BUILD) {
			AppWindowConfig.Title += " | Debug build";
		}

		ApplicationWindow = std::make_unique<Window>(AppWindowConfig);

		RHIConfig RenderConfig = {};
		RenderConfig.FramesInFlight = 2;
		RenderConfig.ApplicationWindow = ApplicationWindow.get();

		RHI::Instance->Initialize(&RenderConfig);

		Initialized &= ApplicationWindow->Initialized();
		Initialized &= RHI::Instance->Initialized();

		if (!Initialized) {
			LOG_CRITICAL("Failed to initialize engine, exiting...");
		}
		else {
			LOG_INFO("Initialization phase complete");
		}

		Running = Initialized;
	}

	void Application::Run()
	{
		if (Initialized) {
			LOG_INFO("Entering engine loop");
		}

		while (Running) 
		{
			ApplicationWindow->PollEvents();
		}

		if (Initialized) {
			LOG_INFO("Exiting engine loop");
		}
	}

	void Application::Shutdown()
	{
		LOG_INFO("Finalizing core objects");
		while (!FinalizationQueue.empty()) {
			auto Exec = FinalizationQueue.top();
			Exec();
			FinalizationQueue.pop();
		}

		LOG_INFO("Saving logs to '{}'", Logging::GetFileName().data());
		LOG_INFO("Exiting application");
	}

	void Application::EnqueueObjectFinalize(std::function<void()> Exec)
	{
		FinalizationQueue.push(Exec);
	}

	void Application::RequestEngineExit()
	{
		LOG_INFO("Requested engine exit");
		Running = false;
	}

}