//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-09-14.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/Tungsten.hpp>
#include <Yimage/Yimage.hpp>
#include "Cross.hpp"
#include "Hud.hpp"
#include "RingBuffer.hpp"
#include "Sphere.hpp"
#include "SpherePosCalculator.hpp"
#include "Debug.hpp"
#include "HelveticaNeue32.hpp"

constexpr double MAX_CENTER_POINT_AGE = 0.05;
constexpr double MAX_SPEED = 4;
constexpr int MAX_ZOOM_LEVEL = 33;

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

double get_view_angle(int zoom_level)
{
    int angle;
    if (zoom_level < 0)
        angle = 4;
    else if (zoom_level < 5)
        angle = 4 + zoom_level;
    else if (zoom_level < 11)
        angle = 10 + 2 * (zoom_level - 5);
    else if (zoom_level < 31)
        angle = 24 + 4 * (zoom_level - 11);
    else if (zoom_level < 33)
        angle = 106 + 6 * (zoom_level - 31);
    else
        angle = 120;
    return Xyz::to_radians(angle);
}

class ImageViewer : public Tungsten::EventLoop
{
public:
    explicit ImageViewer(Yimage::Image img)
        : img_(std::move(img))
    {
        pos_calculator_.set_view_angle(get_view_angle(zoom_level_));
        pos_calculator_.set_eye_dist(0.5);
    }

    void set_image(Yimage::Image img)
    {
        img_ = std::move(img);
        if (sphere_)
            sphere_->set_image(img_);
    }

    void set_view_direction(double azimuth, double polar)
    {
        pos_calculator_.set_fixed_point({0, 0},
                                        {1.0, azimuth, polar});
    }

