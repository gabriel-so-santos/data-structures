cmake_minimum_required(VERSION 4.1)
project(libds VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.c)

add_library(ds STATIC ${SRC_FILES})

target_include_directories(ds
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)

enable_testing()
add_subdirectory(tests)