//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-02-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "YimageGl.hpp"
#include <stdexcept>
#include <string>
#include <utility>
#include <GL/glew.h>

std::pair<int, int> get_ogl_pixel_type(yimage::PixelType type)
{
    switch (type)
    {
    case yimage::PixelType::MONO_8:
        return {GL_RED, GL_UNSIGNED_BYTE};
    case yimage::PixelType::MONO_ALPHA_8:
        return {GL_RG, GL_UNSIGNED_BYTE};
    case yimage::PixelType::RGB_8:
        return {GL_RGB, GL_UNSIGNED_BYTE};
    case yimage::PixelType::RGBA_8:
        return {GL_RGBA, GL_UNSIGNED_BYTE};
    case yimage::PixelType::MONO_1:
    case yimage::PixelType::MONO_2:
    case yimage::PixelType::MONO_4:
    case yimage::PixelType::MONO_16:
    case yimage::PixelType::ALPHA_MONO_8:
    case yimage::PixelType::ALPHA_MONO_16:
    case yimage::PixelType::MONO_ALPHA_16:
    case yimage::PixelType::RGB_16:
    case yimage::PixelType::ARGB_8:
    case yimage::PixelType::ARGB_16:
    case yimage::PixelType::RGBA_16:
    default:
        break;
    }
    throw std::runtime_error("GLES has no corresponding pixel format: "
                             + std::to_string(int(type)));
}
