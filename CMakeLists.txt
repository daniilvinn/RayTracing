cmake_minimum_required(VERSION 3.27)

include("${CMAKE_SOURCE_DIR}/Utils/SetupSourceTree.cmake")

project(RTApp)

add_subdirectory("ThirdParty/glfw")
add_subdirectory("ThirdParty/spdlog")

set(RT_TARGET RayTracing)

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
)

target_compile_definitions(${RT_TARGET} PUBLIC 
	"$<$<CONFIG:Debug>:RT_DEBUG>"
	"$<$<CONFIG:MinSizeRel>:RT_MINSIZEREL>"
    "$<$<CONFIG:RelWithDebInfo>:RT_RELWITHDEBINFO>"
    "$<$<CONFIG:Release>:RT_RELEASE>"
)

target_link_libraries(${RT_TARGET} PUBLIC
	glfw
	spdlog
)

SetupTargetSourceTree(${RT_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/Source")

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${RT_TARGET})
	set_target_properties(${RT_TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Runtime"
)
else()
	message("Please, select a 'RayTracing' target as startup project in your IDE and set a working directory to the 'Runtime' folder")
endif()
