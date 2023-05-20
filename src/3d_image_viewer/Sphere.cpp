//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-04-15.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Sphere.hpp"
#include "ObjFileWriter.hpp"
#include "YimageGl.hpp"

namespace
{
    struct Vertex
    {
        Xyz::Vector3F pos;
        Xyz::Vector2F tex;
    };

    Tungsten::ArrayBuffer<Vertex> make_sphere(int circles, int points)
    {
        if (circles < 2)
            throw std::runtime_error("Number of circles must be at least 2.");
        if (points < 3)
            throw std::runtime_error("Number of points must be at least 3.");
        Tungsten::ArrayBuffer<Vertex> result;
        Tungsten::ArrayBufferBuilder builder(result);

        constexpr auto PI = Xyz::Constants<float>::PI;

        std::vector<float> pos_z_values;
        std::vector<float> z_factors;
        std::vector<float> tex_y_values;
        for (int i = 0; i < circles; ++i)
        {
            const float angle = 0.5f * (-1.f + float(2 * i + 1) / float(circles)) * PI;
            pos_z_values.push_back(sin(angle));
            z_factors.push_back(cos(angle));
            tex_y_values.push_back(1.f - (float(i) + 0.5f) / float(circles));
        }

        for (int i = 0; i <= points; ++i)
        {
            const float angle = (float(i) * 2.f / float(points) - 0.5f) * PI;
            const float pos_x = cos(angle);
            const float pos_y = sin(angle);
            const float tex_x = 1.0f - float(i) / float(points);
            for (int j = 0; j < circles; ++j)
            {
                builder.add_vertex({.pos = {pos_x * z_factors[j],
                                            pos_y * z_factors[j],
                                            pos_z_values[j]},
                                       .tex = {tex_x, tex_y_values[j]}});
            }
        }

        for (int i = 0; i < points; ++i)
        {
            const float tex_x = 1.0f - (float(i) + 0.5f) / float(points);
            builder.add_vertex({.pos = {0, 0, -1}, .tex = {tex_x, 1.0}});
        }

        for (int i = 0; i < points; ++i)
        {
            const float tex_x = 1.0f - (float(i) + 0.5f) / float(points);
            builder.add_vertex({.pos = {0, 0, 1}, .tex = {tex_x, 0.0}});
        }

        for (int i = 0; i < points; ++i)
            builder.add_indexes(i * circles, (i + 1) * circles, i + (circles * (points + 1)));

        for (int i = 0; i < points; ++i)
        {
            for (int j = 0; j < circles - 1; ++j)
            {
                const auto n = uint16_t(i * circles + j);
                builder.add_indexes(n, n + 1, n + circles + 1);
                builder.add_indexes(n, n + circles + 1, n + circles);
            }
        }

        for (int i = 0; i < points; ++i)
            builder.add_indexes((i + 2) * circles - 1, (i + 1) * circles - 1,
                                i + ((circles + 1) * (points + 1)) - 1);

        return result;
    }

    void write(std::ostream& os, Tungsten::ArrayBuffer<Vertex>& buffer)
    {
        ObjFileWriter writer(os);
        for (const auto& vertex: buffer.vertexes)
            writer.write_vertex(vertex.pos);

        for (const auto& vertex: buffer.vertexes)
            writer.write_tex(vertex.tex);

        for (size_t i = 0; i < buffer.indexes.size(); i += 3)
        {
            writer.begin_face();
            for (size_t j = 0; j < 3; ++j)
            {
                auto n = 1 + buffer.indexes[i + j];
                writer.write_face({n, n});
            }
            writer.end_face();
        }
    }

    void triangle_indexes_to_lines(const uint16_t* indexes,
                                   size_t count,
                                   std::vector<uint16_t>& result)
    {
        if (count % 3 != 0)
        {
            throw std::runtime_error("count must be divisible by 3. Value: "
                                     + std::to_string(count));
        }

        std::vector<std::pair<uint16_t, uint16_t>> pairs;
        pairs.reserve(count);
        for (size_t i = 0; i < count; i += 3)
        {
            pairs.emplace_back(std::minmax(indexes[i], indexes[i + 1]));
            pairs.emplace_back(std::minmax(indexes[i + 1], indexes[i + 2]));
            pairs.emplace_back(std::minmax(indexes[i], indexes[i + 2]));
        }

        std::sort(pairs.begin(), pairs.end());
        pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());

