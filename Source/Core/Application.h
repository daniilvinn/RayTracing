#pragma once

#include "Window.h"

#include <functional>
#include <stack>

namespace RT {

	class Application {
	public:

		/*
		*  @brief Launches an application. Initializes render context, sets up resources
		*/
		void Init();

		/*
		*  @brief Launches main loop
		*/
		void Run();

		/*
		*  @brief Shuts application down. Clears resources, writes log file
		*/
		void Shutdown();

		void EnqueueObjectFinalize(std::function<void()> Exec);

		void RequestEngineExit();

	public:
		static Application* const Instance;

	private:

		Ptr<Window> ApplicationWindow = nullptr;
		bool Initialized = true;
		bool Running = false;

		std::stack<std::function<void()>> FinalizationQueue;

	};

}