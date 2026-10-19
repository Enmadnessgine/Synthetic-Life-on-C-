cmake_minimum_required(VERSION 3.14)
project(ArtificialLife LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/SFML/SFML-3.0.0")

find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

add_executable(ArtificialLife src/main.cpp src/entities/Entity.cpp src/spatial/SpatialGrid.cpp)
target_link_libraries(ArtificialLife SFML::Graphics SFML::Window SFML::System)