#pragma once

#include <vector>
#include <memory>

#include "CValue.h"
#include "CounterBase.h"

class Lives : public CounterBase {
public:
	Lives(unsigned int objectInfoHandle, int type, std::string name)
		: CounterBase(objectInfoHandle, type, name) {}

	CValue GetValue() const override { return CValue(Application::Instance().GetAppData()->GetPlayerLives()[Player]); }
};