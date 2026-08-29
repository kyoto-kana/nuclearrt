#pragma once

#include "CValue.h"

#include <cstdint>
#include <vector>

class Input
{
public:
	Input() = default;
	~Input() = default;

	void Update();
	void Reset();

	bool IsKeyDown(short key);
	bool IsKeyPressed(short key);
	bool IsKeyReleased(short key);
	bool IsAnyKeyPressed();
	bool IsAnyMouseButtonPressed();

	int GetControlType(int player);
	void SetControlType(int player, const CValue& type);

	void SetControlKey(int player, const CValue& control, unsigned short key);

	void RestoreControl(int player);
	void IgnoreControl(int player);

	bool IsControlsDown(int player, short control);
	bool IsControlsPressed(int player, short control);

	int GetMouseX();
	int GetMouseY();
	int GetMouseWheelMove();
	bool IsMouseButtonDown(int button);
	bool IsMouseButtonPressed(int button, bool doubleClick = false);

private:
	uint8_t m_keyboardState[2][256];
	int m_currIndex = 0;

	uint32_t currentMouseState;
	uint32_t previousMouseState;

	int lastClickMouseX = 0;
	int lastClickMouseY = 0;
	int lastClickButton = -1;
	int doubleClickButton = -1;
	unsigned int lastClickTicks = 0;

	static constexpr unsigned int DOUBLE_CLICK_MS = 500;

	uint8_t m_gamepadState[2][4];

	std::vector<bool> m_playerEnabled;
};

