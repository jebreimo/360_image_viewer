//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 100

attribute vec3 a_position;
attribute vec2 a_texture_coord;

uniform mat4 u_mv_matrix;
uniform mat4 u_p_matrix;

varying highp vec2 v_texture_coord;

void main()
{
    vec4 p = u_mv_matrix * vec4(a_position, 1.0);
    gl_Position = u_p_matrix * p;
    v_texture_coord = a_texture_coord;
}
