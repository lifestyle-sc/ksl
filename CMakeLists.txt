cmake_minimum_required(VERSION 3.14)
project(ksl CXX C)

set(CMAKE_CXX_STANDARD 23)
enable_testing()

add_subdirectory(src)
add_subdirectory(bench)
add_subdirectory(groups/memory)


add_custom_target(
    check-format ALL
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/format.sh --verify-only ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/bench ${CMAKE_CURRENT_SOURCE_DIR}/groups
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Verifying source code formatting"
)
