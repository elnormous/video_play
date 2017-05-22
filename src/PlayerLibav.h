//
//  native_play
//

#pragma once

#include "Video.h"

class PlayerLibav: public ouzel::Noncopyable
{
public:
    PlayerLibav(const std::string& stream);

    bool handleKeyboard(ouzel::Event::Type type, const ouzel::KeyboardEvent& event);
    bool handleMouse(ouzel::Event::Type type, const ouzel::MouseEvent& event) const;
    bool handleTouch(ouzel::Event::Type type, const ouzel::TouchEvent& event) const;
    bool handleGamepad(ouzel::Event::Type type, const ouzel::GamepadEvent& event) const;

protected:
    ouzel::scene::Layer layer;
    ouzel::scene::Camera camera;
    ouzel::scene::Scene scene;

    ouzel::scene::Node videoNode;
    Video video;
    ouzel::scene::Rotate rotate;
    ouzel::scene::Repeat repeat;

    ouzel::scene::Layer uiLayer;
    ouzel::scene::Camera uiCamera;

    ouzel::gui::Button button;

    ouzel::EventHandler eventHandler;
};
