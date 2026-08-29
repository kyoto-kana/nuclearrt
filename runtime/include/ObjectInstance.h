#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "CValue.h"
#include "CollisionInstanceBounds.h"
#include "ObjectGlobalData.h"
#include "EffectInstance.h"

class ObjectInstance {
public:
    ObjectInstance(unsigned int objectInfoHandle, int type, std::string name)
        : ObjectInfoHandle(objectInfoHandle), Type(type), Name(name) {}
    virtual ~ObjectInstance() = default;
    
    std::string Name = "";
    std::vector<short> Qualifiers = {};
    
    unsigned int Handle = 0;
    unsigned int Type = 0;
    unsigned int ObjectInfoHandle = 0;
	unsigned int Layer = 0;
    unsigned int FixedValue = 0;
private:
    unsigned int Angle = 0;
    int x = 0;
    int y = 0;
public:

    CValue GetX() const {
        return CValue(x);
    }

    CValue GetY() const {
        return CValue(y);
    }

    void SetX(const CValue& x) {
        if (x.GetIntValue() == this->x) return;
        this->x = x.GetIntValue();
        collisionBoundsDirty = true;
    }
    void SetY(const CValue& y) {
        if (y.GetIntValue() == this->y) return;
        this->y = y.GetIntValue();
        collisionBoundsDirty = true;
    }

    void SetPosition(const CValue& x, const CValue& y) {
        if (x.GetIntValue() == this->x && y.GetIntValue() == this->y) return;
        this->x = x.GetIntValue();
        this->y = y.GetIntValue();
        collisionBoundsDirty = true;
    }

    int RGBCoefficient = 0xFFFFFFFF;
    int Effect = 0;
    EffectInstance* effectInstance = nullptr;

    short InstanceValue = 0;
    
    bool global = false;
	bool isSelected = false;
    bool FollowFrame = false;

    bool collisionBoundsDirty = true;
    CollisionInstanceBounds collisionBounds = {0};
public:

    CValue GetEffectInstanceParameter(const CValue& name) const {
        if (effectInstance == nullptr)
            return CValue(0);
        return effectInstance->GetParameter(name);
    }

    void SetEffectInstanceParameter(const CValue& name, const CValue& value) {
        if (effectInstance == nullptr)
            return;
        effectInstance->SetParameter(name.GetStringValue(), value);
    }

    CValue GetBlendCoefficient() const {
        int effectParameter = (RGBCoefficient >> 24) & 0xFF;
        return CValue(effectParameter);
    }

    void SetBlendCoefficient(const CValue& blendCoefficient) {
        RGBCoefficient = (RGBCoefficient & 0x00FFFFFF) | (std::clamp(blendCoefficient.GetIntValue(), 0, 255) << 24);
    }
    
    unsigned int GetAngle() const {
        return Angle;
    }

    void SetAngle(const CValue& angle) {
        int angleValue = angle.GetIntValue();
        angleValue %= 360;
        if (angleValue < 0) angleValue += 360;
        if (angleValue == Angle) return;
        Angle = static_cast<unsigned int>(angleValue);
        collisionBoundsDirty = true;
    }

    virtual ObjectGlobalData* CreateGlobalData() { return nullptr; };
    virtual void ApplyGlobalData(ObjectGlobalData* globalData) { };

    virtual std::vector<unsigned int> GetImagesUsed() { return std::vector<unsigned int>(); };
    virtual std::vector<unsigned int> GetFontsUsed() { return std::vector<unsigned int>(); };

	bool HasQualifier(short qualifier) const {
		short groupIndex = qualifier & 0x7FFF;
		for (short stored : Qualifiers) {
			if ((stored & 0x7FFF) == groupIndex) return true;
		}
		return false;
	}

};