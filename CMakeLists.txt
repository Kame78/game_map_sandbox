cmake_minimum_required(VERSION 3.24)
project(HighPerformanceRenderer CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3.0.0 REQUIRED COMPONENTS Graphics Window System)

add_executable(WorldRenderer
src/main.cpp
src/map.cpp
src/player.cpp
src/world_editor.cpp
src/chunk.cpp
)

target_include_directories(WorldRenderer PRIVATE
     ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(WorldRenderer PRIVATE
SFML::Graphics
SFML::Window
SFML::System
)

add_custom_command(TARGET WorldRenderer POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
"${CMAKE_SOURCE_DIR}/assets"
"${CMAKE_BINARY_DIR}/assets"
COMMENT "Deploying game asset pipelines to build runtime destination..."
)