//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-09-24.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "ObjFileWriter.hpp"
#include <iostream>
#include <sstream>

namespace
{
    std::pair<std::ios::fmtflags, std::streampos>
    set_float_format(std::ostream& s)
    {
        return {s.setf(std::ios::fixed), s.precision(FLT_DECIMAL_DIG)};
    }

    void restore_format(std::ostream& s,
                        std::pair<std::ios::fmtflags, std::streampos> fmt)
    {
        s.setf(fmt.first);
        s.precision(fmt.second);
    }
}

ObjFileWriter::ObjFileWriter()
    : stream_(&std::cout)
{}

ObjFileWriter::ObjFileWriter(std::ostream& stream)
    : stream_(&stream)
{}

std::ostream& ObjFileWriter::stream() const
{
    return *stream_;
}

ObjFileWriter& ObjFileWriter::write_vertex(const Xyz::Vector3F& v)
{
    const auto fmt = set_float_format(*stream_);
    *stream_ << "v " << v[0] << " " << v[1] << " " << v[2] << '\n';
    restore_format(*stream_, fmt);
    return *this;
}

ObjFileWriter& ObjFileWriter::write_tex(const Xyz::Vector2F& v)
{
    const auto fmt = set_float_format(*stream_);
    *stream_ << "vt " << v[0] << " " << v[1] << '\n';
    restore_format(*stream_, fmt);
    return *this;
}

ObjFileWriter& ObjFileWriter::begin_face()
{
    *stream_ << "f";
    return *this;
}

ObjFileWriter& ObjFileWriter::write_face(const FaceIndex& face)
{
    *stream_ << ' ' << face.vertex;
    if (face.texture >= 0)
        *stream_ << '/' << face.texture;
    else if (face.normal >= 0)
        *stream_ << '/';
    if (face.normal >= 0)
        *stream_ << '/' << face.normal;
    return *this;
}

ObjFileWriter& ObjFileWriter::end_face()
{
    *stream_ << '\n';
    return *this;
}
