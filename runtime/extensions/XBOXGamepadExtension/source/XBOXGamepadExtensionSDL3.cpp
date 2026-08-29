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

bool XBOXGamepadExtension::ButtonPressed(CValue gamepadBitmask, CValue button)
{
    if (button.GetIntValue() < 0 || button.GetIntValue() > 13) return false;
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
	for (int i = 0; i < 4; i++)
	{
		if (gamepadBitmask.GetIntValue() & (1 << i))
		{
            if (!input->IsGamepadConnected(i)) return false;
            SDL_Gamepad* gamepad = input->gamepads.at(i);
			if (SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)ConvertToPlatformButton(button.GetIntValue()))) return true;
		}
	}
    return false;
}

CValue XBOXGamepadExtension::ButtonPressedExpression(CValue gamepadBitmask, CValue button)
{
    //This is replicating a bug in the orignal extenstion where left and right stick return the value of the shoulder button
    if (button.GetIntValue() == 8 || button.GetIntValue() == 9) return ButtonPressed(gamepadBitmask, button.GetIntValue() - 4);
    
    return ButtonPressed(gamepadBitmask, button.GetIntValue()) ? CValue(1) : CValue(0);
}

CValue XBOXGamepadExtension::ButtonPressedSpecificExpression(CValue button, CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, button.GetIntValue()) ? CValue(1) : CValue(0);
}

CValue XBOXGamepadExtension::FindPressedButton(CValue gamepadBitmask)
{
    for (int i = 0; i < 14; i++)
    {
        if (ButtonPressed(gamepadBitmask, i)) return CValue(i);
    }
    return CValue(-1);
}

bool XBOXGamepadExtension::GamepadIsConnected(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    return input->IsGamepadConnected(gamepadIndex.GetIntValue());
}

bool XBOXGamepadExtension::ButtonAPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 0);
}

bool XBOXGamepadExtension::ButtonBPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 1);
}

bool XBOXGamepadExtension::ButtonXPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 2);
}

bool XBOXGamepadExtension::ButtonYPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 3);
}

bool XBOXGamepadExtension::ButtonLeftShoulderPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 4);
}

bool XBOXGamepadExtension::ButtonRightShoulderPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 5);
}

bool XBOXGamepadExtension::ButtonBackPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 6);
}

bool XBOXGamepadExtension::ButtonStartPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 7);
}

bool XBOXGamepadExtension::ButtonLeftStickPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 8);
}

bool XBOXGamepadExtension::ButtonRightStickPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 9);
}

bool XBOXGamepadExtension::DPadUpPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 10);
}

bool XBOXGamepadExtension::DPadDownPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 11);
}

bool XBOXGamepadExtension::DPadLeftPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 12);
}

bool XBOXGamepadExtension::DPadRightPressed(CValue gamepadBitmask)
{
    return ButtonPressed(gamepadBitmask, 13);
}

bool XBOXGamepadExtension::AnyButtonPressed(CValue gamepadBitmask)
{
    for (int i = 0; i < 14; i++)
    {
        if (ButtonPressed(gamepadBitmask, i)) return true;
    }
    return false;
}

CValue XBOXGamepadExtension::StickLeftH(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex.GetIntValue()-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex.GetIntValue()-1), SDL_GAMEPAD_AXIS_LEFTX);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return CValue(axis);
}

CValue XBOXGamepadExtension::StickLeftV(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex.GetIntValue()-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex.GetIntValue()-1), SDL_GAMEPAD_AXIS_LEFTY);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return CValue(axis);
}

CValue XBOXGamepadExtension::StickRightH(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex.GetIntValue()-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex.GetIntValue()-1), SDL_GAMEPAD_AXIS_RIGHTX);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return CValue(axis);
}

CValue XBOXGamepadExtension::StickRightV(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex.GetIntValue()-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex.GetIntValue()-1), SDL_GAMEPAD_AXIS_RIGHTY);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < stickDeadzone) return 0;
    return CValue(axis);
}

CValue XBOXGamepadExtension::TriggerLeft(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex.GetIntValue()-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex.GetIntValue()-1), SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < triggerDeadzone) return 0;
    return CValue(axis);
}

CValue XBOXGamepadExtension::TriggerRight(CValue gamepadIndex)
{
    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    if (!input->IsGamepadConnected(gamepadIndex.GetIntValue()-1)) return 0;
    int axis = SDL_GetGamepadAxis(input->gamepads.at(gamepadIndex.GetIntValue()-1), SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
    axis = (axis / 32767.0f) * 100.0f;
    if (abs(axis) < triggerDeadzone) return 0;
    return CValue(axis);
}

void XBOXGamepadExtension::Vibrate(CValue gamepadBitmask, CValue leftMotor, CValue rightMotor, CValue duration)
{
    unsigned short left = (unsigned short)((leftMotor.GetIntValue() / 100.0f) * 65535.0f);
    unsigned short right = (unsigned short)((rightMotor.GetIntValue() / 100.0f) * 65535.0f);

    auto* input = dynamic_cast<SDL3InputBackend*>(Application::Instance().GetBackend()->input);
    for (int i = 0; i < 4; i++)
    {
        if (gamepadBitmask.GetIntValue() & (1 << i))
        {
            if (!input->IsGamepadConnected(i)) continue;
            SDL_Gamepad* gamepad = input->gamepads.at(i);
            SDL_RumbleGamepad(gamepad, left, right, duration.GetIntValue());
        }
    }
}

#endif