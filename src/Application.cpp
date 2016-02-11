// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Application.h"
#include "VideoNode.h"
#include "VideoTextureNode.h"

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
        
        _eventHandler->keyboardHandler = std::bind(&Application::handleKeyboard, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->mouseHandler = std::bind(&Application::handleMouse, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->gamepadHandler = std::bind(&Application::handleGamepad, this, std::placeholders::_1, std::placeholders::_2);
        
        Engine::getInstance()->getEventDispatcher()->addEventHandler(_eventHandler);
        
        Engine::getInstance()->getRenderer()->setClearColor(Color(64, 0, 0));
        Engine::getInstance()->getRenderer()->setTitle("Sample");
        
        ScenePtr scene(new Scene());
        Engine::getInstance()->getSceneManager()->setScene(scene);
        
        _layer = Layer::create();
        _layer->init();
        scene->addLayer(_layer);
        
        std::shared_ptr<VideoNode> videoNode = std::make_shared<VideoNode>();
        //std::shared_ptr<VideoTextureNode> videoNode = std::make_shared<VideoTextureNode>();
        videoNode->init();
        _layer->addChild(videoNode);
        
        _uiLayer = Layer::create();
        scene->addLayer(_uiLayer);
        
        Engine::getInstance()->getInput()->startGamepadDiscovery();
    }
    
    bool Application::handleKeyboard(const KeyboardEventPtr& event, VoidPtr const& sender) const
    {
        if (event->type == Event::Type::KEY_DOWN ||
            event->type == Event::Type::KEY_REPEAT)
        {
            Vector2 position = _layer->getCamera()->getPosition();
            float rotation = _layer->getCamera()->getRotation();
            float zoom = _layer->getCamera()->getZoom();
            
            switch (event->key)
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
                case KeyboardKey::KEY_1:
                    rotation -= ouzel::TAU / 36.0f;
                    break;
                case KeyboardKey::KEY_2:
                    rotation += ouzel::TAU / 36.0f;
                    break;
                case KeyboardKey::KEY_3:
                    zoom -= 0.1f;
                    break;
                case KeyboardKey::KEY_4:
                    zoom += 0.1f;
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
            _layer->getCamera()->setRotation(rotation);
            _layer->getCamera()->setZoom(zoom);
        }
        
        return true;
    }
    
    bool Application::handleMouse(const MouseEventPtr& event, VoidPtr const& sender) const
    {
        return true;
    }
    
    bool Application::handleTouch(const TouchEventPtr& event, VoidPtr const& sender) const
    {
        return true;
    }
    
    bool Application::handleGamepad(const GamepadEventPtr& event, VoidPtr const& sender) const
    {
        if (event->type == Event::Type::GAMEPAD_BUTTON_CHANGE)
        {
            switch (event->button)
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
        }
        
        return true;
    }
}
