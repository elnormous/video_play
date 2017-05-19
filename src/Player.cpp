//
//  native_play
//

#include "Player.h"
//#include "VideoTextureNode.h"

using namespace ouzel;
using namespace graphics;
using namespace scene;
using namespace gui;
using namespace input;

Player::Player(const std::string& stream):
    button("button.png", "button.png", "button_down.png", "button.png", "", "", Color(255, 255, 255, 255)),
    rotate(1.0f, Vector3(0.0f, TAU, 0.0f)),
    repeat(&rotate, 100)
{
    eventHandler.keyboardHandler = std::bind(&Player::handleKeyboard, this, std::placeholders::_1, std::placeholders::_2);
    eventHandler.mouseHandler = std::bind(&Player::handleMouse, this, std::placeholders::_1, std::placeholders::_2);
    eventHandler.touchHandler = std::bind(&Player::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
    eventHandler.gamepadHandler = std::bind(&Player::handleGamepad, this, std::placeholders::_1, std::placeholders::_2);

    sharedEngine->getEventDispatcher()->addEventHandler(&eventHandler);

    sharedEngine->getRenderer()->setClearColor(Color(64, 0, 0));
    sharedEngine->getWindow()->setTitle("Sample");

    sharedEngine->getSceneManager()->setScene(&scene);

    video.init(stream);
    videoNode.addComponent(&video);

    camera.setType(Camera::Type::PERSPECTIVE);
    camera.setNearPlane(1.0f);
    camera.setFarPlane(1000.0f);
    camera.setPosition(Vector3(0.0f, 0.0f, -180.0f));

    layer.addChild(&camera);
    layer.addChild(&videoNode);

    videoNode.addComponent(&repeat);
    repeat.start();

    scene.addLayer(&layer);

    uiLayer.addChild(&uiCamera);
    scene.addLayer(&uiLayer);

    button.setPosition(Vector2(-200.0f, -200.0f));
    button.setScale(Vector2(0.1f, 0.1f));
    uiLayer.addChild(&button);

    sharedEngine->getInput()->startGamepadDiscovery();
}

bool Player::handleKeyboard(ouzel::Event::Type type, const KeyboardEvent& event)
{
    if (type == Event::Type::KEY_DOWN ||
        type == Event::Type::KEY_REPEAT)
    {
        Vector2 position = camera.getPosition();
        float rotation = camera.getRotation().getEulerAngleX();
        Vector3 scale = camera.getScale();

        switch (event.key)
        {
            case KeyboardKey::UP:
                position.y() += 10.0f;
                break;
            case KeyboardKey::DOWN:
                position.y() -= 10.0f;
                break;
            case KeyboardKey::LEFT:
                position.x() -= 10.0f;
                break;
            case KeyboardKey::RIGHT:
                position.x() += 10.0f;
                break;
            case KeyboardKey::KEY_1:
                rotation -= ouzel::TAU / 36.0f;
                break;
            case KeyboardKey::KEY_2:
                rotation += ouzel::TAU / 36.0f;
                break;
            case KeyboardKey::KEY_3:
                scale.x() += 0.1f;
                scale.y() += 0.1f;
                break;
            case KeyboardKey::KEY_4:
                scale.x() -= 0.1f;
                scale.y() -= 0.1f;
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

        camera.setPosition(position);
        camera.setRotation(rotation);
        camera.setScale(scale);
    }

    return true;
}

bool Player::handleMouse(ouzel::Event::Type type, const MouseEvent& event) const
{
    return true;
}

bool Player::handleTouch(ouzel::Event::Type type, const TouchEvent& event) const
{
    return true;
}

bool Player::handleGamepad(ouzel::Event::Type type, const GamepadEvent& event) const
{
    if (type == Event::Type::GAMEPAD_BUTTON_CHANGE)
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
            case GamepadButton::FACE1:
                break;
            default:
                break;
        }
    }

    return true;
}
