# ---------------------------------------------------------------------------
# EXCERPT B on slide 25 of node N1 comes from this file and config/vulnshop.ini.
#
# Everything below builds and runs. Every hardening decision in it is wrong.
# CR2 and CR4 ask you to fix it; do not fix it before then.
# ---------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
project(vulnshop CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# perf: sanitizers made the nightly job too slow
set(CMAKE_CXX_FLAGS "-O2 -g -w -fno-stack-protector")
set(CMAKE_EXE_LINKER_FLAGS "-no-pie -Wl,-z,norelro -Wl,-z,execstack")

# --- dependencies ----------------------------------------------------------
# Three libraries are vendored in third_party/ as directories of source, with
# local edits and no manifest of any kind. See third_party/README.md.
add_subdirectory(third_party/zlib-1.2.11)
add_subdirectory(third_party/libpng-1.6.35)
add_subdirectory(third_party/json-3.7.3)

# A fourth is fetched from a moving tag. CI builds with -DVULNSHOP_FETCH=ON,
# so CI and every developer machine can be building different source.
option(VULNSHOP_FETCH "Fetch the HTTP helper from upstream" OFF)
if(VULNSHOP_FETCH)
  include(FetchContent)
  FetchContent_Declare(httphelper
    GIT_REPOSITORY https://github.com/example/http-helper.git
    GIT_TAG        master)          # not a tag, not a digest, not pinned
  FetchContent_MakeAvailable(httphelper)
endif()

file(GLOB_RECURSE VULNSHOP_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(vulnshop ${VULNSHOP_SOURCES})
target_include_directories(vulnshop PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(vulnshop PRIVATE vendored_zlib vendored_png vendored_json)

# No SBOM target. No checksum or signature verification. No SCA step.
# No sanitizer build type. No warnings, because of -w above.
