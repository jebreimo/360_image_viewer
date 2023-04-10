##****************************************************************************
## Copyright © 2019 Jan Erik Breimo. All rights reserved.
## Created by Jan Erik Breimo on 2019-12-24.
##
## This file is distributed under the BSD License.
## License text is included with the source distribution.
##****************************************************************************
cmake_minimum_required(VERSION 3.13)

set_property(GLOBAL
    PROPERTY
        cppembed_cmake_module_dir "${CMAKE_CURRENT_LIST_DIR}")

function(target_embed_cpp_data target_name)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    cmake_parse_arguments(ARG "" "FILE_EXTENSION" "INCLUDE_DIRS;FILES" ${ARGN})

    foreach (INCLUDE_PATH IN LISTS ARG_INCLUDE_DIRS)
        file(REAL_PATH "${INCLUDE_PATH}" REAL_INCLUDE_PATH)
        list(APPEND INCLUDE_OPTIONS "-i")
        list(APPEND INCLUDE_OPTIONS "${REAL_INCLUDE_PATH}")
    endforeach ()

    if (DEFINED ARG_FILE_EXTENSION)
        set(FILE_EXTENSION "${FILE_EXTENSION}")
    else ()
        set(FILE_EXTENSION ".h")
    endif ()

    foreach (INPUT_PATH IN LISTS ARG_FILES)
        file(REAL_PATH "${INPUT_PATH}" REAL_INPUT_PATH)
        get_filename_component(INPUT_EXTENSION ${INPUT_PATH} LAST_EXT)
        get_filename_component(INPUT_NAME ${INPUT_PATH} NAME_WLE)
        if (NOT INPUT_EXTENSION STREQUAL ".in")
            set(INPUT_NAME "${INPUT_NAME}${FILE_EXTENSION}")
        endif ()
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/cppembed_include/${INPUT_NAME}")
        get_property(SCRIPT_DIR GLOBAL PROPERTY cppembed_cmake_module_dir)
        set(SCRIPT_PATH ${SCRIPT_DIR}/cppembed.py)
        add_custom_command(OUTPUT "${OUTPUT_PATH}"
            COMMAND "${Python3_EXECUTABLE}" ${SCRIPT_PATH} ${INCLUDE_OPTIONS} "${REAL_INPUT_PATH}" -o "${OUTPUT_PATH}"
            DEPENDS "${REAL_INPUT_PATH}")
        list(APPEND OUTPUT_FILES "${OUTPUT_PATH}")
    endforeach ()

    add_custom_target(${target_name}_EmbeddedHeaders ALL
        DEPENDS ${OUTPUT_FILES})

    add_dependencies(${target_name} ${target_name}_EmbeddedHeaders)

    target_include_directories(${target_name} BEFORE
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cppembed_include>
        )
endfunction()
