#include "RHI.h"

#include "Core/Common.h"
#include "Core/Logging.h"
#include "Core/Application.h"
#include "RHIDebug.h"
#include "RHIExtensionList.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <glfw/glfw3.h>
#include <spdlog/fmt/fmt.h>

namespace RT {
	RHI* const RHI::Instance = new RHI;

	void RHI::InitializeInstance()
	{
		LOG_INFO("Initializing Vulkan instance");

		volkInitialize();

		Application::Instance->EnqueueObjectFinalize([]() {
			volkFinalize();
		});

		VkApplicationInfo ApplicationInfo = {};
		ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		ApplicationInfo.apiVersion = VK_API_VERSION_1_4;
		ApplicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		ApplicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		ApplicationInfo.pEngineName = "RTCore";
		ApplicationInfo.pApplicationName = "RT Previewer";

		VkDebugUtilsMessengerCreateInfoEXT VulkanDebugCallbackCreateInfo = {};
		VulkanDebugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		VulkanDebugCallbackCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		VulkanDebugCallbackCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		VulkanDebugCallbackCreateInfo.pfnUserCallback = RHIDebug::DebugLog;

		RHIExtensionList InstanceExtensionList = GetInstanceExtensionList();

		VkInstanceCreateInfo InstanceCreateInfo = {};
		InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceCreateInfo.pNext = &VulkanDebugCallbackCreateInfo;
		InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
		InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensionList.Extensions.data();
		InstanceCreateInfo.enabledExtensionCount = static_cast<u32>(InstanceExtensionList.Extensions.size());
		InstanceCreateInfo.ppEnabledLayerNames = InstanceExtensionList.Layers.data();
		InstanceCreateInfo.enabledLayerCount = static_cast<u32>(InstanceExtensionList.Layers.size());

		VK_CHECK_RESULT(vkCreateInstance(&InstanceCreateInfo, nullptr, &VulkanInstance));

		IsInitialized = static_cast<bool>(VulkanInstance);

		if (IsInitialized) {
			LOG_INFO("Created Vulkan instance");
		}
		else {
			LOG_CRITICAL("Failed to create Vulkan instance");
			return;
		}

		Application::Instance->EnqueueObjectFinalize([InstanceCaptured = VulkanInstance]() {
			vkDestroyInstance(InstanceCaptured, nullptr);
		});

		volkLoadInstance(VulkanInstance);
		
	}

	void RHI::InitializeDebugger()
	{
		if (!IN_RELEASE_BUILD) {
			Debug.Initialize(VulkanInstance);
		}
	}

	void RHI::InitializeRenderDevice()
	{
		LOG_INFO("Initializing render device");

		u32 PhysicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDeviceCount, nullptr);

