cmake_minimum_required(VERSION 3.5)
project(peepa-server C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -ggdb")

include_directories(Include)
include_directories(${PROJECT_SOURCE_DIR}/Library/enet/include)
#include_directories(${PROJECT_SOURCE_DIR}/Library/penet/Include)
include_directories(${PROJECT_SOURCE_DIR}/../penet/Include)
include_directories(${PROJECT_SOURCE_DIR}/Library/LibNerve/Include)
include_directories(${PROJECT_SOURCE_DIR}/Library/buddy_alloc)
include_directories(${PROJECT_SOURCE_DIR}/Library/imgui)
include_directories(${PROJECT_SOURCE_DIR}/Library/imgui/backends)

add_subdirectory(${PROJECT_SOURCE_DIR}/Library/LibNerve)

file(GLOB_RECURSE SOURCES_CXX ${PROJECT_SOURCE_DIR}/Source/*.cpp)
file(GLOB SOURCES_IMGUI ${PROJECT_SOURCE_DIR}/Library/imgui/*.cpp)
file(GLOB SOURCES_ZSTD ${PROJECT_SOURCE_DIR}/Library/zstd/lib/common/*.c)
file(GLOB SOURCES_ZSTD_COMPRESS ${PROJECT_SOURCE_DIR}/Library/zstd/lib/compress/*.c)
file(GLOB SOURCES_ZSTD_DECOMPRESS ${PROJECT_SOURCE_DIR}/Library/zstd/lib/decompress/*.c)
file(GLOB SOURCES_ZSTD_DECOMPRESS_ASM ${PROJECT_SOURCE_DIR}/Library/zstd/lib/decompress/*.S)

add_executable(peepa-server
  ${SOURCES_CXX}
  ${SOURCES_IMGUI}
  ${PROJECT_SOURCE_DIR}/Library/imgui/backends/imgui_impl_opengl3.cpp
  ${PROJECT_SOURCE_DIR}/Library/imgui/backends/imgui_impl_glfw.cpp
  ${SOURCES_ZSTD}
  ${SOURCES_ZSTD_COMPRESS}
  ${SOURCES_ZSTD_DECOMPRESS}
  ${SOURCES_ZSTD_DECOMPRESS_ASM}
  )
find_package(Threads REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
target_link_libraries(peepa-server PUBLIC Nerve)
target_link_libraries(peepa-server PRIVATE Threads::Threads glfw OpenGL::GL)
if(WIN32)
  target_link_libraries(peepa-server PRIVATE wsock32 ws2_32 winmm)
endif()
