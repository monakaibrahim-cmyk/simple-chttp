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
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC Threads::Threads m)

if (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /utf-8)
elseif (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
    )
endif()
