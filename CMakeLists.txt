cmake_minimum_required(VERSION 3.16) # FetchContent is available in 3.11+
project(cad-renderer)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GRAPHICS GRAPHICS_API_OPENGL_SOFTWARE)
#SET(CMAKE_CXX_FLAGS  "-g")
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# para poder descargar raylib en windows (falla por falta de certificados)
SET(CMAKE_TLS_VERIFY 0)
set(CMAKE_C_STANDARD 99)

include(FetchContent)

# Dependencies
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
endif()

file(GLOB SOURCES
  *.h
  *.c
)

ADD_EXECUTABLE(${PROJECT_NAME}
${SOURCES}
)

target_link_libraries(${PROJECT_NAME} raylib -static)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/src
          ${CMAKE_CURRENT_BINARY_DIR}/src
)