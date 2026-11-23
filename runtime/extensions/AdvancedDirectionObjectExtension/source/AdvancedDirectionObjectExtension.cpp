#include "AdvancedDirectionObjectExtension.h"
#include "Active.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

#pragma region Actions

void AdvancedDirectionObjectExtension::GetObjects(ObjectInstance* instance, ObjectSelector* origin, int offsetX, int offsetY, bool actionPoint)
{
    int x = offsetX;
    int y = offsetY;
    if (origin != nullptr)
    {
        x = 0;
        y = 0;
        if (origin->Count() > 0)
        {
            ObjectInstance* parent = *origin->begin();
            if (actionPoint && parent->Type == 2)
            {
                x = ((Active*)parent)->GetXActionPoint().GetIntValue();
                y = ((Active*)parent)->GetYActionPoint().GetIntValue();
            }
            else
            {
                x = parent->GetX().GetIntValue();
                y = parent->GetY().GetIntValue();
            }
            x += offsetX;
            y += offsetY;
        }
    }
    
    //resetting if another event
    if (EventCount != FrameCounter)
    {
        CurrentObject = 0;
        EventCount = FrameCounter;
    }

    int x1 = x;
    int y1 = y;
    LastX = x;
    LastY = y;
    int x2 = instance->GetX().GetIntValue();
    int y2 = instance->GetY().GetIntValue();

    while (CurrentObject >= (int)Distance.size())
    {
        Distance.push_back(0.0f);
        Fixed.push_back(0);
    }

    Distance[CurrentObject] = (float)std::sqrt((double)((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
    Fixed[CurrentObject] = (int)instance->FixedValue;

    CurrentObject++;
}

void AdvancedDirectionObjectExtension::AddObjects(ObjectInstance* instance)
{
    int x1 = LastX;
    int y1 = LastY;
    int x2 = instance->GetX().GetIntValue();
    int y2 = instance->GetY().GetIntValue();
    
    while (CurrentObject >= (int)Distance.size())
    {
        Distance.push_back(0.0f);
        Fixed.push_back(0);
    }
    
    Distance[CurrentObject] = (float)std::sqrt((double)((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
    Fixed[CurrentObject] = (int)instance->FixedValue;

    CurrentObject++;
}

#pragma endregion Actions

#pragma region Conditions

bool AdvancedDirectionObjectExtension::CompDist(CValue x1, CValue y1, CValue x2, CValue y2, CValue v)
{
    int x1v = x1.GetIntValue();
    int y1v = y1.GetIntValue();
    int x2v = x2.GetIntValue();
    int y2v = y2.GetIntValue();
    int vv  = v.GetIntValue();

    if ((int)std::sqrt((double)((x1v - x2v) * (x1v - x2v) + (y1v - y2v) * (y1v - y2v))) <= vv)
    {
        return true;
    }
    return false;
}

static constexpr double PID = 3.14159265358979323846;

int AdvancedDirectionObjectExtension::lMin(int v1, int v2, int v3)
{
    return std::min({v1, v2, v3});
}

int AdvancedDirectionObjectExtension::lSMin(int v1, int v2, int v3) {
    if (std::abs(v1) <= std::abs(v2) && std::abs(v1) <= std::abs(v3)) {
        return v1;
    }
    if (std::abs(v2) <= std::abs(v1) && std::abs(v2) <= std::abs(v3)) {
        return v2;
    }
    return v3;
}

bool AdvancedDirectionObjectExtension::CompDir(CValue x1, CValue y1, CValue x2, CValue y2, CValue dir, CValue offset)
{
    int x1v = x1.GetIntValue();
    int y1v = y1.GetIntValue();
    int x2v = x2.GetIntValue();
    int y2v = y2.GetIntValue();
    int dirv  = dir.GetIntValue();
    int offsetv  = offset.GetIntValue();

    while (dirv >= NumDir)
    {
        dirv -= NumDir;
    }
    while (dirv < 0)
    {
        dirv += NumDir;
    }

    int dir2 = (int)(((((std::atan2(y2v - y1v, x2v - x1v) * 180) / PID) * -1) / 360) * NumDir);

    while (dir2 >= NumDir)
    {
        dir2 -= NumDir;
    }
    while (dir2 < 0)
    {
        dir2 += NumDir;
    }

    if (lMin(std::abs(dirv - dir2), std::abs(dirv - dir2 - NumDir), std::abs(dirv - dir2 + NumDir)) <
            offsetv)
    {
        return true;
    }
    return false;
}

#pragma endregion Conditions

#pragma region Expressions

CValue AdvancedDirectionObjectExtension::Direction(CValue x1, CValue y1, CValue x2, CValue y2)
{
    int x1v = x1.GetIntValue();
    int y1v = y1.GetIntValue();
    int x2v = x2.GetIntValue();
    int y2v = y2.GetIntValue();

    //Just doing simple math now.

    float r = (float)(((((std::atan2(y2v - y1v, x2v - x1v) * 180) / PID) * -1) / 360) * NumDir);

    while (r >= NumDir)
    {
        r -= NumDir;
    }
    while (r < 0)
    {
        r += NumDir;
    }

    return CValue(r);
}
CValue AdvancedDirectionObjectExtension::getDistance(CValue x1, CValue y1, CValue x2, CValue y2)
{
    int x1v = x1.GetIntValue();
    int y1v = y1.GetIntValue();
    int x2v = x2.GetIntValue();
    int y2v = y2.GetIntValue();

    float r = (float)std::sqrt((x1v - x2v) * (x1v - x2v) + (y1v - y2v) * (y1v - y2v));
    return CValue(r);
}
CValue AdvancedDirectionObjectExtension::LongDir(CValue x1, CValue y1, CValue x2, CValue y2)
{
    int x1v = x1.GetIntValue();
    int y1v = y1.GetIntValue();
    int x2v = x2.GetIntValue();
    int y2v = y2.GetIntValue();

    //Just doing simple math now.
    float r = (float)(((((std::atan2(y2v - y1v, x2v - x1v) * 180) / PID) * -1) / 360) * NumDir);
    if ((int)r < NumDir / 2)
    {
        r += 0.5F;
    }
    if ((int)r > NumDir / 2)
    {
        r -= 0.5F;
    }
    while (r >= NumDir)
    {
        r -= NumDir;
    }
    while (r < 0)
    {
        r += NumDir;
    }

    return CValue((int)r);
}
CValue AdvancedDirectionObjectExtension::LongDist(CValue x1, CValue y1, CValue x2, CValue y2)
{
    int x1v = x1.GetIntValue();
    int y1v = y1.GetIntValue();
    int x2v = x2.GetIntValue();
    int y2v = y2.GetIntValue();

    return CValue((int)std::sqrt((x1v - x2v) * (x1v - x2v) + (y1v - y2v) * (y1v - y2v)));
}
CValue AdvancedDirectionObjectExtension::Rotate(CValue angle, CValue angletgt, CValue rotation)
{
    int anglev = angle.GetIntValue();
    int angletgtv = angletgt.GetIntValue();
    int rotationv = rotation.GetIntValue();

    
    if (rotationv < 0)
    {
        rotationv *= -1;
        angletgtv += NumDir / 2;
    }

    while (angletgtv < 0)
    {
        angletgtv += NumDir;
    }
    while (angletgtv >= NumDir)
    {
        angletgtv -= NumDir;
    }

    if (std::abs((int)(anglev - angletgtv)) <= rotationv)
    {
        anglev = angletgtv;
    }
    if (std::abs((int)(anglev - angletgtv - NumDir)) <= rotationv)
    {
        anglev = angletgtv;
    }
    if (std::abs((int)(anglev - angletgtv + NumDir)) <= rotationv)
    {
        anglev = angletgtv;
    }

    if (angletgtv != anglev)
    {
        if (anglev - angletgtv >= 0 && anglev - angletgtv < NumDir / 2)
        {
            anglev -= rotationv;
        }
        if (anglev - angletgtv >= NumDir / 2)
        {
            anglev += rotationv;
        }
        if (anglev - angletgtv <= 0 && anglev - angletgtv > NumDir / -2)
        {
            anglev += rotationv;
        }
        if (anglev - angletgtv <= NumDir / -2)
        {
            anglev -= rotationv;
        }
    }

    while (anglev >= NumDir)
    {
        anglev -= NumDir;
    }
    while (anglev < 0)
    {
        anglev += NumDir;
    }

    return CValue(anglev);
    
}
CValue AdvancedDirectionObjectExtension::DirDiffAbs(CValue p1, CValue p2)
{
    int p1v = p1.GetIntValue();
    int p2v = p2.GetIntValue();
    return CValue(lMin(std::abs(p1v - p2v), std::abs(p1v - p2v - NumDir), std::abs(p1v - p2v + NumDir)));
}
CValue AdvancedDirectionObjectExtension::DirDiff(CValue p1, CValue p2)
{
    int p1v = p1.GetIntValue();
    int p2v = p2.GetIntValue();
    return CValue(lSMin(p1v - p2v, p1v - p2v - NumDir, p1v - p2v + NumDir));
}
CValue AdvancedDirectionObjectExtension::GetFixedObj(CValue p1)
{
    int p1v = p1.GetIntValue();
    
    if (p1v >= CurrentObject || p1v < 0)
    {
        p1v = CurrentObject - 1;
    }
    int r = 0;
    if (CurrentObject > 0)
    {
        std::vector<bool> Fixes(CurrentObject, false);// = (long *)malloc(sizeof(long) * rdPtr->CurrentObject);
        for (int i = 0; i < CurrentObject; i++)
        {
            Fixes.push_back(Fixed[i]);
        }
        for (int i = 0; i <= p1v; i++)
        {
            int ClosestID = -1;
            for (int k = 0; k < CurrentObject; k++)
            {
                if (!Fixes[k])
                {
                    if (ClosestID == -1)
                    {
                        ClosestID = k;
                    }
                    else
                    {
                        float dAtK = (float)Distance[k];
                        float dAtClosestID = (float)Distance[ClosestID];
                        if (dAtK < dAtClosestID)
                        {
                            ClosestID = k;
                        }
                    }
                }
            }
            if (ClosestID != -1)
            {
                Fixes[ClosestID] = true;
                r = (int)Fixed[ClosestID];
            }
        }
    }
    return CValue(r);
}
CValue AdvancedDirectionObjectExtension::GetDistObj(CValue p1)
{
    int p1v = p1.GetIntValue();

    if (p1v >= CurrentObject || p1v < 0)
    {
        p1v = CurrentObject - 1;
    }
    long r = 0;
    if (CurrentObject > 0)
    {
        std::vector<bool> Fixes(CurrentObject, false);// = (long *)malloc(sizeof(long) * rdPtr->CurrentObject);
        for (int i = 0; i < CurrentObject; i++)
        {
            Fixes.push_back(Fixed[i]);
        }
        for (int i = 0; i <= p1v; i++)
        {
            int ClosestID = -1;
            for (int k = 0; k < CurrentObject; k++)
            {
                if (!Fixes[k])
                {
                    if (ClosestID == -1)
                    {
                        ClosestID = k;
                    }
                    else
                    {
                        float dAtK = (float)Distance[k];
                        float dAtClosestID = (float)Distance[ClosestID];
                        if (dAtK < dAtClosestID)
                        {
                            ClosestID = k;
                        }
                    }
                }
            }
            if (ClosestID != -1)
            {
                Fixes[ClosestID] = true;
                r = (int)((float)Distance[ClosestID]);
            }
        }
    }
    return CValue(r);
}
CValue AdvancedDirectionObjectExtension::XMov(CValue dir, CValue speed)
{
    int dirv = dir.GetIntValue();
    int speedv = speed.GetIntValue();

    float r;
    dirv = ((dirv * 360) / NumDir);
    if (dirv == 270 || dirv == 90)
    {
        r = 0;
    }
    else
    {
        float angle = (float)((dirv * PID * 2) / 360);
        r = (float)(std::cos(angle * -1) * speedv);
    }
    return CValue(r);
}
CValue AdvancedDirectionObjectExtension::YMov(CValue dir, CValue speed)
{
    int dirv = dir.GetIntValue();
    int speedv = speed.GetIntValue();

    float r;
    dirv = ((dirv * 360) / NumDir);
    if (dirv == 180 || dirv == 0)
    {
        r = 0;
    }
    else
    {
        float angle = (float)((dirv * PID * 2) / 360);
        r = (float)(std::sin(angle * -1) * speedv);
    }
    return CValue(r);
}
CValue AdvancedDirectionObjectExtension::DirBase(CValue p1, CValue p2)
{
    int p1v = p1.GetIntValue();
    int p2v = p2.GetIntValue();
    float r = (float)((p1v * p2v) / NumDir);
    return CValue(r);
}

#pragma endregion Expressions