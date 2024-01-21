//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-12-30.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <optional>
#include <Xyz/SphericalPoint.hpp>

class SpherePosCalculator
{
public:
    [[nodiscard]]
    Xyz::Vector3D calc_center_pos();

    [[nodiscard]]
    Xyz::Vector3D calc_eye_pos();

    [[nodiscard]]
    Xyz::SphericalPointD calc_sphere_pos(const Xyz::Vector2D& screen_pos);

    [[nodiscard]]
    Xyz::SphericalPointD calc_center_sphere_pos();

    [[nodiscard]]
    Xyz::Vector3D calc_up_vector();

    [[nodiscard]]
    std::pair<const Xyz::Vector2D&, const Xyz::SphericalPointD&>
    fixed_point() const;

    void set_fixed_point(const Xyz::Vector2D& screen_pos,
                         const Xyz::SphericalPointD& sphere_pos);

    void clear_fixed_point();

    [[nodiscard]]
    double eye_dist() const;

    void set_eye_dist(double eye_dist);

    [[nodiscard]]
    double view_angle() const;

    void set_view_angle(double view_angle);

    [[nodiscard]]
    const Xyz::Vector2D& screen_res() const;

    void set_screen_res(const Xyz::Vector2D& screen_res);
private:
    void invalidate_center_pos();

    void ensure_valid_center_pos();

    Xyz::Vector2D fixed_screen_pos_;
    Xyz::SphericalPointD fixed_sphere_pos_ = {1, 0, 0};
    std::optional<Xyz::SphericalPointD> center_pos_ = fixed_sphere_pos_;
    double eye_dist_ = {};
    double view_angle_ = {};
    Xyz::Vector2D screen_res_;
};