		std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
		vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDeviceCount, PhysicalDevices.data());

		LOG_INFO("Enumerating available render devices: ");
		for (auto& Device : PhysicalDevices) {
			VkPhysicalDeviceProperties DeviceProperties = {};
			vkGetPhysicalDeviceProperties(Device, &DeviceProperties);

			u32 DriverVersion = DeviceProperties.driverVersion;
			std::string DriverVersionCorrected = fmt::format(
				"{}.{}.{}.{}", 
				VK_API_VERSION_VARIANT(DriverVersion),
				VK_API_VERSION_MAJOR(DriverVersion), 
				VK_API_VERSION_MINOR(DriverVersion), 
				VK_API_VERSION_PATCH(DriverVersion)
			);

			LOG_INFO("  {} [{}]", DeviceProperties.deviceName, DriverVersionCorrected);

			if (CheckRenderDeviceSuitability(Device) && !PhysicalDevice) {
				PhysicalDevice = Device;
				PhysicalDeviceProperties = DeviceProperties;
			}
		}

		if (PhysicalDevice) {
			LOG_INFO("Selected render device: {}", PhysicalDeviceProperties.deviceName);
		}
		else {
			LOG_CRITICAL("No suitable render device found");
		}

		LOG_INFO("Starting render device setup");

		FetchQueueFamilyIndices();

		VkPhysicalDeviceFeatures DeviceFeatures = {};
		DeviceFeatures.samplerAnisotropy = true;

		VkPhysicalDeviceVulkan11Features Device11Features = {};
		Device11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		Device11Features.shaderDrawParameters = true;
		Device11Features.storageBuffer16BitAccess = true;
		Device11Features.storagePushConstant16 = true;
		Device11Features.variablePointers = true;
		Device11Features.variablePointersStorageBuffer = true;

		VkPhysicalDeviceVulkan12Features Device12Features = {};
		Device12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		Device12Features.pNext = &Device11Features;
		Device12Features.descriptorIndexing = true;
		Device12Features.shaderSampledImageArrayNonUniformIndexing = true;
		Device12Features.descriptorBindingPartiallyBound = true;
		Device12Features.runtimeDescriptorArray = true;
		Device12Features.scalarBlockLayout = true;
		Device12Features.bufferDeviceAddress = true;
		Device12Features.bufferDeviceAddressCaptureReplay = !IN_RELEASE_BUILD;
		Device12Features.shaderFloat16 = true;

		VkPhysicalDeviceVulkan13Features Device13Features = {};
		Device13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		Device13Features.pNext = &Device12Features;
		Device13Features.dynamicRendering = true;
		Device13Features.maintenance4 = true;
		Device13Features.synchronization2 = true;

		VkPhysicalDeviceFeatures2 DeviceFeatures2 = {};
		DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		DeviceFeatures2.features = DeviceFeatures;
		DeviceFeatures2.pNext = &Device13Features;

		const fp32 DefaultQueuePriority = 1.0f;

		VkDeviceQueueCreateInfo GraphicsQueueCreateInfo = {};
		GraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		GraphicsQueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
		GraphicsQueueCreateInfo.queueCount = 1;
		GraphicsQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices.Graphics;

		VkDeviceQueueCreateInfo AsyncComputeQueueCreateInfo = {};
		GraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		GraphicsQueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
		GraphicsQueueCreateInfo.queueCount = 1;
		GraphicsQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices.AsyncCompute;

		VkDeviceQueueCreateInfo AsyncTransferQueueCreateInfo = {};
		GraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		GraphicsQueueCreateInfo.pQueuePriorities = &DefaultQueuePriority;
		GraphicsQueueCreateInfo.queueCount = 1;
		GraphicsQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices.AsyncTransfer;

		std::vector<VkDeviceQueueCreateInfo> DeviceQueueCreateInfos = {
			GraphicsQueueCreateInfo,
			AsyncComputeQueueCreateInfo,
			AsyncTransferQueueCreateInfo
		};

		RHIExtensionList DeviceExtensionList = GetRenderDeviceExtensionList();

		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pNext = &DeviceFeatures2;
		device_create_info.pQueueCreateInfos = DeviceQueueCreateInfos.data();
		device_create_info.queueCreateInfoCount = static_cast<u32>(DeviceQueueCreateInfos.size());
		device_create_info.pEnabledFeatures = {};
		device_create_info.ppEnabledExtensionNames = DeviceExtensionList.Extensions.data();
		device_create_info.enabledExtensionCount = static_cast<u32>(DeviceExtensionList.Extensions.size());

		VK_CHECK_RESULT(vkCreateDevice(PhysicalDevice, &device_create_info, nullptr, &RenderDevice));

		IsInitialized &= static_cast<bool>(RenderDevice);

		if (!IsInitialized) {
			LOG_CRITICAL("Failed to setup render device");
		}
		else {
			LOG_INFO("Created render device with extensions:");

			for (const auto& ExtensionName : DeviceExtensionList.Extensions) {
				LOG_INFO("  {}", ExtensionName);
			}

			LOG_INFO("Initialized render device");

			Application::Instance->EnqueueObjectFinalize([DeviceCaptured = RenderDevice]() {
				vkDestroyDevice(DeviceCaptured, nullptr);
			});
		}
	}

	bool RHI::CheckRenderDeviceSuitability(VkPhysicalDevice Device)
	{
		bool AllExtensionsSupported = true;
		RHIExtensionList RequiredDeviceExtensions = GetRenderDeviceExtensionList();

		u32 DeviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(Device, nullptr, &DeviceExtensionCount, nullptr);

		std::vector<VkExtensionProperties> SupportedExtensions(DeviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(Device, nullptr, &DeviceExtensionCount, SupportedExtensions.data());

		for (const auto& Extension : RequiredDeviceExtensions.Extensions) {
			bool ExtensionSupported = false;

			for (u32 i = 0; i < SupportedExtensions.size(); i++) {
				VkExtensionProperties ExtensionProperties = SupportedExtensions[i];

				if (!strcmp(ExtensionProperties.extensionName, Extension)) {
					ExtensionSupported = true;
				}
			}

			AllExtensionsSupported &= ExtensionSupported;
		}

		return AllExtensionsSupported;
	}

	void RHI::FetchQueueFamilyIndices()
	{
		u32 FamilyPropertyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &FamilyPropertyCount, nullptr);

		std::vector<VkQueueFamilyProperties> FamilyProperties(FamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &FamilyPropertyCount, FamilyProperties.data());

		u32 FamilyIndex = 0;

		// Try find general queue that will be used for rendering
		for (const auto& FamilyProperty : FamilyProperties) {
			const u32 RequestedFlagBits = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

			if (FamilyProperty.queueFlags & RequestedFlagBits) {
				QueueFamilyIndices.Graphics = FamilyIndex;
				break;
			}
			FamilyIndex++;
		}
		FamilyIndex = 0;

		// Try find dedicated async compute queue
		for (const auto& FamilyProperty : FamilyProperties) {
			const u32 RequestedFlagBits = VK_QUEUE_COMPUTE_BIT;
			const u32 ExceptedFlagBits = VK_QUEUE_GRAPHICS_BIT;

			if ((FamilyProperty.queueFlags & RequestedFlagBits) && !(FamilyProperty.queueFlags & ExceptedFlagBits)) {
				QueueFamilyIndices.AsyncCompute = FamilyIndex;
				break;
			}
			FamilyIndex++;
		}
		FamilyIndex = 0;

		// Try find dedicated async transfer queue
		for (const auto& FamilyProperty : FamilyProperties) {
			const u32 RequestedFlagBits = VK_QUEUE_TRANSFER_BIT;
			const u32 ExceptedFlagBits = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

			if ((FamilyProperty.queueFlags & RequestedFlagBits) && !(FamilyProperty.queueFlags & ExceptedFlagBits)) {
				QueueFamilyIndices.AsyncTransfer = FamilyIndex;
				break;
			}
			FamilyIndex++;
		}
		FamilyIndex = 0;

		// Assign present queue to the graphics queue
		QueueFamilyIndices.Present = QueueFamilyIndices.Graphics;
	}

	void RHI::Initialize(void* InputData)
	{
		LOG_INFO("Initializing render hardware interface system");

		if (!InputData) {
			LOG_CRITICAL("No input data for RHI initializer provided");
			IsInitialized = false;
			return;
		}
		
		Config = *static_cast<RHIConfig*>(InputData);

		LOG_INFO("RHI config:");
		LOG_INFO("  Frames in flight: {}", Config.FramesInFlight);

		InitializeInstance();
		
		if (!IsInitialized) {
			return;
		}

		InitializeDebugger();

		InitializeRenderDevice();

		if (!IsInitialized) {
			return;
		}
	}

	void RHI::Shutdown()
	{

	}

	RHIExtensionList RHI::GetInstanceExtensionList()
	{
		RHIExtensionList ExtensionList = {};

		u32 PropertyCount = 0;
		vkEnumerateInstanceLayerProperties(&PropertyCount, nullptr);

		std::vector<VkLayerProperties> LayerProperties(PropertyCount);
		vkEnumerateInstanceLayerProperties(&PropertyCount, LayerProperties.data());

		std::vector<const char*> layers;
		if (IN_DEBUG_BUILD)
		{
			LOG_INFO("Running in debug mode, enabling RHI validation");
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		u32 GLFWExtensionCount = 0;
		const char** GLFWExtensions = glfwGetRequiredInstanceExtensions(&GLFWExtensionCount);

		for (u32 i = 0; i < GLFWExtensionCount; i++) {
			ExtensionList.Extensions.push_back(GLFWExtensions[i]);
		}

		if (!IN_RELEASE_BUILD) {
			ExtensionList.Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		ExtensionList.Extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		LOG_INFO("Vulkan instance extensions:");

		for (auto& ExtensionName : ExtensionList.Extensions) {
			LOG_INFO("  {}", ExtensionName);
		}

		return ExtensionList;
	}

	RHIExtensionList RHI::GetRenderDeviceExtensionList()
	{
		RHIExtensionList ExtensionList = {};
		ExtensionList.Extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		ExtensionList.Extensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
		ExtensionList.Extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		ExtensionList.Extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		ExtensionList.Extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

		return ExtensionList;
	}

}