#include "Input.h"

#include <cstring>

#include "Application.h"
#include "InputBackend.h"

void Input::Update()
{
	Application::Instance().GetBackend()->input->Update();
	
	m_currIndex ^= 1;
	Application::Instance().GetBackend()->input->GetKeyboardState(m_keyboardState[m_currIndex]);

	previousMouseState = currentMouseState;
	currentMouseState = Application::Instance().GetBackend()->input->GetMouseState();

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

	for (int i = 0; i < 4; i++)
	{
		m_gamepadState[0][i] = 0;
		m_gamepadState[1][i] = 0;
	}

	m_playerEnabled = {true, true, true, true};
}

bool Input::IsKeyDown(short key)
{
#if defined(__wiiu__) || defined(__WIIU__)
	uint8_t gp = m_gamepadState[m_currIndex][0];

	switch (key) {
		case 0x57: // W
		case 0x26: // Up arrow
			if (gp & (1 << 0)) return true;
			break;

		case 0x53: // S
		case 0x28: // Down arrow
			if (gp & (1 << 1)) return true;
			break;

		case 0x41: // A
		case 0x25: // Left arrow
			if (gp & (1 << 2)) return true;
			break;

		case 0x44: // D
		case 0x27: // Right arrow
			if (gp & (1 << 3)) return true;
			break;

		case 0x58: // X / Enter -> A
		case 0x0D:
			if (gp & (1 << 4)) return true;
			break;

		case 0x5A: // Z -> B
			if (gp & (1 << 5)) return true;
			break;

		case 0x09: // Tab -> X
			if (gp & (1 << 6)) return true;
			break;

		case 0x52: // R -> Y
			if (gp & (1 << 7)) return true;
			break;
	}
#endif

	return m_keyboardState[m_currIndex][key] == 1;
}

bool Input::IsKeyPressed(short key)
{
#if defined(__wiiu__) || defined(__WIIU__)
	auto IsKeyDownAt = [&](int index) -> bool {
		uint8_t gp = m_gamepadState[index][0];

		switch (key) {
			case 0x57:
			case 0x26: return (gp & (1 << 0)) != 0;
			case 0x53:
			case 0x28: return (gp & (1 << 1)) != 0;
			case 0x41:
			case 0x25: return (gp & (1 << 2)) != 0;
			case 0x44:
			case 0x27: return (gp & (1 << 3)) != 0;
			case 0x58:
			case 0x0D: return (gp & (1 << 4)) != 0;
			case 0x5A: return (gp & (1 << 5)) != 0;
			case 0x09: return (gp & (1 << 6)) != 0;
			case 0x52: return (gp & (1 << 7)) != 0;
		}

		return m_keyboardState[index][key] == 1;
	};

	return IsKeyDownAt(m_currIndex) && !IsKeyDownAt(m_currIndex ^ 1);
#else
	return m_keyboardState[m_currIndex][key] == 1 && m_keyboardState[m_currIndex ^ 1][key] == 0;
#endif
}

bool Input::IsKeyReleased(short key)
{
#if defined(__wiiu__) || defined(__WIIU__)
	auto IsKeyDownAt = [&](int index) -> bool {
		uint8_t gp = m_gamepadState[index][0];

		switch (key) {
			case 0x57:
			case 0x26: return (gp & (1 << 0)) != 0;
			case 0x53:
			case 0x28: return (gp & (1 << 1)) != 0;
			case 0x41:
			case 0x25: return (gp & (1 << 2)) != 0;
			case 0x44:
			case 0x27: return (gp & (1 << 3)) != 0;
			case 0x58:
			case 0x0D: return (gp & (1 << 4)) != 0;
			case 0x5A: return (gp & (1 << 5)) != 0;
			case 0x09: return (gp & (1 << 6)) != 0;
			case 0x52: return (gp & (1 << 7)) != 0;
		}

		return m_keyboardState[index][key] == 1;
	};

	return !IsKeyDownAt(m_currIndex) && IsKeyDownAt(m_currIndex ^ 1);
#else
	return m_keyboardState[m_currIndex][key] == 0 && m_keyboardState[m_currIndex ^ 1][key] == 1;
#endif
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

int Input::GetControlType(int player)
{
	return Application::Instance().GetAppData()->GetControlTypes()[player];
}

void Input::SetControlType(int player, int type)
{
	if (type >= 1 && type <= 4) //is gamepad
	{
		if (!Application::Instance().GetBackend()->input->IsGamepadConnected(type-1))
			type = 0; //reset back to keyboard
	}
	
	Application::Instance().GetAppData()->GetControlTypes()[player] = type;
}

void Input::SetControlKey(int player, short control, unsigned short key)
{
	Application::Instance().GetAppData()->GetControlKeys()[player][control] = key;
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
	if (doubleClick) return false; // TODO: implement double click
	
	if (button == 1) button = 0;
	else if (button == 4) button = 1;
	return (currentMouseState & (1 << button)) && !(previousMouseState & (1 << button));
}