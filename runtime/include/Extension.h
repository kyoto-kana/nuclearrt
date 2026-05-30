#pragma once

#include "ObjectInstance.h"
#include <vector>
#include <memory>

#include "AlterableValues.h"
#include "AlterableStrings.h"
#include "AlterableFlags.h"

class Extension : public ObjectInstance {
public:
	Extension(unsigned int objectInfoHandle, int type, std::string name)
		: ObjectInstance(objectInfoHandle, type, name) {}
	virtual ~Extension() = default;

	virtual void Initialize() {}
	virtual void Update(float deltaTime) {}
	virtual void Draw() {}

	AlterableValues Values;
	AlterableStrings Strings;
	AlterableFlags Flags;

	bool Visible = true;

	ObjectGlobalData* CreateGlobalData() override {
		ObjectGlobalData* globalData = new ObjectGlobalData(ObjectInfoHandle);

		globalData->flags = Flags;
		globalData->values = Values;
		globalData->strings = Strings;

		return globalData;
	}

	void ApplyGlobalData(ObjectGlobalData* globalData) override {
		Flags = globalData->flags;
		Values = globalData->values;
		Strings = globalData->strings;
	}
};