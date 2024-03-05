cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(TetGeo VERSION 0.1.0)

add_definitions(-DASSETS_DIR=\"${CMAKE_SOURCE_DIR}/asset/\")


set(CMAKE_BUILD_FILES_DIRECTORY  "${CMAKE_HOME_DIRECTORY}/build")
set(CMAKE_BUILD_DIRECTORY  "${CMAKE_HOME_DIRECTORY}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES /bigobj")
SET(CMAKE_BUILD_TYPE "Release")

include_directories(src)

include_directories(third)

set(VTK_SMP_ENABLE_STDTHREAD OFF)
add_subdirectory(third/VTK)

include_directories(third/CLI11)

include_directories(third/nlohmann)

file(GLOB_RECURSE  SOURCE_FILES 
	${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(MAIN ${SOURCE_FILES}  )
target_link_libraries(MAIN
	PUBLIC
        VTK::IOXML
)
