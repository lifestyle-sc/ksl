# === Exclude main and test files using regex
file(GLOB SRC_FILES CONFIGURE_DEPENDS "*.cpp")
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.m\\.cpp$")
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.t\\.cpp$")

# === Build Source files as libraries
add_library(ksllib STATIC ${SRC_FILES})
target_include_directories(ksllib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ksllib PUBLIC memorylib)

# Create a "warnings" target that doesn't build code itself,
# but holds all our strict settings.
add_library(project_strict_warnings INTERFACE)

target_compile_options(project_strict_warnings INTERFACE
    # Settings for GCC and Clang
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall                   # Basic common warnings
        -Wextra                  # Additional common warnings
        -Wshadow                # Warn when a variable "shadows" another (see below)
        -Wuninitialized         # Warn about using variables before they are set
        -Wpedantic              # Ensure you aren't using non-standard compiler "extensions"
        -Wconversion            # Warn if data is lost during type conversion (e.g., double to int)
        -Wreturn-type           # The one that caught your previous error!
        -Werror                 # Turn all the above into errors
    >
    # Settings for Visual Studio (MSVC)
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4                     # Set warning level 4 (highest standard level)
        /WX                     # Warnings as errors
        /permissive-            # Enforce standard C++ behavior (turns off MSVC "cheats")
    >
)

# === Main App ===
add_executable(ksl.tsk
ksl.m.cpp
)
target_link_libraries(ksl.tsk PRIVATE ksllib project_strict_warnings)

# === GoogleTest via FetchContent ===
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# === Auto-discover test files matching *.t.cpp in root ===
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.t.cpp")

include(GoogleTest)

foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(FILE_NAME ${TEST_SRC} NAME)
    string(REGEX REPLACE "\\.cpp$" "" TEST_NAME ${FILE_NAME}) # e.g., math.t.cpp -> math.t
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} PRIVATE ksllib gtest_main)
    gtest_discover_tests(${TEST_NAME})
endforeach()
