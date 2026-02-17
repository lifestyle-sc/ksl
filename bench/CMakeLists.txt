# === Options
option(ENABLE_PROFILING "Link with gperftools profiler" OFF)
set(SANITIZERS "" CACHE STRING "Comma-separated list: address,undefined,leak,thread")

# This makes release builds debuggable (but still optimized)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")

# Set flags for Debug (low optimization, high debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wpedantic")

# Set flags for Release (high optimization)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -Wpedantic")

# === Helper: parse SANITIZERS into flags
function(apply_sanitizers tgt)
  if(SANITIZERS)
    string(REPLACE "," ";" _list "${SANITIZERS}")
    set(_flags "")
    foreach(s IN LISTS _list)
      if(s STREQUAL "address")
        list(APPEND _flags address)
      elseif(s STREQUAL "undefined")
        list(APPEND _flags undefined)
      elseif(s STREQUAL "leak")
        list(APPEND _flags leak)
      elseif(s STREQUAL "thread")
        list(APPEND _flags thread)
      else()
        message(FATAL_ERROR "Unknown santizer: ${s}")
      endif()
    endforeach()
    if(_flags)
      # add compile+link sanitizer flags
      target_compile_options(${tgt} PRIVATE -fsanitize=${_flags})
      target_link_options(${tgt} PRIVATE -fsanitize=${_flags})
      # helpful for nicer stacks
      target_compile_options(${tgt} PRIVATE -fno-omit-frame-pointer)
    endif()

  endif()
endfunction()

# === Helper: profiler link
function(apply_profiler tgt)
  if(ENABLE_PROFILING)
    # Prefer find_library; fall back to raw flag if not found
    find_library(PROFILER_LIB profiler)
    if(PROFILER_LIB)
      target_link_libraries(${tgt} PRIVATE ${PROFILER_LIB})
    else()
      target_link_options(${tgt} PRIVATE -lprofiler)
    endif()
  endif()
endfunction()

# === Exclude main and test files using regex
file(GLOB SRC_FILES CONFIGURE_DEPENDS "*.cpp")
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.m\\.cpp$")
list(FILTER SRC_FILES EXCLUDE REGEX ".*\\.t\\.cpp$")

find_package(benchmark REQUIRED)

# === Build Source files as libraries
add_library(benchlib STATIC ${SRC_FILES})
target_include_directories(benchlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(benchlib PUBLIC memorylib)

# Create a "warnings" target that doesn't build code itself,
# but holds all our strict settings.
add_library(bench_strict_warnings INTERFACE)

target_compile_options(bench_strict_warnings INTERFACE
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
add_executable(bench.tsk
bench.m.cpp
)
target_link_libraries(bench.tsk PRIVATE benchlib bench_strict_warnings benchmark)

# ===  Apply knobs in one place
apply_sanitizers(benchlib)
apply_sanitizers(bench.tsk)
apply_profiler(bench.tsk)

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
    target_link_libraries(${TEST_NAME} PRIVATE benchlib gtest_main)
    gtest_discover_tests(${TEST_NAME})
endforeach()