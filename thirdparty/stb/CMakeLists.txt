
add_library                 (stb_image STATIC ./src/impl.cpp)
target_include_directories  (stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)