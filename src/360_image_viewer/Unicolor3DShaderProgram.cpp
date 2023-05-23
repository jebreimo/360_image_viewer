//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Unicolor3DShaderProgram.hpp"

#include <Tungsten/ShaderProgramBuilder.hpp>
#include "Unicolor3D-frag.glsl.hpp"
#include "Unicolor3D-vert.glsl.hpp"

void Unicolor3DShaderProgram::setup()
{
    using namespace Tungsten;
    program = ShaderProgramBuilder()
        .add_shader(ShaderType::VERTEX, Unicolor3D_vert)
        .add_shader(ShaderType::FRAGMENT, Unicolor3D_frag)
        .build();

    position = get_vertex_attribute(program, "a_position");

    mv_matrix = get_uniform<Xyz::Matrix4F>(program, "u_mv_matrix");
    p_matrix = get_uniform<Xyz::Matrix4F>(program, "u_p_matrix");
    color = get_uniform<Xyz::Vector4F>(program, "u_color");
}
