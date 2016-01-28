// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Application.h"
#include "VideoLayer.h"

namespace ouzel
{    
    Application::~Application()
    {
        Engine::getInstance()->getEventDispatcher()->removeEventHandler(_eventHandler);
    }
    
    Settings Application::getSettings()
    {
        Settings settings;
        settings.size = ouzel::Size2(800.0f, 600.0f);
        settings.resizable = true;
        
        return settings;
    }
    
    void Application::begin()
    {
        _eventHandler = std::make_shared<EventHandler>();
        
        _eventHandler->keyDownHandler = std::bind(&Application::handleKeyDown, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->mouseMoveHandler = std::bind(&Application::handleMouseMove, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchBeginHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchMoveHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchEndHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->gamepadButtonChangeHandler = std::bind(&Application::handleGamepadButtonChange, this, std::placeholders::_1, std::placeholders::_2);
        
        Engine::getInstance()->getEventDispatcher()->addEventHandler(_eventHandler);
        
        Engine::getInstance()->getRenderer()->setClearColor(Color(64, 0, 0));
        Engine::getInstance()->getRenderer()->setTitle("Sample");
        
        ScenePtr scene(new Scene());
        Engine::getInstance()->getSceneManager()->setScene(scene);
        
        _layer = std::make_shared<VideoLayer>();
        scene->addLayer(_layer);
        
        _uiLayer = std::make_shared<ouzel::Layer>();
        scene->addLayer(_uiLayer);
        
        Engine::getInstance()->getInput()->startGamepadDiscovery();
    }
    
    bool Application::handleKeyDown(const KeyboardEvent& event, VoidPtr const& sender) const
    {
        Vector2 position = _layer->getCamera()->getPosition();
        
        switch (event.key)
        {
            case KeyboardKey::UP:
                position.y += 10.0f;
                break;
            case KeyboardKey::DOWN:
                position.y -= 10.0f;
                break;
            case KeyboardKey::LEFT:
                position.x -= 10.0f;
                break;
            case KeyboardKey::RIGHT:
                position.x += 10.0f;
                break;
            case KeyboardKey::SPACE:
                break;
            case KeyboardKey::RETURN:
                Engine::getInstance()->getRenderer()->resize(Size2(640.0f, 480.0f));
                break;
            case KeyboardKey::TAB:
                break;
            default:
                break;
        }
        
        _layer->getCamera()->setPosition(position);
        
        return true;
    }
    
    bool Application::handleMouseMove(const MouseEvent& event, VoidPtr const& sender) const
    {
        return true;
    }
    
    bool Application::handleTouch(const TouchEvent& event, VoidPtr const& sender) const
    {
        return true;
    }
    
    bool Application::handleGamepadButtonChange(const GamepadEvent& event, VoidPtr const& sender) const
    {
        switch (event.button)
        {
            case GamepadButton::DPAD_UP:
            case GamepadButton::LEFT_THUMB_UP:
            case GamepadButton::RIGHT_THUMB_UP:
                break;
            case GamepadButton::DPAD_DOWN:
            case GamepadButton::LEFT_THUMB_DOWN:
            case GamepadButton::RIGHT_THUMB_DOWN:
                break;
            case GamepadButton::DPAD_LEFT:
            case GamepadButton::LEFT_THUMB_LEFT:
            case GamepadButton::RIGHT_THUMB_LEFT:
                break;
            case GamepadButton::DPAD_RIGHT:
            case GamepadButton::LEFT_THUMB_RIGHT:
            case GamepadButton::RIGHT_THUMB_RIGHT:
                break;
            case GamepadButton::A:
                break;
            default:
                break;
        }
        
        return true;
    }
}
