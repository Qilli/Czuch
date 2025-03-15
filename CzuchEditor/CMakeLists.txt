project(CzuchEditor)
add_compile_options(/utf-8)
add_executable(CzuchEditor
    source/App/CzuchEditorApplication.cpp
)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

target_include_directories(CzuchEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/Czuch/source
    ${CMAKE_SOURCE_DIR}/Czuch/vendors/imgui
    ${CMAKE_SOURCE_DIR}/Czuch/vendors/ImGuizmo
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "Source" FILES ${SOURCES})

target_sources(
CzuchEditor PRIVATE ${SOURCES}
)

target_link_libraries(CzuchEditor PRIVATE Czuch yaml-cpp::yaml-cpp fmt::fmt spdlog::spdlog glm::glm imgui ImGuizmo)

# Define preprocessor macros
target_compile_definitions(CzuchEditor PRIVATE
    CZUCH_PLATFORM_WINDOWS
)

set_target_properties(CzuchEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)