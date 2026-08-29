#pragma once

#include "Extension.h"
#include "ObjectInstance.h"
	
class CrasherExtension : public Extension {
public:
	CrasherExtension(unsigned int objectInfoHandle, int type, std::string name)
		: Extension(objectInfoHandle, type, name) {}
};