//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-09-14.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <chrono>
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>
#include <Yimage/Yimage.hpp>
#include "Cross.hpp"
#include "DefaultImage.hpp"
#include "RingBuffer.hpp"
#include "Sphere.hpp"
#include "SpherePosCalculator.hpp"
#include "YimageGl.hpp"
#include "Debug.hpp"

constexpr double MAX_CENTER_POINT_AGE = 0.05;
constexpr double MAX_SPEED = 4;

using time_point = std::chrono::high_resolution_clock::time_point;
using PrevPositionList = RingBuffer<std::pair<time_point, Xyz::SphericalPointD>, 4>;

struct ScreenMotion
{
    time_point start_time = {};
    time_point end_time = {};
    Xyz::SphericalPointD origin;
    double azimuth_speed = 0;
    double polar_speed = 0;
};

class ImageViewer : public Tungsten::EventLoop
{
public:
    explicit ImageViewer(yimage::Image img)
        : img_(std::move(img))
    {
        pos_calculator_.set_view_angle(Xyz::to_radians(scale_));
        pos_calculator_.set_eye_dist(0.5);
    }

    void on_startup(Tungsten::SdlApplication& app) override
    {
        app.set_swap_interval(1);
        sphere_ = std::make_unique<Sphere>(img_, 16, 60);
        cross_ = std::make_unique<Cross>();
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        switch (event.type)
        {
        case SDL_MOUSEWHEEL:
            return on_mouse_wheel(app, event);
        case SDL_MOUSEMOTION:
            return on_mouse_motion(app, event);
        case SDL_MOUSEBUTTONDOWN:
            return on_mouse_button_down(app, event);
        case SDL_MOUSEBUTTONUP:
            return on_mouse_button_up(app, event);
        case SDL_KEYDOWN:
            return on_key_down(app, event);
        default:
            return false;
        }
    }

    void on_update(Tungsten::SdlApplication& app) override
    {
        if (!motion_)
            return;

        auto position = calculate_current_position(*motion_);
        if (!position)
        {
            motion_.reset();
            return;
        }

        pos_calculator_.set_fixed_point({0, 0}, *position);
        redraw();
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto mv_matrix = get_mv_matrix(app);
        auto p_matrix = get_p_matrix(app);
        sphere_->draw(mv_matrix, p_matrix);
        cross_->draw();

        if (motion_)
            redraw();
    }

private:
    bool on_mouse_wheel(const Tungsten::SdlApplication&,
                        const SDL_Event& event)
    {
        if (event.wheel.y > 0)
            scale_ += 4;
        else
            scale_ -= 4;

        if (scale_ > 120)
            scale_ = 120;
        else if (scale_ < 8)
            scale_ = 4;

        pos_calculator_.set_view_angle(Xyz::to_radians(scale_));
        redraw();
        return true;
    }

    bool on_mouse_motion(const Tungsten::SdlApplication& app,
                         const SDL_Event& event)
    {
        auto [w, h] = app.window_size();
        Xyz::Vector2D new_mouse_pos(
            2.0 * event.motion.x / double(w) - 1,
            2.0 * (h - event.motion.y) / double(h) - 1);

        if (is_panning_)
        {
            pos_calculator_.set_fixed_point(new_mouse_pos,
                                            pos_calculator_.fixed_point().second);
            prev_center_points_.push({
                std::chrono::high_resolution_clock::now(),
                Xyz::to_spherical(pos_calculator_.calc_center_pos())
            });
            redraw();
        }

        mouse_pos_ = new_mouse_pos;
        return true;
    }

