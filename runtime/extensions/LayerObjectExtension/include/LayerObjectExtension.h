#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"
	
class LayerObjectExtension : public Extension {
public:
	LayerObjectExtension(unsigned int objectInfoHandle, int type, std::string name)
		: Extension(objectInfoHandle, type, name) {}

	void Initialize() override;

	unsigned int GetLayerIndex(std::string layerName);
	
	bool IsLayerVisible(int layerIndex);
	bool IsLayerVisible(std::string layerName) { return IsLayerVisible(GetLayerIndex(layerName)); }

	void YSort(bool ascending = true);
	void XSort(bool ascending = true);

	void AltValueSort(bool ascending, int altIndex, const CValue& defaultValue);

	void ShowLayer(int layerIndex);
	void ShowLayer(std::string layerName) { ShowLayer(GetLayerIndex(layerName)); }

	void HideLayer(int layerIndex);
	void HideLayer(std::string layerName) { HideLayer(GetLayerIndex(layerName)); }

	CValue GetLayerEffectParameter(const CValue& layerName, const CValue& parameter) { return CValue(0); }

private:
	unsigned int currentLayer = 1; // 1-indexed
};