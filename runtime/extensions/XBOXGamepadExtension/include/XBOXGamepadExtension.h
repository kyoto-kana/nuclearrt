#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"
#include <string>
	
class XBOXGamepadExtension : public Extension {
public:
	XBOXGamepadExtension(unsigned int objectInfoHandle, int type, std::string name)
		: Extension(objectInfoHandle, type, name) {}

	const int stickDeadzone = 24;
	const int triggerDeadzone = 12;

	int ConvertToPlatformButton(int button);

	bool ButtonPressed(int gamepadBitmask, int button);
	int ButtonPressedExpression(int gamepadBitmask, int button);
	int ButtonPressedSpecificExpression(int button, int gamepadBitmask);

	int FindPressedButton(int gamepadBitmask);

	int GamepadIsConnected(int gamepadIndex);

	bool ButtonAPressed(int gamepadBitmask);
	bool ButtonBPressed(int gamepadBitmask);
	bool ButtonXPressed(int gamepadBitmask);
	bool ButtonYPressed(int gamepadBitmask);
	bool ButtonLeftShoulderPressed(int gamepadBitmask);
	bool ButtonRightShoulderPressed(int gamepadBitmask);
	bool ButtonBackPressed(int gamepadBitmask);
	bool ButtonStartPressed(int gamepadBitmask);
	bool ButtonLeftStickPressed(int gamepadBitmask);
	bool ButtonRightStickPressed(int gamepadBitmask);
	bool DPadUpPressed(int gamepadBitmask);
	bool DPadDownPressed(int gamepadBitmask);
	bool DPadLeftPressed(int gamepadBitmask);
	bool DPadRightPressed(int gamepadBitmask);

	bool AnyButtonPressed(int gamepadBitmask);

	int StickLeftH(int gamepadIndex);
	int StickLeftV(int gamepadIndex);
	int StickRightH(int gamepadIndex);
	int StickRightV(int gamepadIndex);
	int TriggerLeft(int gamepadIndex);
	int TriggerRight(int gamepadIndex);

	void Vibrate(int gamepadBitmask, int leftMotor, int rightMotor, unsigned int duration);
};