#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"
#include <string>
#include <vector>

class AdvancedDirectionObjectExtension : public Extension
{
public:
	AdvancedDirectionObjectExtension(unsigned int objectInfoHandle, int type, std::string name, int numDir)
		: Extension(objectInfoHandle, type, name), NumDir(numDir) {}

	void Update(float deltaTime) override { FrameCounter++; }

    //A
	void SetNumDir(CValue numDir) { NumDir = numDir.GetIntValue(); }
	void GetObjects(ObjectInstance* instance, CValue x, CValue y);
	void AddObjects(ObjectInstance* instance);
    //C
    bool CompDist(CValue x1, CValue y1, CValue x2, CValue y2, CValue v);
    bool CompDir(CValue x1, CValue y1, CValue x2, CValue y2, CValue dir, CValue offset);
    //E
	CValue GetNumDir() { return CValue(NumDir) };
	CValue Direction(CValue x1, CValue y1, CValue x2, CValue y2);
	CValue getDistance(CValue x1, CValue y1, CValue x2, CValue y2);
	CValue LongDir(CValue x1, CValue y1, CValue x2, CValue y2);
	CValue LongDist(CValue x1, CValue y1, CValue x2, CValue y2);
	CValue Rotate(CValue angle, CValue angletgt, CValue rotation);
	CValue DirDiffAbs(CValue p1, CValue p2);
	CValue DirDiff(CValue p1, CValue p2);
	CValue GetFixedObj(CValue p1);
	CValue GetDistObj(CValue p1);
	CValue XMov(CValue dir, CValue speed);
	CValue YMov(CValue dir, CValue speed);
	CValue DirBase(CValue p1, CValue p2);
private:
    int FrameCounter = 0;

	int NumDir = 0;
	int EventCount = -1;
    int CurrentObject = 0;

    std::vector<float> Distance;
    std::vector<int> Fixed;

    int LastX = 0;
    int LastY = 0;
    int lMin(int v1, int v2, int v3);
    int lSMin(int v1, int v2, int v3);
};