// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

namespace ouzel
{
    class Application: public Noncopyable, public App
    {
    public:
        virtual ~Application();
        
        virtual Settings getSettings() override;
        
        virtual void begin() override;
        
        bool handleKeyboard(const KeyboardEventPtr& event, VoidPtr const& sender) const;
        bool handleMouse(const MouseEventPtr& event, VoidPtr const& sender) const;
        bool handleTouch(const TouchEventPtr& event, VoidPtr const& sender) const;
        bool handleGamepad(const GamepadEventPtr& event, VoidPtr const& sender) const;
        
    protected:
        LayerPtr _layer;
        LayerPtr _uiLayer;
        
        EventHandlerPtr _eventHandler;
    };
}
