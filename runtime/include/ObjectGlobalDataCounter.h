#pragma once

#include "ObjectGlobalData.h"

class ObjectGlobalDataCounter : public ObjectGlobalData {
public:
    ObjectGlobalDataCounter(unsigned int objectInfoHandle) : ObjectGlobalData(objectInfoHandle) {}

    CValue value = 0;
    CValue minValue = 0;
    CValue maxValue = 0;
};