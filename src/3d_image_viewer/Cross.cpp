//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-04-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Cross.hpp"

Cross::Cross()
    : buffer_(Tungsten::generate_buffer()),
      vertex_array_(Tungsten::generate_vertex_array())
{
    float array[] = {-1, 0, 0,
                     1, 0, 0,
                     0, -1, 0,
                     0, 1, 1};

    Tungsten::bind_vertex_array(vertex_array_);
    Tungsten::bind_buffer(GL_ARRAY_BUFFER, buffer_);
    Tungsten::set_buffer_data(GL_ARRAY_BUFFER, sizeof(array),
                              array, GL_STATIC_DRAW);
    count_ = std::size(array) / 3;
    program_.setup();
    Tungsten::use_program(program_.program);
    program_.color.set({1.f, 1.f, 0.f, 1.f});
    program_.mv_matrix.set(Xyz::make_identity_matrix<float, 4>());
    program_.p_matrix.set(Xyz::make_identity_matrix<float, 4>());
    Tungsten::define_vertex_attribute_float_pointer(
        program_.position, 3, 3 * sizeof(float), 0);
    Tungsten::enable_vertex_attribute(program_.position);
}

void Cross::draw()
{
    if (!visible)
        return;

    Tungsten::bind_vertex_array(vertex_array_);
    Tungsten::use_program(program_.program);
    Tungsten::draw_line_array(0, count_);
}
