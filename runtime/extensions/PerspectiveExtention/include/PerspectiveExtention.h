#pragma once

#include "Extension.h"
	
class PerspectiveExtention : public Extension {
public:
PerspectiveExtention(unsigned int objectInfoHandle, int type, std::string name, short width, short height, char effect, bool direction, int zoomValue, int offset, int sineWaveWaves, bool perspectiveDir)
		: Extension(objectInfoHandle, type, name), width(width), height(height), effect(effect), direction(direction), zoomValue(zoomValue), offset(offset), sineWaveWaves(sineWaveWaves), perspectiveDir(perspectiveDir) {}

	void Initialize() override;
	void Draw(int scrollX, int scrollY) override;

	void UpdateShaderParameters();

	void SetEffectType(char effect);
	void SetDirection(bool direction);
	void SetPerspectiveDirection(bool perspectiveDir);

	void SetWidth(CValue width);
	void SetHeight(CValue height);
	void SetZoomValue(CValue zoomValue);
	void SetOffset(CValue offset);
	void SetSineWaveWaves(CValue sineWaveWaves);

	CValue GetWidth() const { return CValue(width); }
	CValue GetHeight() const { return CValue(height); }
	CValue GetZoomValue() const { return CValue(zoomValue); }
	CValue GetOffset() const { return CValue(offset); }
	CValue GetSineWaveWaves() const { return CValue(sineWaveWaves); }
private:
	short width;
	short height;
	char effect;
	bool direction;
	int zoomValue;
	int offset;
	int sineWaveWaves;
	bool perspectiveDir;
};