    bool on_mouse_button_down(const Tungsten::SdlApplication&,
                              const SDL_Event& event)
    {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            auto center = Xyz::to_spherical(pos_calculator_.calc_center_pos());
            prev_center_points_.clear();
            prev_center_points_.push({std::chrono::high_resolution_clock::now(), center});
            is_panning_ = true;
            pos_calculator_.set_fixed_point(
                mouse_pos_,
                pos_calculator_.calc_sphere_pos(mouse_pos_));
            motion_ = {};
        }
        else if (event.button.button == SDL_BUTTON_RIGHT)
        {
            auto pos = pos_calculator_.calc_sphere_pos(mouse_pos_);
            std::clog << mouse_pos_ << " -> " << Xyz::to_degrees(pos) << "\n";
            pos = Xyz::to_spherical(pos_calculator_.calc_center_pos());
            std::clog << "  center: " << Xyz::to_degrees(pos) << "\n";
        }
        return true;
    }

    bool on_mouse_button_up(const Tungsten::SdlApplication&,
                            const SDL_Event& event)
    {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            motion_ = calculate_motion(prev_center_points_);
            if (motion_)
                redraw();
            is_panning_ = false;
            pos_calculator_.clear_fixed_point();
        }
        return true;
    }

    bool on_key_down(const Tungsten::SdlApplication&,
                     const SDL_Event& event)
    {
        if (event.key.repeat)
            return true;

        if (event.key.keysym.sym == SDLK_m)
        {
            sphere_->show_mesh = !sphere_->show_mesh;
            cross_->visible = !cross_->visible;
            redraw();
            return true;
        }
        return false;
    }

    [[nodiscard]]
    Xyz::Matrix4F get_mv_matrix(const Tungsten::SdlApplication& app)
    {
        auto [w, h] = app.window_size();
        pos_calculator_.set_screen_res({double(w), double(h)});
        auto eye_vec = Xyz::vector_cast<float>(pos_calculator_.calc_eye_pos());
        auto center_vec = Xyz::vector_cast<float>(pos_calculator_.calc_center_pos());
        auto up_vec = Xyz::vector_cast<float>(pos_calculator_.calc_up_vector());
        return Xyz::make_look_at_matrix(eye_vec, center_vec, up_vec);
    }

    [[nodiscard]]
    Xyz::Matrix4F get_p_matrix(const Tungsten::SdlApplication& app) const
    {
        auto [w, h] = app.window_size();
        float x, y;
        if (w < h)
        {
            x = float(w) / float(h);
            y = 1.0;
        }
        else
        {
            x = 1.0;
            y = float(h) / float(w);
        }

        double angle = 0.5 * Xyz::to_radians(double(scale_));
        auto size = float(0.5 * sin(angle) / (cos(angle) + 0.5));
        return Xyz::make_frustum_matrix<float>(-size * x, size * x, -size * y, size * y, 0.5f, 2.f);
    }

    [[nodiscard]] static std::optional<ScreenMotion>
    calculate_motion(const PrevPositionList& prev_positions)
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();

        auto it = std::find_if(prev_positions.begin(),
                               prev_positions.end(),
                               [&](const auto& p)
                               {
                                   return duration_cast<duration<double>>(
                                       now - p.first).count() < MAX_CENTER_POINT_AGE;
                               });

        if (it == prev_positions.end())
            return {};

        const auto& [time0, pos0] = *it;
        const auto& [time1, pos1] = prev_positions.back();
        auto secs = duration_cast<duration<double>>(now - time0).count();
        auto azimuth_speed = Xyz::clamp((pos1.azimuth - pos0.azimuth) / secs,
                                        -MAX_SPEED, MAX_SPEED);
        auto polar_speed = Xyz::clamp((pos1.polar - pos0.polar) / secs,
                                      -MAX_SPEED, MAX_SPEED);
        auto max_speed = std::max(std::abs(azimuth_speed),
                                  std::abs(polar_speed));

        auto duration = std::chrono::duration<double>(std::sqrt(max_speed));
        auto end_time = now + duration_cast<high_resolution_clock::duration>(duration);
        return ScreenMotion{now, end_time, pos1, azimuth_speed, polar_speed};
    }

    [[nodiscard]] static std::optional<Xyz::SphericalPointD>
    calculate_current_position(const ScreenMotion& motion)
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        if (now >= motion.end_time)
            return {};

        auto secs = duration_cast<duration<double>>(now - motion.start_time).count();

        // I'm using the equation of the "top left" quarter of an ellipse
        // to control the "deceleration" of the screen movement.
        // The ellipses a-value is the square root of the greatest absolute
        // value of the two speed, its b-value is one quarter of the a-value,
        // its center lies at x, y = radius, 0.
        constexpr auto pi = Xyz::Constants<double>::PI;
        auto radius = std::sqrt(std::max(std::abs(motion.azimuth_speed),
                                         std::abs(motion.polar_speed)));
        auto factor = 0.25 * std::sqrt(secs * (2 * radius - secs));
        auto az = motion.origin.azimuth
                  + motion.azimuth_speed * factor;
        auto po = Xyz::clamp(motion.origin.polar + motion.polar_speed * factor,
                             -pi / 2, pi / 2);
        return Xyz::SphericalPointD(1.0, az, po);
    }

    int scale_ = 60;
    Xyz::Vector2D mouse_pos_;
    yimage::Image img_;
    SpherePosCalculator pos_calculator_;
    bool is_panning_ = false;
    std::unique_ptr<Cross> cross_;
    std::unique_ptr<Sphere> sphere_;
    PrevPositionList prev_center_points_;
    std::optional<ScreenMotion> motion_;
};

int main(int argc, char* argv[])
{
    try
    {
        argos::ArgumentParser parser(argv[0]);
        parser.add(argos::Arg("IMAGE")
                       .optional(true)
                       .help("An image file (PNG or JPEG)."));
        Tungsten::SdlApplication::add_command_line_options(parser);
        auto args = parser.parse(argc, argv);
        yimage::Image img;
        if (auto img_arg = args.value("IMAGE"))
            img = yimage::read_image(img_arg.as_string());
        else
            img = yimage::read_jpeg(DEFAULT_IMAGE, std::size(DEFAULT_IMAGE) - 1);
        auto event_loop = std::make_unique<ImageViewer>(img);
        Tungsten::SdlApplication app("ShowPng", std::move(event_loop));
        #ifndef __EMSCRIPTEN__
        app.set_event_loop_mode(Tungsten::EventLoopMode::WAIT_FOR_EVENTS);
        #endif
        app.read_command_line_options(args);
        app.run();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    return 0;
}
