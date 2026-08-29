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

	bool ButtonPressed(CValue gamepadBitmask, CValue button);
	CValue ButtonPressedExpression(CValue gamepadBitmask, CValue button);
	CValue ButtonPressedSpecificExpression(CValue button, CValue gamepadBitmask);

	CValue FindPressedButton(CValue gamepadBitmask);

	bool GamepadIsConnected(CValue gamepadIndex);

	bool ButtonAPressed(CValue gamepadBitmask);
	bool ButtonBPressed(CValue gamepadBitmask);
	bool ButtonXPressed(CValue gamepadBitmask);
	bool ButtonYPressed(CValue gamepadBitmask);
	bool ButtonLeftShoulderPressed(CValue gamepadBitmask);
	bool ButtonRightShoulderPressed(CValue gamepadBitmask);
	bool ButtonBackPressed(CValue gamepadBitmask);
	bool ButtonStartPressed(CValue gamepadBitmask);
	bool ButtonLeftStickPressed(CValue gamepadBitmask);
	bool ButtonRightStickPressed(CValue gamepadBitmask);
	bool DPadUpPressed(CValue gamepadBitmask);
	bool DPadDownPressed(CValue gamepadBitmask);
	bool DPadLeftPressed(CValue gamepadBitmask);
	bool DPadRightPressed(CValue gamepadBitmask);

	bool AnyButtonPressed(CValue gamepadBitmask);

	CValue StickLeftH(CValue gamepadIndex);
	CValue StickLeftV(CValue gamepadIndex);
	CValue StickRightH(CValue gamepadIndex);
	CValue StickRightV(CValue gamepadIndex);
	CValue TriggerLeft(CValue gamepadIndex);
	CValue TriggerRight(CValue gamepadIndex);

	void Vibrate(CValue gamepadBitmask, CValue leftMotor, CValue rightMotor, CValue duration);
};