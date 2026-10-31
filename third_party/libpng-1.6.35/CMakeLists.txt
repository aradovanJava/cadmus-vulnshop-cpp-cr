add_library(vendored_png STATIC png_shim.c)
target_include_directories(vendored_png PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
