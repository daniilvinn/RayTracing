#pragma once

#include "Common.h"

#include "ICoreSystem.h"

struct GLFWwindow; // Forward declaration

namespace RT {

	struct WindowConfig {
		WindowConfig()
			: Width(800)
			, Height(600)
		{}

		u32 Width;
		u32 Height;
		std::string Title;
	};


	/*
	*  @brief An application window class
	*/
	class Window : public ICoreSystem {
	public:
		Window(const WindowConfig& InConfig) 
			: Config(InConfig)
			, WindowHandle(nullptr)
			, IsMinimized(false)
		{
			Initialize(nullptr);
		}

		~Window() {
			Shutdown();
		}

		void Initialize(void* InputData) override;
		void Shutdown() override;

		void PollEvents();

	private:
		WindowConfig Config;
		GLFWwindow* WindowHandle;
		bool IsMinimized;

	};

}