        result.reserve(result.size() + pairs.size() * 2);
        for (auto [a, b]: pairs)
        {
            result.push_back(a);
            result.push_back(b);
        }
    }

    Yimage::Image make_dummy_image()
    {
        constexpr size_t WIDTH = 512;
        constexpr size_t HEIGHT = WIDTH / 2;
        Yimage::Image img(Yimage::PixelType::RGB_8, WIDTH, HEIGHT);
        // Blue sky
        Yimage::fill_rgba8(img.mutable_subimage(0, 0, WIDTH, HEIGHT / 2),
                           {0x99, 0xAA, 0xEE, 0xFF});
        // Green ground
        Yimage::fill_rgba8(img.mutable_subimage(0, HEIGHT / 2, WIDTH, HEIGHT),
                           {0x88, 0xDD, 0x33, 0xFF});
        return img;
    }
}

Sphere::Sphere(int circles, int points)
    : Sphere(Yimage::Image(), circles, points)
{}

Sphere::Sphere(const Yimage::Image& img, int circles, int points)
{
    auto array = make_sphere(circles, points);
    vertex_array_ = Tungsten::generate_vertex_array();
    Tungsten::bind_vertex_array(vertex_array_);

    buffers_ = Tungsten::generate_buffers(2);

    auto [vertexes, vertexes_size] = array.array_buffer();
    Tungsten::bind_buffer(GL_ARRAY_BUFFER, buffers_[0]);
    Tungsten::set_buffer_data(GL_ARRAY_BUFFER, GLsizeiptr(vertexes_size),
                              vertexes, GL_STATIC_DRAW);

    count_ = int(array.indexes.size());

    triangle_indexes_to_lines(array.indexes.data(),
                              array.indexes.size(),
                              array.indexes);

    line_count_ = int(array.indexes.size()) - count_;
    auto [indexes, index_size] = array.index_buffer();

    Tungsten::bind_buffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[1]);
    Tungsten::set_buffer_data(GL_ELEMENT_ARRAY_BUFFER,
                              GLsizeiptr(index_size),
                              indexes, GL_STATIC_DRAW);

    texture_ = Tungsten::generate_texture();
    Tungsten::bind_texture(GL_TEXTURE_2D, texture_);

    Tungsten::set_texture_min_filter(GL_TEXTURE_2D, GL_LINEAR);
    Tungsten::set_texture_mag_filter(GL_TEXTURE_2D, GL_LINEAR);
    Tungsten::set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    Tungsten::set_texture_parameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (img)
        set_image(img);
    else
        set_image(make_dummy_image());

    program_.setup();
    Tungsten::use_program(program_.program);
    Tungsten::define_vertex_attribute_float_pointer(
        program_.position, 3, 5 * sizeof(float), 0);
    Tungsten::enable_vertex_attribute(program_.position);
    Tungsten::define_vertex_attribute_float_pointer(
        program_.texture_coord, 2, 5 * sizeof(float), 3 * sizeof(float));
    Tungsten::enable_vertex_attribute(program_.texture_coord);

    line_program_.setup();
    Tungsten::use_program(line_program_.program);
    line_program_.color.set({1.f, 0.f, 0.f, 1.f});
    Tungsten::define_vertex_attribute_float_pointer(
        line_program_.position, 3, 5 * sizeof(float), 0);
    Tungsten::enable_vertex_attribute(line_program_.position);
}

void Sphere::set_image(const Yimage::Image& img)
{
    Tungsten::bind_texture(GL_TEXTURE_2D, texture_);

    auto [format, type] = get_ogl_pixel_type(img.pixel_type());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Tungsten::set_texture_image_2d(GL_TEXTURE_2D, 0, GL_RGB,
                                   int(img.width()), int(img.height()),
                                   format, type,
                                   img.data());
}

void Sphere::draw(const Xyz::Matrix4F& mv_matrix,
                  const Xyz::Matrix4F& p_matrix)
{
    Tungsten::bind_texture(GL_TEXTURE_2D, texture_);
    Tungsten::bind_vertex_array(vertex_array_);
    Tungsten::use_program(program_.program);
    program_.mv_matrix.set(mv_matrix);
    program_.p_matrix.set(p_matrix);
    Tungsten::draw_triangle_elements_16(0, count_);

    if (show_mesh)
    {
        Tungsten::use_program(line_program_.program);
        line_program_.mv_matrix.set(mv_matrix);
        line_program_.p_matrix.set(p_matrix);
        Tungsten::draw_line_elements_16(count_, line_count_);
    }
}
