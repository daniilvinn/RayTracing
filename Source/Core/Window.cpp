#include "Window.h"

#include "Logging.h"
#include "Application.h"

#include <Glfw/glfw3.h>

namespace RT {

	void Window::Initialize(void* InputData)
	{
		LOG_INFO("Initializing application window");
		LOG_INFO("Window config: ");
		LOG_INFO("  Width: {}", Config.Width);
		LOG_INFO("  Height: {}", Config.Height);
		LOG_INFO("  Title: {}", Config.Title);

		IsInitialized = glfwInit();

		if (!IsInitialized) {
			LOG_CRITICAL("Failed to initialize windowing system, exiting");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for Vulkan

		WindowHandle = glfwCreateWindow(
			Config.Width,
			Config.Height,
			Config.Title.c_str(),
			nullptr,
			nullptr
		);

		if (!WindowHandle) {
			LOG_CRITICAL("Failed to create application window, exiting...");
		}
		else {
			LOG_INFO("Initialized application window");
		}

		glfwSetWindowCloseCallback(WindowHandle, [](GLFWwindow* Handle) {
			Application::Instance->RequestEngineExit();
			});
	}

	void Window::Shutdown()
	{

	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

}