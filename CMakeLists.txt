project(gentiming)
cmake_minimum_required(VERSION 2.8)

SET (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

aux_source_directory(. SRC_LIST)

#file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/*.cpp)

file(GLOB SRC_LIST_SC ${PROJECT_SOURCE_DIR}/struct_controls/*.cpp)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/struct_controls)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${SRC_LIST_SC})

set(CMAKE_BUILD_TYPE Debug)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)


if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

target_link_libraries(gentiming boost_thread boost_system rt)

if(WITHOUT_QT)
	message(STATUS "qt off")
	add_definitions(-DWITHOUT_QT)
else()
	message(STATUS "qt on")
endif()
