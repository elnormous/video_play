//
//  native_play
//

#pragma once

class Application: public ouzel::Noncopyable, public ouzel::App
{
public:
    virtual ~Application();

    virtual void begin() override;

    bool handleKeyboard(const ouzel::KeyboardEventPtr& event, ouzel::VoidPtr const& sender) const;
    bool handleMouse(const ouzel::MouseEventPtr& event, ouzel::VoidPtr const& sender) const;
    bool handleTouch(const ouzel::TouchEventPtr& event, ouzel::VoidPtr const& sender) const;
    bool handleGamepad(const ouzel::GamepadEventPtr& event, ouzel::VoidPtr const& sender) const;

protected:
    ouzel::scene::LayerPtr _layer;
    ouzel::scene::LayerPtr _uiLayer;

    ouzel::EventHandlerPtr _eventHandler;
};
