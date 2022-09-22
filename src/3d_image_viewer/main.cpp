//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-09-14.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <iostream>
#include <Argos/Argos.hpp>
#include <Tungsten/SdlApplication.hpp>
#include <Yimage/Yimage.hpp>
#include "Render3DShaderProgram.hpp"

class ImageViewer : public Tungsten::EventLoop
{
public:
    ImageViewer(yimage::Image img)
        : img_(std::move(img))
    {}

    void on_startup(Tungsten::SdlApplication& app) override
    {
        EventLoop::on_startup(app);
    }

    bool on_event(Tungsten::SdlApplication& app, const SDL_Event& event) override
    {
        return EventLoop::on_event(app, event);
    }

    void on_draw(Tungsten::SdlApplication& app) override
    {
        EventLoop::on_draw(app);
    }
private:
    yimage::Image img_;
    std::vector<Tungsten::BufferHandle> buffers_;
    Tungsten::VertexArrayHandle vertex_array_;
    Tungsten::TextureHandle texture_;
};

int main(int argc, char* argv[])
{
    try
    {
        argos::ArgumentParser parser(argv[0]);
        parser.add(argos::Argument("IMAGE")
                       .help("An image file (PNG or JPEG)."));
        Tungsten::SdlApplication::add_command_line_options(parser);
        auto args = parser.parse(argc, argv);
        auto event_loop = std::make_unique<ImageViewer>(
            yimage::read_image(args.value("IMAGE").as_string()));
        Tungsten::SdlApplication app("ShowPng", std::move(event_loop));
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
