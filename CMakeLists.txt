cmake_minimum_required(VERSION 3.23)
project(360_image_viewer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(argos
    GIT_REPOSITORY "https://github.com/jebreimo/Argos.git"
    GIT_TAG v1.2.1)
FetchContent_Declare(tungsten
    GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
    GIT_TAG master)
FetchContent_Declare(yimage
    GIT_REPOSITORY "https://github.com/jebreimo/Yimage.git"
    GIT_TAG v0.1.7)
FetchContent_Declare(xyz
    GIT_REPOSITORY "https://github.com/jebreimo/Xyz.git"
    GIT_TAG v0.2.91)
FetchContent_MakeAvailable(argos tungsten yimage xyz)

list(APPEND CMAKE_MODULE_PATH ${tungsten_SOURCE_DIR}/tools/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/cppembed)

include(TungstenTargetEmbedShaders)
include(TargetEmbedCppData)

add_executable(360_image_viewer
    src/360_image_viewer/main.cpp
    src/360_image_viewer/ObjFileWriter.cpp
    src/360_image_viewer/ObjFileWriter.hpp
    src/360_image_viewer/Render3DShaderProgram.cpp
    src/360_image_viewer/Render3DShaderProgram.hpp
    src/360_image_viewer/SpherePosCalculator.cpp
    src/360_image_viewer/SpherePosCalculator.hpp
    src/360_image_viewer/Unicolor3DShaderProgram.cpp
    src/360_image_viewer/Unicolor3DShaderProgram.hpp
    src/360_image_viewer/Cross.cpp
    src/360_image_viewer/Cross.hpp
    src/360_image_viewer/Sphere.cpp
    src/360_image_viewer/Sphere.hpp
    src/360_image_viewer/RingBuffer.hpp
    src/360_image_viewer/HelveticaNeue32.hpp
    src/360_image_viewer/Hud.cpp
    src/360_image_viewer/Hud.hpp)

target_link_libraries(360_image_viewer
    PRIVATE
        Argos::Argos
        Tungsten::Tungsten
        Yimage::Yimage
    )

tungsten_target_embed_shaders(360_image_viewer
    FILES
    src/360_image_viewer/shaders/Render3D-frag.glsl
    src/360_image_viewer/shaders/Render3D-vert.glsl
    src/360_image_viewer/shaders/Unicolor3D-frag.glsl
    src/360_image_viewer/shaders/Unicolor3D-vert.glsl
    )

#target_embed_cpp_data(360_image_viewer
#    INCLUDE_DIRS
#        data
#    FILES
#        src/360_image_viewer/DefaultImage.hpp.in
#    )

if (EMSCRIPTEN)
    target_link_options(360_image_viewer
        PRIVATE
            -sALLOW_MEMORY_GROWTH=1
            -sEXPORTED_FUNCTIONS=['_main','_load_image']
            -sEXPORTED_RUNTIME_METHODS=['ccall']
            -sFORCE_FILESYSTEM=1
        )
    set(EMSCRIPTEN_TARGET_NAME 360_image_viewer)
    configure_file(src/emscripten/index.html.in index.html)
    configure_file(src/emscripten/index.css index.css)
endif ()
