#include "LayerObjectExtension.h"

#include <algorithm>

#include "Active.h"
#include "Application.h"
#include "Frame.h"

void LayerObjectExtension::Initialize()
{
    currentLayer = Layer;
}

unsigned int LayerObjectExtension::GetLayerIndex(std::string layerName)
{
    for (unsigned int i = 0; i < Application::Instance().GetCurrentFrame().get()->Layers.size(); i++) {
        if (Application::Instance().GetCurrentFrame().get()->Layers[i].Name == layerName) {
            return i;
        }
    }
    return 0;
}

bool LayerObjectExtension::IsLayerVisible(int layerIndex)
{
    return Application::Instance().GetCurrentFrame().get()->Layers[layerIndex].Visible;
}

void LayerObjectExtension::YSort(bool ascending)
{
    auto& layer = Application::Instance().GetCurrentFrame().get()->Layers[currentLayer];
    std::sort(layer.instances.begin(), layer.instances.end(), [ascending](ObjectInstance* a, ObjectInstance* b) { return ascending ? a->GetY() < b->GetY() : a->GetY() > b->GetY(); });
}

void LayerObjectExtension::XSort(bool ascending)
{
    auto& layer = Application::Instance().GetCurrentFrame().get()->Layers[currentLayer];
    std::sort(layer.instances.begin(), layer.instances.end(), [ascending](ObjectInstance* a, ObjectInstance* b) { return ascending ? a->GetX() < b->GetX() : a->GetX() > b->GetX(); });
}

void LayerObjectExtension::AltValueSort(bool ascending, int altIndex, const CValue& defaultValue)
{
    auto& layer = Application::Instance().GetCurrentFrame().get()->Layers[currentLayer];
    std::sort(layer.instances.begin(), layer.instances.end(), [ascending, altIndex, defaultValue](ObjectInstance* a, ObjectInstance* b) {
        double aValue = defaultValue.GetDoubleValue();
        double bValue = defaultValue.GetDoubleValue();

        if (Active* activeA = dynamic_cast<Active*>(a)) {
            aValue = activeA->Values.GetValue(altIndex).GetDoubleValue();
        }
        if (Active* activeB = dynamic_cast<Active*>(b)) {
            bValue = activeB->Values.GetValue(altIndex).GetDoubleValue();
        }

        return ascending ? aValue < bValue : aValue > bValue;
    });
}

void LayerObjectExtension::ShowLayer(int layerIndex)
{
    Application::Instance().GetCurrentFrame().get()->Layers[layerIndex].Visible = true;
}

void LayerObjectExtension::HideLayer(int layerIndex)
{
    Application::Instance().GetCurrentFrame().get()->Layers[layerIndex].Visible = false;
}