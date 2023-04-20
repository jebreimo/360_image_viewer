//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-12-30.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SpherePosCalculator.hpp"
#include <Xyz/Xyz.hpp>
#include "Debug.hpp"

namespace
{
    constexpr auto PI = Xyz::Constants<double>::PI;

    struct ViewParams
    {
        Xyz::Vector2D screen_res;
        double view_angle;
        double eye_dist;
    };

    [[nodiscard]]
    Xyz::Vector2D calc_screen_factors(const ViewParams& vp)
    {
        auto [hor_res, ver_res] = vp.screen_res;
        double hor_view_angle, ver_view_angle;
        if (hor_res >= ver_res)
        {
            hor_view_angle = vp.view_angle;
            ver_view_angle = ver_res * vp.view_angle / hor_res;
        }
        else
        {
            hor_view_angle = hor_res * vp.view_angle / ver_res;
            ver_view_angle = vp.view_angle;
        }

        auto calc_size = [&vp](double angle)
        {
            return sin(angle / 2) * vp.eye_dist
                   / (vp.eye_dist + cos(angle / 2));
        };

        return {calc_size(hor_view_angle), calc_size(ver_view_angle)};
    }

    [[nodiscard]]
    std::pair<Xyz::Vector3D, Xyz::Vector3D>
    calc_screen_vectors(const ViewParams& vp,
                        const Xyz::SphericalPointD& cp)
    {
        auto up = to_cartesian(Xyz::SphericalPoint(1.0, cp.azimuth, cp.polar + PI / 2));
        auto fwd = to_cartesian(Xyz::SphericalPoint(1.0, cp.azimuth, cp.polar));
        auto right = cross(fwd, up);
        auto factors = calc_screen_factors(vp);
        return {factors[0] * right, factors[1] * up};
    }

    [[nodiscard]]
    Xyz::Vector3D
    calc_point_on_sphere(const ViewParams& vp,
                         const Xyz::SphericalPointD& screen_center,
                         const Xyz::Vector2D& screen_pos)
    {
        auto [right, up] = calc_screen_vectors(vp, screen_center);
        auto scr = screen_pos[0] * right + screen_pos[1] * up;
        auto eye = -vp.eye_dist * to_cartesian(screen_center);
        auto delta = scr - eye;
        auto a = get_length_squared(delta);
        auto b = 2 * dot(eye, delta);
        auto c = get_length_squared(eye) - 1;
        auto solutions = Xyz::solve_real_quadratic_equation(a, b, c);
        if (!solutions)
            throw std::runtime_error("Can not find a point on the sphere.");

        auto t = std::max(solutions->first, solutions->second);
        return eye + t * delta;
    }

    [[nodiscard]]
    Xyz::SphericalPointD
    calc_center_of_screen(const ViewParams& vp,
                          const Xyz::SphericalPointD& fixed_sphere_pos,
                          const Xyz::Vector2D& fixed_screen_pos)
    {
        auto [x, y, z] = calc_point_on_sphere(
            vp,
            {1.0, 0.0, 0.0},
            fixed_screen_pos
        );

        auto radius = sqrt(x * x + z * z);
        auto phi0 = atan(z / x);
        auto sin_phi = sin(fixed_sphere_pos.polar) / radius;
        sin_phi = Xyz::clamp(sin_phi, -1.0, 1.0);
        auto phi = asin(sin_phi) - phi0;
        auto sp = to_cartesian(fixed_sphere_pos);
        auto theta = get_ccw_angle(Xyz::Vector2D(x * cos(phi + phi0), y),
                                   Xyz::Vector2D(sp[0], sp[1]));
        if (theta > PI)
            theta = theta - 2 * PI;
        return {1.0, theta, phi};
    }
}

Xyz::Vector3D SpherePosCalculator::calc_center_pos()
{
    ensure_valid_center_pos();
    return Xyz::to_cartesian(*center_pos_);
}

Xyz::Vector3D SpherePosCalculator::calc_eye_pos()
{
    return eye_dist_ * -calc_center_pos();
}

Xyz::SphericalPointD
SpherePosCalculator::calc_sphere_pos(const Xyz::Vector2D& screen_pos)
{
    ensure_valid_center_pos();

    ViewParams vp = {screen_res_, view_angle_, eye_dist_};
    if (screen_pos == Xyz::Vector2D(0, 0))
        return *center_pos_;

    return to_spherical(calc_point_on_sphere(vp, *center_pos_, screen_pos));
}

Xyz::Vector3D SpherePosCalculator::calc_up_vector()
{
    ensure_valid_center_pos();
    return to_cartesian(Xyz::SphericalPoint(center_pos_->radius,
                                            center_pos_->azimuth,
                                            center_pos_->polar + PI / 2));
}

std::pair<const Xyz::Vector2D&, const Xyz::SphericalPointD&>
SpherePosCalculator::fixed_point() const
{
    return {fixed_screen_pos_, fixed_sphere_pos_};
}

void SpherePosCalculator::set_fixed_point(const Xyz::Vector2D& screen_pos,
                                          const Xyz::SphericalPointD& sphere_pos)
{
    fixed_screen_pos_ = screen_pos;
    fixed_sphere_pos_ = {1, sphere_pos.azimuth, sphere_pos.polar};
    invalidate_center_pos();
}

void SpherePosCalculator::clear_fixed_point()
{
    ensure_valid_center_pos();
    fixed_screen_pos_ = {0, 0};
    fixed_sphere_pos_ = *center_pos_;
}

double SpherePosCalculator::eye_dist() const
{
    return eye_dist_;
}

void SpherePosCalculator::set_eye_dist(double eye_dist)
{
    eye_dist_ = eye_dist;
    invalidate_center_pos();
}

double SpherePosCalculator::view_angle() const
{
    return view_angle_;
}

void SpherePosCalculator::set_view_angle(double view_angle)
{
    view_angle_ = view_angle;
    invalidate_center_pos();
}

const Xyz::Vector2D& SpherePosCalculator::screen_res() const
{
    return screen_res_;
}

void SpherePosCalculator::set_screen_res(const Xyz::Vector2D& screen_res)
{
    if (screen_res_ != screen_res)
    {
        screen_res_ = screen_res;
        invalidate_center_pos();
    }
}

void SpherePosCalculator::invalidate_center_pos()
{
    center_pos_ = fixed_screen_pos_ == Xyz::Vector2D(0, 0)
                  ? fixed_sphere_pos_
                  : std::optional<Xyz::SphericalPointD>();
}

void SpherePosCalculator::ensure_valid_center_pos()
{
    if (!center_pos_)
    {
        ViewParams vp = {screen_res_, view_angle_, eye_dist_};
        center_pos_ = calc_center_of_screen(vp, fixed_sphere_pos_,
                                            fixed_screen_pos_);
    }
}
