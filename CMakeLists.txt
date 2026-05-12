cmake_minimum_required(VERSION 3.10)
project(chttp VERSION 1.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.c")

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    ${SRC_DIR}
    ${SRC_DIR}/common
    ${SRC_DIR}/common/parser
    ${SRC_DIR}/common/Logs
)

find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Threads::Threads)

if (WIN32)
    if (MSVC)
        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
            /utf-8
            /W4
            /Zi
        )
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall
            -Wextra
            -Winvalid-pch
        )
    endif()
else()
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC m)

        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
            -finput-charset=UTF-8
            -fexec-charset=UTF-8
            -Wall
            -Wextra
            -Wpedantic
            -finput-charset=UTF-8
            -fexec-charset=UTF-8
            -rdynamic
            -g
            -save-temps=obj
        )

        add_custom_target(clean-asm
            COMMAND find "${CMAKE_BINARY_DIR}/CMakeFiles/chttp.dir/src" -name "*.c.s" 
                -exec sed -i -E "/^[[:space:]]*\.(ident|cfi|file)/d" {} +
            COMMENT "Stripping metadata from build assembly files..."
            VERBATIM
        )

        add_custom_target(generate-asm
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/export_asm.sh 
                    "${CMAKE_CURRENT_SOURCE_DIR}/assembly" 
                    "${CMAKE_BINARY_DIR}/CMakeFiles/chttp.dir/src"
            COMMENT "Exporting clean assembly to /assembly folder..."
            VERBATIM
        )
    endif()
endif()
