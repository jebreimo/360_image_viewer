//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Tungsten.hpp"

class Unicolor3DShaderProgram
{
public:
    void setup();

    Tungsten::ProgramHandle program;

    Tungsten::Uniform<Xyz::Matrix4F> mv_matrix;
    Tungsten::Uniform<Xyz::Matrix4F> p_matrix;
    Tungsten::Uniform<Xyz::Vector4F> color;

    GLuint position;
    GLuint texture_coord;
};
