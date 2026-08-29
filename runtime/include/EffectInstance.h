#pragma once

#include "CValue.h"

#include <variant>
#include <string>
#include <vector>

struct EffectParameter {
    std::string Name;
    int Type;
    std::variant<int, float> Value;

    EffectParameter(std::string name, int type, std::variant<int, float> value)
        : Name(name), Type(type), Value(value) {}
};

class EffectInstance {
public:
    EffectInstance(std::string filename, std::vector<EffectParameter> parameters, bool usesBackground = false)
        : filename(filename), Parameters(parameters), UsesBackground(usesBackground) {}

    std::string filename;
    std::vector<EffectParameter> Parameters;
    bool UsesBackground = false;

    void SetParameter(std::string name, const CValue& value)
    {
        for (auto& parameter : Parameters) {
            if (parameter.Name == name) {
                if (parameter.Type == 1)
                {
                    parameter.Value = (float)value.GetDoubleValue();
                }
                else
                {
                    parameter.Value = value.GetIntValue();
                }

                return;
            }
        }
    }

    CValue GetParameter(const CValue& name)
    {
        for (auto& parameter : Parameters) {
            if (parameter.Name == name.GetStringValue()) {
                if (parameter.Type == 1)
                {
                    return CValue(std::get<float>(parameter.Value));
                }
                else
                {
                    return CValue(std::get<int>(parameter.Value));
                }
            }
        }
        return CValue(0);
    }
};