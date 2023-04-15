//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-04-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>
#include "Unicolor3DShaderProgram.hpp"

class Cross
{
public:
    Cross();

    void draw();
private:
    Tungsten::BufferHandle buffer_;
    Tungsten::VertexArrayHandle vertex_array_;
    GLsizei count_;
    Unicolor3DShaderProgram program_;
};
