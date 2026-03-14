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

set (LWS_WITH_SSL OFF CACHE BOOL "" FORCE)
set (LWS_WITH_STATIC ON CACHE BOOL "" FORCE)
set (LWS_WITH_SHARED OFF CACHE BOOL "" FORCE)
set (LWS_WITHOUT_CLIENT ON CACHE BOOL "" FORCE)
set (LWS_WITHOUT_SERVER OFF CACHE BOOL "" FORCE)
set (LWS_WITHOUT_TESTAPPS ON CACHE BOOL "" FORCE)
set (LWS_WITHOUT_TEST_SERVER ON CACHE BOOL "" FORCE)
set (LWS_WITHOUT_TEST_SERVER_EXTPOLL ON CACHE BOOL "" FORCE)
set (LWS_WITHOUT_TEST_PING ON CACHE BOOL "" FORCE)
set (LWS_WITH_MINIMAL_EXAMPLES OFF CACHE BOOL "" FORCE)
set (LWS_WITH_HTTP2 OFF CACHE BOOL "" FORCE)
add_subdirectory(libwebsockets)
