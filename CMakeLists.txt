cmake_minimum_required(VERSION 3.27)

include("${CMAKE_SOURCE_DIR}/Utils/SetupSourceTree.cmake")

project(RTApp)

add_subdirectory("ThirdParty/glfw")
add_subdirectory("ThirdParty/spdlog")
add_subdirectory("ThirdParty/volk")

set(RT_TARGET RayTracing)
set(Vulkan_SDK $ENV{VULKAN_SDK})

file(GLOB_RECURSE RT_SOURCE_FILES
	"Source/*.h"
	"Source/*.cpp"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RT_SOURCE_FILES})

add_executable(${RT_TARGET} ${RT_SOURCE_FILES})

set_property(TARGET ${RT_TARGET} PROPERTY CXX_STANDARD 23)

target_include_directories(${RT_TARGET} PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/"
	"${CMAKE_SOURCE_DIR}/ThirdParty/glfw/include"
	"${CMAKE_SOURCE_DIR}/ThirdParty/spdlog/include"
	"${CMAKE_SOURCE_DIR}/ThirdParty/volk"
	"${Vulkan_SDK}\\Include" 
	"${Vulkan_SDK}\\Source"
)

target_compile_definitions(${RT_TARGET} PUBLIC 
	"$<$<CONFIG:Debug>:RT_DEBUG>"
	"$<$<CONFIG:MinSizeRel>:RT_MINSIZEREL>"
    "$<$<CONFIG:RelWithDebInfo>:RT_RELWITHDEBINFO>"
    "$<$<CONFIG:Release>:RT_RELEASE>"
	GLM_ENABLE_EXPERIMENTAL
	VK_NO_PROTOTYPES
)

target_link_directories(${RT_TARGET} PUBLIC
	"${Vulkan_SDK}\\Lib" 
	"${Vulkan_SDK}\\Bin"
)

target_link_libraries(${RT_TARGET} PUBLIC
	glfw
	spdlog
	volk
)

SetupTargetSourceTree(${RT_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/Source")

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${RT_TARGET})
	set_target_properties(${RT_TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime")
	target_compile_options(${RT_TARGET} PRIVATE /MP /W1)
else()
	message("Please, select a 'RayTracing' target as startup project in your IDE and set a working directory to the 'Runtime' folder")
endif()
