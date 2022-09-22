//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Render3DShaderProgram.hpp"
#include "Render3D-frag.glsl.hpp"
#include "Render3D-vert.glsl.hpp"

void Render3DShaderProgram::setup()
{
    using namespace Tungsten;
    program = create_program();
    auto vertexShader = create_shader(GL_VERTEX_SHADER, Render3D_vert);
    attach_shader(program, vertexShader);
    auto fragmentShader = create_shader(GL_FRAGMENT_SHADER, Render3D_frag);
    attach_shader(program, fragmentShader);
    link_program(program);
    use_program(program);

    position = get_vertex_attribute(program, "a_position");
    textureCoord = get_vertex_attribute(program, "a_texture_coord");

    transformation = get_uniform<Xyz::Matrix3F>(program, "u_transformation");
    texture = get_uniform<GLint>(program, "u_texture");
    z = get_uniform<GLfloat>(program, "u_z");
}
