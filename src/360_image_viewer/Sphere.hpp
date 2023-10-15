//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-04-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>
#include <Yimage/Yimage.hpp>
#include <Yimage/ImageAlgorithms.hpp>
#include "Render3DShaderProgram.hpp"
#include "Unicolor3DShaderProgram.hpp"

class Sphere
{
public:
    Sphere(int circles, int points);

    Sphere(const Yimage::Image& img, int circles, int points);

    void set_image(const Yimage::Image& img);

    void draw(const Xyz::Matrix4F& mv_matrix, const Xyz::Matrix4F& p_matrix);

    bool show_mesh = false;
private:
    int line_count_ = 0;
    int count_ = 0;
    std::vector<Tungsten::BufferHandle> buffers_;
    Tungsten::VertexArrayHandle vertex_array_;
    Tungsten::TextureHandle texture_;
    Render3DShaderProgram program_;
    Unicolor3DShaderProgram line_program_;
};
