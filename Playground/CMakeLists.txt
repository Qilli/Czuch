project(Playground)

# ZMIANA: /utf-8 działa tylko na Windows (MSVC). Na Macu (Clang) UTF-8 jest domyślne.
if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(Playground
    source/Application.cpp
)

target_include_directories(Playground PRIVATE
    ${CMAKE_SOURCE_DIR}/Czuch/source
    # Opcjonalnie: Jeśli w Playground używasz ImGui bezpośrednio w kodzie, 
    # odkomentuj poniższą linię:
    # ${CMAKE_SOURCE_DIR}/Czuch/vendors/imgui
)

target_link_libraries(Playground PRIVATE Czuch yaml-cpp::yaml-cpp spdlog::spdlog)

# ZMIANA: Automatyczne wykrywanie platformy
if(WIN32)
    target_compile_definitions(Playground PRIVATE 
        CZUCH_PLATFORM_WINDOWS
    )
elseif(APPLE)
    target_compile_definitions(Playground PRIVATE 
        CZUCH_PLATFORM_MACOS
    )
endif()

set_target_properties(Playground PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Kopiowanie assetów (to zadziała na obu systemach, o ile ścieżki folderów są poprawne)
#file(COPY ${CMAKE_SOURCE_DIR}/Assets DESTINATION ${CMAKE_BINARY_DIR}/bin)