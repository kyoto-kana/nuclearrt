#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"
#include <string>

struct EaseVars
{
	float overshoot;
	float amplitude;
	float period;
};

struct MoveStruct
{
	int fixedValue;
	int startX;
	int startY;
	int destX;
	int destY;

	EaseVars vars;

	unsigned char easingMode;
	unsigned char functionA;
	unsigned char functionB;

	unsigned char timeMode;
	long starttime;
	long pausetime;
	int timespan;
	int eventloop_step;
	int pausecount;
};

class EasingObjectExtension : public Extension
{
public:
	EasingObjectExtension(unsigned int objectInfoHandle, int type, std::string name, float overshoot, float amplitude, float period)
		: Extension(objectInfoHandle, type, name), Overshoot(overshoot), Amplitude(amplitude), Period(period) {}

	void Update(float deltaTime) override;

	void MoveObject(ObjectInstance* instance, uint8_t easingMethod, uint8_t firstFunction, uint8_t secondFunction, CValue x, CValue y, uint8_t timeMode, CValue time);
	void MoveObjectExplicit(CValue fixedValue, CValue easingMethod, CValue firstFunction, CValue secondFunction, CValue x, CValue y, CValue timeMode, CValue time);

	void SetOvershoot(CValue overshoot) { Overshoot = (float)overshoot.GetDoubleValue(); }
	void SetAmplitude(CValue amplitude) { Amplitude = (float)amplitude.GetDoubleValue(); }
	void SetPeriod(CValue period) { Period = (float)period.GetDoubleValue(); }

	double CalculateEasingValue(int mode, int functionA, int functionB, double step, EaseVars vars);

private:
	float Overshoot = 1.5f;
	float Amplitude = 1.0f;
	float Period = 0.4f;

	std::vector<MoveStruct> controlled = {};
};