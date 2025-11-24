cmake_minimum_required(VERSION 3.20)
project(CzuchEngine)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- VCPKG SETUP ---
# Zakładamy, że folder vcpkg jest wewnątrz projektu (git submodule).
# Jeśli nie, CMake spróbuje znaleźć go w zmiennych środowiskowych.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    else()
        message(WARNING "Nie znaleziono pliku toolchain vcpkg. Upewnij się, że vcpkg jest zainstalowany.")
    endif()
endif()

if(MSVC)
    # Enable iterator debugging for all targets
    add_compile_options(/D_ITERATOR_DEBUG_LEVEL=2)
endif()

if(MSVC)
    # Windows (Visual Studio) specific flags
    add_compile_options(/W4) # Warning level 4
    add_compile_options(/D_ITERATOR_DEBUG_LEVEL=2)
    
    # Runtime Libraries
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
else()
    # macOS / Linux (Clang/GCC) specific flags
    add_compile_options(-Wall -Wextra) # Standardowe ostrzeżenia
    # Na Macu nie ustawiamy ręcznie flag /MD itp.
endif()



# Find all required packages from vcpkg
find_package(glad CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# --- VULKAN SETUP ---
# Zamiast ręcznych ścieżek, używamy standardowego find_package.
find_package(Vulkan REQUIRED)

# Udostępniamy nagłówki Vulkana globalnie (jeśli Twoje pod-projekty tego wymagają)
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    # Nie linkujemy tutaj bibliotek globalnie, zrobimy to w target_link_libraries w pod-projektach
endif()

# Include subdirectories
add_subdirectory(Czuch)
add_subdirectory(Playground)
add_subdirectory(CzuchEditor)
