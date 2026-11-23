#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectSelector.h"
#include "Frame.h"
#include <string>
#include <vector>

class SelectObjectExtension : public Extension
{
public:
	SelectObjectExtension(unsigned int objectInfoHandle, int type, std::string name)
		: Extension(objectInfoHandle, type, name) {}
    //E
	CValue GetSelectedByName(Frame* frame, const CValue& name);
	CValue GetSelectedByFixed(Frame* frame, const CValue& fixed);
	CValue GetNameByFixed(Frame* frame, const CValue& fixed);
private:
};