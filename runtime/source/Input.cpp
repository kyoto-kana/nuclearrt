#include "Input.h"

#include "Application.h"
#include "InputBackend.h"
#include "PlatformBackend.h"

void Input::Update()
{
	Application::Instance().GetBackend()->input->Update();
	
	m_currIndex ^= 1;
	Application::Instance().GetBackend()->input->GetKeyboardState(m_keyboardState[m_currIndex]);

	previousMouseState = currentMouseState;
	currentMouseState = Application::Instance().GetBackend()->input->GetMouseState();

	doubleClickButton = -1;

	if (currentMouseState & ~previousMouseState)
	{
		int button = -1;
		for (int i = 0; i < 3; i++)
		{
			if ((currentMouseState & ~previousMouseState) & (1 << i))
			{
				button = i;
				break;
			}
		}

		if (button != -1)
		{
			int mouseX = GetMouseX();
			int mouseY = GetMouseY();
			
			unsigned int now = Application::Instance().GetBackend()->platform->GetTicks();

			bool isDoubleClick = (button == lastClickButton)
				&& (now - lastClickTicks) < DOUBLE_CLICK_MS
				&& mouseX == lastClickMouseX
				&& mouseY == lastClickMouseY;

			if (isDoubleClick)
			{
				doubleClickButton = button;
				lastClickButton = -1;
			}
			else
			{
				lastClickButton = button;
				lastClickTicks = now;
			}

			lastClickMouseX = mouseX;
			lastClickMouseY = mouseY;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		m_gamepadState[m_currIndex][i] = Application::Instance().GetBackend()->input->GetGamepadButtonState(i);
	}
}

void Input::Reset()
{
	memset((void*)m_keyboardState, 0, sizeof(m_keyboardState));

	previousMouseState = 0;
	currentMouseState = 0;
	lastClickMouseX = 0;
	lastClickMouseY = 0;
	lastClickButton = -1;
	doubleClickButton = -1;
	lastClickTicks = 0;

	for (int i = 0; i < 4; i++)
	{
		m_gamepadState[0][i] = 0;
		m_gamepadState[1][i] = 0;
	}

	m_playerEnabled = {true, true, true, true};
}

bool Input::IsKeyDown(short key)
{
	return m_keyboardState[m_currIndex][key] == 1;
}

bool Input::IsKeyPressed(short key)
{
	return m_keyboardState[m_currIndex][key] == 1 && m_keyboardState[m_currIndex ^ 1][key] == 0;
}

bool Input::IsKeyReleased(short key)
{
	return m_keyboardState[m_currIndex][key] == 0 && m_keyboardState[m_currIndex ^ 1][key] == 1;
}

bool Input::IsAnyKeyPressed()
{
    for (int i = 0; i < 256; i++)
    {
        if (IsKeyPressed(i))
            return true;
    }
    return false;
}

bool Input::IsAnyMouseButtonPressed()
{
	return (currentMouseState & ~previousMouseState) != 0;
}

int Input::GetControlType(int player)
{
	return Application::Instance().GetAppData()->GetControlTypes()[player];
}

void Input::SetControlType(int player, const CValue& type)
{
	int controlType = type.GetIntValue();
	if (controlType >= 1 && controlType <= 4) //is gamepad
	{
		if (!Application::Instance().GetBackend()->input->IsGamepadConnected(controlType - 1))
			controlType = 0; //reset back to keyboard
	}
	
	Application::Instance().GetAppData()->GetControlTypes()[player] = controlType;
}

void Input::SetControlKey(int player, const CValue& control, unsigned short key)
{
	Application::Instance().GetAppData()->GetControlKeys()[player][control.GetIntValue()] = key;
}

void Input::RestoreControl(int player)
{
	m_playerEnabled[player] = true;
}

void Input::IgnoreControl(int player)
{
	m_playerEnabled[player] = false;
}

bool Input::IsControlsDown(int player, short control)
{
	if (!m_playerEnabled[player]) return false;

	int controlType = Application::Instance().GetAppData()->GetControlTypes()[player];
	if (controlType != 0)
	{
		if (!Application::Instance().GetBackend()->input->IsGamepadConnected(controlType-1))
			return false;

		for (int i = 0; i < 8; i++)
		{
			if ((control & (1 << i)) != 0 && (m_gamepadState[m_currIndex][player] & (1 << i)) == 0)
				return false;
		}
		return true;
	}

	for (int i = 0; i < 8; i++)
	{
		if ((control & (1 << i)) != 0 && !IsKeyDown(Application::Instance().GetAppData()->GetControlKeys()[player][i])) return false;
	}

	return true;
}

bool Input::IsControlsPressed(int player, short control)
{
	if (!m_playerEnabled[player]) return false;
	
	int controlType = Application::Instance().GetAppData()->GetControlTypes()[player];
	if (controlType != 0)
	{
		if (!Application::Instance().GetBackend()->input->IsGamepadConnected(controlType-1))
			return false;

		for (int i = 0; i < 8; i++)
		{
			if ((control & (1 << i)) == 0)
				continue;
			if ((m_gamepadState[m_currIndex][player] & (1 << i)) != 0 && (m_gamepadState[m_currIndex ^ 1][player] & (1 << i)) == 0)
				return true;
		}
		return false;
	}

	for (int i = 0; i < 8; i++)
	{
		if ((control & (1 << i)) != 0 && IsKeyPressed(Application::Instance().GetAppData()->GetControlKeys()[player][i])) return true;
	}

	return false;
}

int Input::GetMouseX()
{
	return Application::Instance().GetBackend()->input->GetMouseX();
}

int Input::GetMouseY()
{
	return Application::Instance().GetBackend()->input->GetMouseY();
}

int Input::GetMouseWheelMove()
{
	return Application::Instance().GetBackend()->input->GetMouseWheelMove();
}

bool Input::IsMouseButtonDown(int button)
{
	if (button == 1) button = 0;
	else if (button == 4) button = 1;
	return currentMouseState & (1 << button);
}

bool Input::IsMouseButtonPressed(int button, bool doubleClick)
{
	if (button == 1) button = 0;
	else if (button == 4) button = 1;

	bool isPressed = (currentMouseState & (1u << button)) && !(previousMouseState & (1u << button));
	if (!isPressed)
		return false;

	if (doubleClick)
		return button == doubleClickButton;

	return button != doubleClickButton;
}