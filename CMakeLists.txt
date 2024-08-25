cmake_minimum_required(VERSION 3.7...3.28)

project(PictureProjections VERSION 0.1.0 LANGUAGES CXX)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp")
add_executable(PictureProjections ${sources})

target_link_libraries(PictureProjections raylib)

target_compile_definitions(PictureProjections PUBLIC _CRT_SECURE_NO_WARNINGS)

target_include_directories(PictureProjections PRIVATE raylib/include)

add_subdirectory(raylib)
add_dependencies(PictureProjections raylib)