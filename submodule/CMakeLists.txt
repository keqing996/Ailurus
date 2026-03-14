set (BUILD_SHARED_LIBS OFF)

# spdlog
set (SPDLOG_BUILD_SHARED OFF)
add_subdirectory(spdlog)

# glm
add_subdirectory(glm)
target_compile_definitions(glm PUBLIC GLM_FORCE_LEFT_HANDED GLM_FORCE_QUAT_DATA_XYZW)

# nlohmann json
add_subdirectory(nlohmann)

# sdl
set (SDL_STATIC ON) # static link sdl
set (SDL_SHARED OFF)
add_subdirectory(sdl)

# assimp
set (ASSIMP_NO_EXPORT ON) # strip out export functionality to reduce binary size
set (ASSIMP_BUILD_TESTS OFF) # don't need the test programs
set (ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF) # only build the importers we need
set (ASSIMP_BUILD_OBJ_IMPORTER ON)
set (ASSIMP_BUILD_FBX_IMPORTER ON)
add_subdirectory(assimp)
