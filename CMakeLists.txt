cmake_minimum_required(VERSION 3.20)
project(CzuchEngine)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

if(MSVC)
    # Enable iterator debugging for all targets
    add_compile_options(/D_ITERATOR_DEBUG_LEVEL=2)
endif()



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ITERATOR_DEBUG_LEVEL=2")
if(MSVC)
    # Set Debug Runtime Library to /MDd (Dynamic Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

    # Set Release Runtime Library to /MD (Dynamic Release)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

# Find all required packages from vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

include_directories(${VCPKG_INSTALLED_DIR}/x64-windows/include)

# Vulkan SDK (not available via vcpkg, using environment variable)
set(VULKAN_SDK $ENV{VULKAN_SDK})
include_directories(${VULKAN_SDK}/Include)
link_directories(${VULKAN_SDK}/Lib)

# Include subdirectories
add_subdirectory(Czuch)
add_subdirectory(Playground)
add_subdirectory(CzuchEditor)
