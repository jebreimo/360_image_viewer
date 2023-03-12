//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Unicolor3DShaderProgram.hpp"
#include "Unicolor3D-frag.glsl.hpp"
#include "Unicolor3D-vert.glsl.hpp"

void Unicolor3DShaderProgram::setup()
{
    using namespace Tungsten;
    program = create_program();
    auto vertexShader = create_shader(GL_VERTEX_SHADER, Unicolor3D_vert);
    attach_shader(program, vertexShader);
    auto fragmentShader = create_shader(GL_FRAGMENT_SHADER, Unicolor3D_frag);
    attach_shader(program, fragmentShader);
    link_program(program);
    use_program(program);

    position = get_vertex_attribute(program, "a_position");
    texture_coord = get_vertex_attribute(program, "a_texture_coord");

    mv_matrix = get_uniform<Xyz::Matrix4F>(program, "u_mv_matrix");
    p_matrix = get_uniform<Xyz::Matrix4F>(program, "u_p_matrix");
    color = get_uniform<Xyz::Vector4F>(program, "u_color");
}
