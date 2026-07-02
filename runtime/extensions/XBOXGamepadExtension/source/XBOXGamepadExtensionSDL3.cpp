#ifdef NUCLEAR_BACKEND_SDL3
#include "XBOXGamepadExtension.h"

#include "SDL3Backend.h"

int XBOXGamepadExtension::ConvertToPlatformButton(int button)
{
    switch (button)
    {
        case 0: return SDL_GAMEPAD_BUTTON_SOUTH;
        case 1: return SDL_GAMEPAD_BUTTON_EAST;
        case 2: return SDL_GAMEPAD_BUTTON_WEST;
        case 3: return SDL_GAMEPAD_BUTTON_NORTH;
        case 4: return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
        case 5: return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
        case 6: return SDL_GAMEPAD_BUTTON_BACK;
        case 7: return SDL_GAMEPAD_BUTTON_START;
        case 8: return SDL_GAMEPAD_BUTTON_LEFT_STICK;
        case 9: return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
        case 10: return SDL_GAMEPAD_BUTTON_DPAD_UP;
        case 11: return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        case 12: return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        case 13: return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        default: return -1;
    }
    return -1;
}

bool XBOXGamepadExtension::ButtonPressed(int gamepadBitmask, int button)
{
    if (button < 0 || button > 13) return false;
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
	for (int i = 0; i < 4; i++)
	{
		if (gamepadBitmask & (1 << i))
		{
            if (!input->IsGamepadConnected(i)) return false;
            SDL_Gamepad* gamepad = input->gamepads.at(i);
			if (SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)ConvertToPlatformButton(button))) return true;
		}
	}
    return false;
}

int XBOXGamepadExtension::ButtonPressedExpression(int gamepadBitmask, int button)
{
    //This is replicating a bug in the orignal extenstion where left and right stick return the value of the shoulder button
    if (button == 8 || button == 9) return ButtonPressed(gamepadBitmask, button - 4);
    
    return ButtonPressed(gamepadBitmask, button) ? 1 : 0;
}

int XBOXGamepadExtension::ButtonPressedSpecificExpression(int button, int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, button) ? 1 : 0;
}

int XBOXGamepadExtension::FindPressedButton(int gamepadBitmask)
{
    for (int i = 0; i < 14; i++)
    {
        if (ButtonPressed(gamepadBitmask, i)) return i;
    }
    return -1;
}

int XBOXGamepadExtension::GamepadIsConnected(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    return input->IsGamepadConnected(gamepadIndex);
}

bool XBOXGamepadExtension::ButtonAPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 0);
}

bool XBOXGamepadExtension::ButtonBPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 1);
}

bool XBOXGamepadExtension::ButtonXPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 2);
}

bool XBOXGamepadExtension::ButtonYPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 3);
}

bool XBOXGamepadExtension::ButtonLeftShoulderPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 4);
}

bool XBOXGamepadExtension::ButtonRightShoulderPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 5);
}

bool XBOXGamepadExtension::ButtonBackPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 6);
}

bool XBOXGamepadExtension::ButtonStartPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 7);
}

bool XBOXGamepadExtension::ButtonLeftStickPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 8);
}

bool XBOXGamepadExtension::ButtonRightStickPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 9);
}

bool XBOXGamepadExtension::DPadUpPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 10);
}

bool XBOXGamepadExtension::DPadDownPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 11);
}

bool XBOXGamepadExtension::DPadLeftPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 12);
}

bool XBOXGamepadExtension::DPadRightPressed(int gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 13);
}

bool XBOXGamepadExtension::AnyButtonPressed(int gamepadBitmask)
{
    for (int i = 0; i < 14; i++)
    {
        if (ButtonPressed(gamepadBitmask, i)) return true;
    }
    return false;
}

int XBOXGamepadExtension::StickLeftH(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex-1), SDL_GAMEPAD_AXIS_LEFTX);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return axis;
}

int XBOXGamepadExtension::StickLeftV(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex-1), SDL_GAMEPAD_AXIS_LEFTY);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return axis;
}

int XBOXGamepadExtension::StickRightH(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex-1), SDL_GAMEPAD_AXIS_RIGHTX);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return axis;
}

int XBOXGamepadExtension::StickRightV(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex-1), SDL_GAMEPAD_AXIS_RIGHTY);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return axis;
}

int XBOXGamepadExtension::TriggerLeft(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex-1), SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < triggerDeadzone) return 0;
    return axis;
}

int XBOXGamepadExtension::TriggerRight(int gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex-1), SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < triggerDeadzone) return 0;
    return axis;
}

void XBOXGamepadExtension::Vibrate(int gamepadBitmask, int leftMotor, int rightMotor, unsigned int duration)
{
    unsigned short left = (unsigned short)((leftMotor / 100.0f) * 65535.0f);
    unsigned short right = (unsigned short)((rightMotor / 100.0f) * 65535.0f);

    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    for (int i = 0; i < 4; i++)
    {
        if (gamepadBitmask & (1 << i))
        {
            if (!input->IsGamepadConnected(i)) continue;
            SDL_Gamepad* gamepad = input->gamepads.at(i);
            SDL_RumbleGamepad(gamepad, left, right, duration);
        }
    }
}

#endif