project(Czuch)

# Znajdź pliki źródłowe (bez zmian)
file(GLOB_RECURSE SOURCES 
CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

add_library(Czuch SHARED 
    source/EngineRoot.cpp
)

# --- IMGUI SETUP ---
add_library(imgui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui/backends/imgui_impl_vulkan.cpp 
)

# --- IMGUIZMO SETUP ---
add_library(ImGuizmo STATIC  ${CMAKE_CURRENT_SOURCE_DIR}/vendors/ImGuizmo/ImGuizmo.cpp)
 
target_include_directories(ImGuizmo PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "Source" FILES ${SOURCES})

target_sources(
    Czuch PRIVATE ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/ImGuizmo
)

# --- INCLUDE DIRECTORIES ---
target_include_directories(Czuch PRIVATE
    source
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/ImGuizmo
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/Json
    ${VulkanMemoryAllocator_INCLUDE_DIRS}
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendors/imgui)
target_include_directories(ImGuizmo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendors/ImGuizmo)

target_precompile_headers(Czuch PRIVATE source/czpch.h)

# --- LINKING LIBRARIES ---

#ImGui potrzebuje nagłówków Vulkana, żeby skompilować imgui_impl_vulkan.cpp
target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)

# Link external dependencies from vcpkg
target_link_libraries(Czuch PRIVATE
    glfw
    glad::glad
    yaml-cpp::yaml-cpp
    assimp::assimp
    glm::glm-header-only
    spdlog::spdlog
    EnTT::EnTT
    glslang::glslang glslang::glslang-default-resource-limits glslang::SPIRV glslang::SPVRemapper
    #Zamiast sztywnej ścieżki .lib używamy targetu CMake
    Vulkan::Vulkan
)

target_link_libraries(Czuch PRIVATE imgui)
target_link_libraries(Czuch PRIVATE ImGuizmo)

# --- DEFINICJE PREPROCESORA (Kluczowa zmiana) ---
target_compile_definitions(Czuch PRIVATE
    GLFW_INCLUDE_NONE
)

if(WIN32)
    # Ustawienia tylko dla Windowsa
    target_compile_definitions(Czuch PRIVATE 
        CZUCH_PLATFORM_WINDOWS
        CZUCH_BUILD_DLL
    )
elseif(APPLE)
    # Ustawienia tylko dla macOS
    target_compile_definitions(Czuch PRIVATE 
        CZUCH_PLATFORM_MACOS
        CZUCH_BUILD_DLL # Zostawiamy, jeśli kod używa tego do exportu symboli
    )
endif()

# Set build configurations
set_target_properties(Czuch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)