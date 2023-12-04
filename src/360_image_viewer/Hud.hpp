//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-10-22.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Tungsten/Tungsten.hpp>

class Hud
{
public:
    Hud();

    void set_angles(double azimuth, double polar);

    void draw(const Xyz::Vector2F& screen_size);
private:
    Tungsten::TextRenderer renderer_;
    double azimuth_ = {};
    double polar_ = {};
};
