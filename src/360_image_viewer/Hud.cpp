//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-10-22.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Hud.hpp"
#include <Yconvert/Yconvert.hpp>

namespace
{
    std::u32string to_u32string(const std::string& str)
    {
        return Yconvert::convert_to<std::u32string>(
            str,
            Yconvert::Encoding::UTF_8,
            Yconvert::Encoding::UTF_32_NATIVE);
    }
}

Hud::Hud()
    : renderer_(Tungsten::FontManager::instance().default_font())
{}

void Hud::set_angles(double azimuth, double polar)
{
    azimuth_ = azimuth;
    polar_ = polar;
}

void Hud::draw(const Xyz::Vector2F& screen_size)
{
    auto text = "Azimuth: " + std::to_string(azimuth_) + "\n"
                "Polar: " + std::to_string(polar_);
    renderer_.draw(to_u32string(text), {-1, -1}, screen_size,
                   {.color = Yimage::Color::White});
}
