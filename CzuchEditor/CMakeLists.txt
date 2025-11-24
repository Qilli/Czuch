project(CzuchEditor)

# --- OPCJE KOMPILATORA ---
# Flaga /utf-8 tylko dla Windowsa (MSVC)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# --- TWORZENIE CELU (TARGET) ---
# To musi być NA GÓRZE, zanim użyjemy target_...
add_executable(CzuchEditor
    source/App/CzuchEditorApplication.cpp
)

# --- ŹRÓDŁA ---
file(GLOB_RECURSE SOURCES 
CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

# Dodajemy znalezione pliki do targetu
target_sources(CzuchEditor PRIVATE ${SOURCES})

# --- INCLUDES ---
target_include_directories(CzuchEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/Czuch/source
    ${CMAKE_SOURCE_DIR}/Czuch/vendors/imgui
    ${CMAKE_SOURCE_DIR}/Czuch/vendors/ImGuizmo
    ${VulkanMemoryAllocator_INCLUDE_DIRS} 
)

# --- GRUPOWANIE W IDE (OPCJONALNE) ---
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "Source" FILES ${SOURCES})

file(COPY ${CMAKE_SOURCE_DIR}/Assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
file(COPY ${CMAKE_SOURCE_DIR}/CzuchEditor/imgui.ini DESTINATION ${CMAKE_BINARY_DIR}/bin)

# --- LINKOWANIE BIBLIOTEK ---
target_link_libraries(CzuchEditor PRIVATE 
    Czuch 
    yaml-cpp::yaml-cpp 
    fmt::fmt 
    spdlog::spdlog 
    glm::glm 
    imgui 
    ImGuizmo
    Vulkan::Vulkan
)

# --- !!! KLUCZOWA POPRAWKA !!! ---
# Definiujemy platformę. Bez tego build się wywali.
target_compile_definitions(CzuchEditor PRIVATE
    GLFW_INCLUDE_NONE
)

if(WIN32)
    target_compile_definitions(CzuchEditor PRIVATE 
        CZUCH_PLATFORM_WINDOWS
    )
elseif(APPLE)
    target_compile_definitions(CzuchEditor PRIVATE 
        CZUCH_PLATFORM_MACOS
    )
endif()

# --- FOLDER WYJŚCIOWY ---
set_target_properties(CzuchEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)