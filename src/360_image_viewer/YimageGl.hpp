//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-02-12.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <utility>
#include <Yimage/PixelType.hpp>

std::pair<int, int> get_ogl_pixel_type(Yimage::PixelType type);
