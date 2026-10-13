cmake_minimum_required(VERSION 3.20)
project(MoonLake VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ---------------------------------------------------------------------------
# GLFW 3.4
# ---------------------------------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

# ---------------------------------------------------------------------------
# GLM 1.0.1 (Header-only)
# ---------------------------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw glm)

# ---------------------------------------------------------------------------
# GLAD (OpenGL 3.3 Core Profile Loader)
# ---------------------------------------------------------------------------
add_library(glad STATIC
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

# ---------------------------------------------------------------------------
# stb_image (Header-only image loading)
# ---------------------------------------------------------------------------
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

# ---------------------------------------------------------------------------
# Collect All Source Files (Modular Object Structure)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# ---------------------------------------------------------------------------
# Main Executable
# ---------------------------------------------------------------------------
add_executable(MoonLake
    ${PROJECT_SOURCES}
)

target_include_directories(MoonLake PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

target_link_libraries(MoonLake PRIVATE
    glad
    glfw
    glm::glm
    stb
    opengl32
)

# Automatically copy shader files to the executable directory after building
add_custom_command(TARGET MoonLake POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:MoonLake>/shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:MoonLake>/textures
    COMMENT "Copying shaders and ensuring textures in output directory..."
)
