//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 100

uniform highp vec4 u_color;

void main()
{
    gl_FragColor = u_color;
}
