cmake_minimum_required(VERSION 3.28)
project(Repro LANGUAGES CXX)

# --- Dependencies ---

if(WIN32)
    set(SLANG_IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/slang/windows/bin/slang-compiler.dll")
    set(SLANG_IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/slang/windows/lib/slang-compiler.lib")
elseif(UNIX)
    set(SLANG_IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/slang/linux/lib/libslang-compiler.so.0.2025.21")
endif()

add_library(Slang SHARED IMPORTED)
set_target_properties(Slang
    PROPERTIES
        IMPORTED_LOCATION "${SLANG_IMPORTED_LOCATION}"
        IMPORTED_IMPLIB "${SLANG_IMPORTED_IMPLIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/slang/include"
)

# --- Build ---

add_executable(Repro)
set_target_properties(Repro
        PROPERTIES
        CXX_STANDARD 23
        OUTPUT_NAME "Repro"
)

file(GLOB sources CONFIGURE_DEPENDS src/*.cpp)
target_sources(Repro PRIVATE ${sources})

target_link_libraries(Repro PRIVATE Slang)

# --- Copy shared libraries ---

add_custom_command(
    TARGET Repro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SLANG_IMPORTED_LOCATION}" "$<TARGET_FILE_DIR:Repro>"
)
