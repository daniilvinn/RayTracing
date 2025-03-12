cmake_minimum_required(VERSION 3.27)

include("${CMAKE_SOURCE_DIR}/Utils/SetupSourceTree.cmake")

project(RTApp)

set(RT_TARGET RayTracing)

file(GLOB_RECURSE RT_SOURCE_FILES
	"Source/.h"
	"Source/*.cpp"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RT_SOURCE_FILES})

add_executable(${RT_TARGET} ${RT_SOURCE_FILES})

SetupTargetSourceTree(${RT_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/Source")