#pragma once

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

	};

}