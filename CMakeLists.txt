project(gentiming)
cmake_minimum_required(VERSION 2.8)

SET (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

aux_source_directory(. SRC_LIST)

#file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/*.cpp)

file(GLOB SRC_LIST_SC ${PROJECT_SOURCE_DIR}/struct_controls/*.cpp)
file(GLOB SRC_LIST_U ${PROJECT_SOURCE_DIR}/utils/*.cpp)
file(GLOB SRC_LIST_SNS ${PROJECT_SOURCE_DIR}/sensors/*.cpp)
file(GLOB SRC_LIST_GP ${PROJECT_SOURCE_DIR}/gpio/*.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${SRC_LIST_SC} ${SRC_LIST_U} ${SRC_LIST_SNS} ${SRC_LIST_GP})

target_include_directories(${PROJECT_NAME} PUBLIC
	./
	struct_controls/
	utils/
	sensors/
	gpio/
)

if(BUILD_ARM)
	set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
	set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-c++)
	include_directories(/opt/crosstools/boost/include)
#	link_directories(/opt/crosstools/boost/lib/)
#	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/crosstools/boost/lib/)
#	message(STATUS "libpath: ${CMAKE_LIBRARY_PATH}")
	set(LIBS -L"/opt/crosstools/boost/lib/")
else()
	set(CMAKE_BUILD_TYPE Debug)
endif()


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

target_link_libraries(gentiming ${LIBS} -lboost_system -lboost_thread rt -lpthread)

if(WITHOUT_QT)
	message(STATUS "qt off")
	add_definitions(-DWITHOUT_QT)
else()
	message(STATUS "qt on")
endif()
