cmake_minimum_required(VERSION 3.23)
project(3d_image_viewer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG v1.1.269)
FetchContent_Declare(tungsten
    GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
    GIT_TAG v0.2.77)
FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG main)
FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
    GIT_TAG v0.2.86)
FetchContent_MakeAvailable(argos tungsten yimage xyz)

list(APPEND CMAKE_MODULE_PATH ${tungsten_SOURCE_DIR}/tools/cmake)

include(TungstenTargetEmbedShaders)

add_executable(3d_image_viewer
    src/3d_image_viewer/main.cpp
    src/3d_image_viewer/ObjFileWriter.cpp
    src/3d_image_viewer/ObjFileWriter.hpp
    src/3d_image_viewer/Render3DShaderProgram.cpp
    src/3d_image_viewer/Render3DShaderProgram.hpp
    src/3d_image_viewer/SpherePosCalculator.cpp
    src/3d_image_viewer/SpherePosCalculator.hpp
    src/3d_image_viewer/Unicolor3DShaderProgram.cpp
    src/3d_image_viewer/Unicolor3DShaderProgram.hpp
    src/3d_image_viewer/YimageGl.cpp
    src/3d_image_viewer/YimageGl.hpp
    )

target_link_libraries(3d_image_viewer
    PRIVATE
        Argos::Argos
        Tungsten::Tungsten
        Yimage::Yimage
    )

tungsten_target_embed_shaders(3d_image_viewer
    FILES
        src/3d_image_viewer/Render3D-frag.glsl
        src/3d_image_viewer/Render3D-vert.glsl
        src/3d_image_viewer/Unicolor3D-frag.glsl
        src/3d_image_viewer/Unicolor3D-vert.glsl
    )

if (EMSCRIPTEN)
    set(EMSCRIPTEN_TARGET_NAME 3d_image_viewer)
    configure_file(src/emscripten/index.html.in index.html)
endif ()
