//
//  native_play
//

#include "Application.h"
#include "VideoNode.h"
//#include "VideoTextureNode.h"

using namespace ouzel;
using namespace video;
using namespace scene;
using namespace gui;
using namespace input;

Application::~Application()
{
    sharedEngine->getEventDispatcher()->removeEventHandler(_eventHandler);
}

void Application::begin()
{
    _eventHandler = std::make_shared<EventHandler>();

    _eventHandler->keyboardHandler = std::bind(&Application::handleKeyboard, this, std::placeholders::_1, std::placeholders::_2);
    _eventHandler->mouseHandler = std::bind(&Application::handleMouse, this, std::placeholders::_1, std::placeholders::_2);
    _eventHandler->touchHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
    _eventHandler->gamepadHandler = std::bind(&Application::handleGamepad, this, std::placeholders::_1, std::placeholders::_2);

    sharedEngine->getEventDispatcher()->addEventHandler(_eventHandler);

    sharedEngine->getRenderer()->setClearColor(Color(64, 0, 0));
    sharedEngine->getWindow()->setTitle("Sample");

    ScenePtr scene(new Scene());
    sharedEngine->getSceneManager()->setScene(scene);

    _layer = std::make_shared<Layer>();
    scene->addLayer(_layer);

    std::shared_ptr<VideoNode> videoNode = std::make_shared<VideoNode>();
    //std::shared_ptr<VideoTextureNode> videoNode = std::make_shared<VideoTextureNode>();
    videoNode->init();
    _layer->addChild(videoNode);

    _uiLayer = std::make_shared<Layer>();
    scene->addLayer(_uiLayer);

    ButtonPtr button = Button::create("button.png", "button.png", "button_down.png", "button.png", "", Color(255, 255, 255, 255), "");
    button->setPosition(Vector2(-200.0f, -200.0f));
    button->setScale(Vector2(0.3f, 0.3f));
    _uiLayer->addChild(button);

    sharedEngine->getInput()->startGamepadDiscovery();
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
                sharedEngine->getWindow()->setSize(Size2(640.0f, 480.0f));
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