    void on_startup(Tungsten::SdlApplication& app) override
    {
        app.throttle_events(SDL_MOUSEWHEEL, 50);
        app.throttle_events(SDL_MULTIGESTURE, 50);
        set_swap_interval(app, Tungsten::SwapInterval::ADAPTIVE_VSYNC_OR_VSYNC);
        sphere_ = std::make_unique<Sphere>(img_, 16, 60);
        cross_ = std::make_unique<Cross>();
        hud_ = std::make_unique<Hud>();

        auto center = to_degrees(pos_calculator_.calc_center_sphere_pos());
        hud_->set_angles(center.azimuth, center.polar);
        hud_->set_zoom(zoom_level_);

        auto& fm = Tungsten::FontManager::instance();
        fm.add_font(get_helveticaneue_32());
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        switch (event.type)
        {
        case SDL_MOUSEWHEEL:
            return on_mouse_wheel(app, event.wheel);
        case SDL_MOUSEMOTION:
            return on_mouse_motion(app, event.motion);
        case SDL_MOUSEBUTTONDOWN:
            return on_mouse_button_down(app, event.button);
        case SDL_MOUSEBUTTONUP:
            return on_mouse_button_up(app, event.button);
        case SDL_KEYDOWN:
            return on_key_down(app, event.key);
        case SDL_MULTIGESTURE:
            return on_multi_gesture(app, event.mgesture);
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
        auto degrees = Xyz::to_degrees(*position);
        hud_->set_angles(degrees.azimuth, degrees.polar);
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
        hud_->draw(Xyz::Vector2F(app.window_size()));

        if (motion_)
            redraw();
    }

    void set_zoom_level(int zoom_level)
    {
        zoom_level = std::clamp(zoom_level, 0, MAX_ZOOM_LEVEL);
        if (zoom_level != zoom_level_)
        {
            zoom_level_ = zoom_level;
            pos_calculator_.set_view_angle(get_view_angle(zoom_level_));
            hud_->set_zoom(zoom_level_);
            redraw();
        }
    }
private:
    void zoom_out(int n = 1)
    {
        set_zoom_level(zoom_level_ + n);
    }

    void zoom_in(int n = 1)
    {
        set_zoom_level(zoom_level_ - n);
   }

    bool on_mouse_wheel(const Tungsten::SdlApplication&,
                        const SDL_MouseWheelEvent& event)
    {
        if (event.y > 1)
            zoom_out(event.y > 2 ? 2 : 1);
        else if (event.y < -1)
            zoom_in(event.y < -2 ? 2 : 1);
        return true;
    }

    bool on_mouse_motion(const Tungsten::SdlApplication& app,
                         const SDL_MouseMotionEvent& event)
    {
        auto [w, h] = app.window_size();
        Xyz::Vector2D new_mouse_pos(
            2.0 * event.x / double(w) - 1,
            2.0 * (h - event.y) / double(h) - 1);

        if (is_panning_)
        {
            pos_calculator_.set_fixed_point(new_mouse_pos,
                                            pos_calculator_.fixed_point().second);
            auto center = Xyz::to_spherical(pos_calculator_.calc_center_pos());
            auto degrees = Xyz::to_degrees(center);
            hud_->set_angles(degrees.azimuth, degrees.polar);
            prev_center_points_.push({
                std::chrono::high_resolution_clock::now(),
                center
            });
            redraw();
        }

        mouse_pos_ = new_mouse_pos;
        return true;
    }

    bool on_mouse_button_down(const Tungsten::SdlApplication&,
                              const SDL_MouseButtonEvent& event)
    {
        if (event.button == SDL_BUTTON_LEFT)
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
        return true;
    }

    bool on_mouse_button_up(const Tungsten::SdlApplication&,
                            const SDL_MouseButtonEvent& event)
    {
        if (event.button == SDL_BUTTON_LEFT)
        {
            motion_ = calculate_motion(prev_center_points_);
            if (motion_)
                redraw();
            is_panning_ = false;
            pos_calculator_.clear_fixed_point();
        }
        return true;
    }

    bool on_key_down(const Tungsten::SdlApplication& app,
                     const SDL_KeyboardEvent& event)
    {
        if (event.repeat)
            return true;

        if (event.keysym.sym == SDLK_m)
        {
            sphere_->show_mesh = !sphere_->show_mesh;
            cross_->visible = !cross_->visible;
            hud_->visible = !hud_->visible;
            redraw();
            return true;
        }
        else if (event.keysym.sym == SDLK_f)
        {
            bool is_fullscreen = SDL_GetWindowFlags(app.window()) & SDL_WINDOW_FULLSCREEN;
            SDL_SetWindowFullscreen(app.window(), is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        }

        return false;
    }

    bool on_multi_gesture(const Tungsten::SdlApplication&,
                          const SDL_MultiGestureEvent& event)
    {
        constexpr float THRESHOLD = 0.01;
        JEB_SHOW(event.numFingers, event.dDist, event.padding, event.type);
        if (event.numFingers == 2)
        {
            if  (event.dDist < -THRESHOLD)
                zoom_out();
            else if (event.dDist > THRESHOLD)
                zoom_in();
        }
        return true;
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

        double angle = 0.5 * get_view_angle(zoom_level_);
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

        JEB_SHOW(secs, azimuth_speed, polar_speed, max_speed);
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
        if (az < -pi)
            az += 2 * pi;
        else if (az > pi)
            az -= 2 * pi;
        auto po = Xyz::clamp(motion.origin.polar + motion.polar_speed * factor,
                             -pi / 2, pi / 2);

        return Xyz::SphericalPointD(1.0, az, po);
    }

    int zoom_level_ = 20;
    Xyz::Vector2D mouse_pos_;
    Yimage::Image img_;
    SpherePosCalculator pos_calculator_;
    bool is_panning_ = false;
    std::unique_ptr<Cross> cross_;
    std::unique_ptr<Sphere> sphere_;
    std::unique_ptr<Hud> hud_;
    PrevPositionList prev_center_points_;
    std::optional<ScreenMotion> motion_;
};

Tungsten::SdlApplication the_app;

extern "C"
{
    void load_image(const char* file_path,
                    int azimuth, int polar, int zoom_level)
    {
        try
        {
            JEB_SHOW(file_path, azimuth, polar);
            auto* viewer = dynamic_cast<ImageViewer*>(the_app.event_loop());
            if (!viewer)
            {
                std::cerr << "The ImageViewer has not been initialized yet.\n";
                return;
            }
            viewer->clear_redraw();
            viewer->set_image(Yimage::read_image(file_path));
            viewer->set_view_direction(Xyz::to_radians(azimuth),
                                       Xyz::to_radians(polar));
            viewer->set_zoom_level(zoom_level);
            viewer->redraw();
        }
        catch (std::exception& ex)
        {
            std::cerr << ex.what() << "\n";
        }
    }
}

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
        Yimage::Image img;
        if (auto img_arg = args.value("IMAGE"))
            img = Yimage::read_image(img_arg.as_string());
        auto event_loop = std::make_unique<ImageViewer>(img);
        the_app = Tungsten::SdlApplication("ShowPng", std::move(event_loop));
        #ifndef __EMSCRIPTEN__
        the_app.set_event_loop_mode(Tungsten::EventLoopMode::WAIT_FOR_EVENTS);
        #endif
        the_app.read_command_line_options(args);
        the_app.run();
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
        return 1;
    }
    return 0;
}
