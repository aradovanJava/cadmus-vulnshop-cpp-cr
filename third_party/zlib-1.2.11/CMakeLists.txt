add_library(vendored_zlib STATIC zlib_shim.c)
target_include_directories(vendored_zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
