//
//  native_play
//

#pragma once

class Application: public ouzel::Noncopyable
{
public:
    virtual ~Application();

    virtual void begin();

    bool handleKeyboard(const ouzel::KeyboardEventPtr& event, ouzel::VoidPtr const& sender) const;
    bool handleMouse(const ouzel::MouseEventPtr& event, ouzel::VoidPtr const& sender) const;
    bool handleTouch(const ouzel::TouchEventPtr& event, ouzel::VoidPtr const& sender) const;
    bool handleGamepad(const ouzel::GamepadEventPtr& event, ouzel::VoidPtr const& sender) const;

protected:
    ouzel::scene::LayerPtr layer;
    ouzel::scene::LayerPtr uiLayer;

    ouzel::EventHandlerPtr eventHandler;
};
