//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-09-24.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iosfwd>
#include <memory>
#include <string>
#include <Xyz/Xyz.hpp>

struct FaceIndex
{
    int vertex;
    int texture = -1;
    int normal = -1;
};

class ObjFileWriter
{
public:
    ObjFileWriter();

    ObjFileWriter(std::ostream& stream);

    std::ostream& stream() const;

    ObjFileWriter& write_vertex(const Xyz::Vector3F& v);

    ObjFileWriter& write_tex(const Xyz::Vector2F& v);

    ObjFileWriter& begin_face();

    ObjFileWriter& write_face(const FaceIndex& face);

    ObjFileWriter& end_face();
private:
    std::unique_ptr<std::ostream> stream_ptr_;
    std::ostream* stream_;
};
