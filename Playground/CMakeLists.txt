project(Playground)

add_compile_options(/utf-8)
add_executable(Playground
    source/Application.cpp
)

target_include_directories(Playground PRIVATE
    ${CMAKE_SOURCE_DIR}/Czuch/source
)

target_link_libraries(Playground PRIVATE Czuch yaml-cpp::yaml-cpp spdlog::spdlog)

# Define preprocessor macros
target_compile_definitions(Playground PRIVATE
    CZUCH_PLATFORM_WINDOWS
)

set_target_properties(Playground PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

file(COPY ${CMAKE_SOURCE_DIR}/Assets DESTINATION ${CMAKE_BINARY_DIR}/Playground)