#include "SelectObjectExtension.h"
#include "Active.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

#pragma region Expressions

CValue SelectObjectExtension::GetSelectedByName(Frame* frame, const CValue& name)
{
    if (!frame) return CValue(0);

    const std::string& namev = name.GetStringValue();
    unsigned int objectInfoHandle = 0;
    bool found = false;

    for (auto& [handle, instance] : frame->ObjectInstances) {
        if (Frame::LoopNameEquals(instance->Name, namev)) {
            objectInfoHandle = instance->ObjectInfoHandle;
            found = true;
            break;
        }
    }
    if (!found) return CValue(0);

    int total = 0;
    int selected = 0;
    for (auto& [handle, instance] : frame->ObjectInstances) {
        if (instance->ObjectInfoHandle == objectInfoHandle) {
            total++;
            if (instance->isSelected) selected++;
        }
    }
    return CValue(selected > 0 ? selected : total);
}

CValue SelectObjectExtension::GetSelectedByFixed(Frame* frame, const CValue& fixed)
{
    if (!frame) return CValue(0);

    ObjectInstance* targetInstance = frame->GetInstanceByFixedValue(static_cast<unsigned int>(fixed.GetIntValue()));
    if (!targetInstance) return CValue(0);

    unsigned int objectInfoHandle = targetInstance->ObjectInfoHandle;
    int total = 0;
    int selected = 0;
    for (auto& [handle, instance] : frame->ObjectInstances) {
        if (instance->ObjectInfoHandle == objectInfoHandle) {
            total++;
            if (instance->isSelected) selected++;
        }
    }
    return CValue(selected > 0 ? selected : total);
}

CValue SelectObjectExtension::GetNameByFixed(Frame* frame, const CValue& fixed)
{
    if (!frame) return CValue("");

    ObjectInstance* instance = frame->GetInstanceByFixedValue(static_cast<unsigned int>(fixed.GetIntValue()));
    return instance ? CValue(instance->Name) : CValue("");
}

#pragma endregion Expressions