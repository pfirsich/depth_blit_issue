cmake_minimum_required(VERSION 3.7)

project(depth_blit_issue)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include_directories(dependencies/glm)

add_subdirectory(dependencies/glad)
include_directories(dependencies/glad/include)

add_subdirectory(dependencies/glfw)
include_directories(dependencies/glfw/include)

add_compile_definitions(NOMINMAX)
add_executable(depth_blit_issue main.cpp)
target_link_libraries(depth_blit_issue glfw glad)
