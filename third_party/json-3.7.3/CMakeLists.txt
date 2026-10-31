add_library(vendored_json STATIC json_shim.c)
target_include_directories(vendored_json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
