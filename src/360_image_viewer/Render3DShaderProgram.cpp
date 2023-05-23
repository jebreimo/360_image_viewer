//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Render3DShaderProgram.hpp"

#include <Tungsten/ShaderProgramBuilder.hpp>
#include "Render3D-frag.glsl.hpp"
#include "Render3D-vert.glsl.hpp"

void Render3DShaderProgram::setup()
{
    using namespace Tungsten;
    program = ShaderProgramBuilder()
        .add_shader(ShaderType::VERTEX, Render3D_vert)
        .add_shader(ShaderType::FRAGMENT, Render3D_frag)
        .build();

    position = get_vertex_attribute(program, "a_position");
    texture_coord = get_vertex_attribute(program, "a_texture_coord");

    mv_matrix = get_uniform<Xyz::Matrix4F>(program, "u_mv_matrix");
    p_matrix = get_uniform<Xyz::Matrix4F>(program, "u_p_matrix");

    texture = get_uniform<GLint>(program, "u_texture");
}
