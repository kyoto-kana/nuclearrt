#include "Frame.h"

#include <map>
#include <algorithm>
#include <math.h>
#include <unordered_map>

#include "Application.h"
#include "Active.h"
#include "Counter.h"
#include "Extension.h"
#include "FontBank.h"
#include "ImageBank.h"
#include "ObjectGlobalDataCounter.h"
#include "GraphicsBackend.h"
#include "InputBackend.h"
#include "PlatformBackend.h"

constexpr float PI = 3.14159265358979323846f;

void Frame::Initialize()
{
}

void Frame::PostInitialize()
{
}

void Frame::Update()
{
	updateCount++;
	
	float deltaTime = Application::Instance().GetBackend()->platform->GetTimeDelta();
	GameTimer.Update(deltaTime);
	scrollDirty = false;

	DispatchTrueEvents();

	std::vector<unsigned int> handles;
	handles.reserve(ObjectInstances.size());
	for (auto& [handle, instance] : ObjectInstances)
	{
		handles.push_back(handle);
	}

	for (unsigned int handle : handles)
	{
		auto found = ObjectInstances.find(handle);
		if (found == ObjectInstances.end()) continue;
		ObjectInstance* instance = found->second;

		//Animation update
		if (instance->Type == 2) // Common object with possible animation
		{
			((Active*)instance)->movements.Update(deltaTime);

			int movementSpeed = 0;
			int movementMinimumSpeed = 0;
			int movementMaximumSpeed = 0;
			auto* movement = ((Active*)instance)->movements.GetCurrentMovement();
			if (movement != nullptr) {
				movementSpeed = movement->GetRealSpeed().GetIntValue();
				movementMinimumSpeed = movement->GetMinimumSpeed();
				movementMaximumSpeed = movement->GetMaximumSpeed();
			}
			((Active*)instance)->animations.SetAnimationSpeed(movementSpeed, movementMinimumSpeed, movementMaximumSpeed);
			((Active*)instance)->animations.Update(deltaTime);
		}
		else if (instance->Type == 5 || instance->Type == 6 || instance->Type == 7) // Counter
		{
			((CounterBase*)instance)->movements.Update(deltaTime);
		}
		else if (instance->Type >= 32) // Extension
		{
			((Extension*)instance)->Update(deltaTime);
		}
	}

	//TODO: should timer update be here?
	GameTimer.ProcessNamedEvents([this](const std::string& eventName) {
		OnTimerEvent(eventName);
	});
}

void Frame::DispatchTrueEvents()
{
	if (updateCount == 1)
	{
		GenerateEvent(-3, -1);
	}

	auto input = Application::Instance().GetInput();
	if (input->IsAnyKeyPressed())
	{
		GenerateEvent(-6, -9); // Upon pressing any key
	}

	if (input->IsAnyMouseButtonPressed())
	{
		GenerateEvent(-6, -5); // Upon clicking
		GenerateEvent(-6, -6); // Upon clicking in zone
		GenerateEvent(-6, -7); // Upon clicking on object
	}

	for (int player = 0; player < 4; player++)
	{
		if (input->IsControlsPressed(player, 0xFF))
		{
			GenerateEvent(-7, -4);
			break;
		}
	}
}

void Frame::Draw()
{
	Application::Instance().GetBackend()->graphics->Clear(BackgroundColor);

	for (unsigned int i = 0; i < Layers.size(); i++)
	{
		Layer& layer = Layers[i];
		if (!layer.Visible) continue;
		
		bool hasLayerEffect = (layer.usePreviousLayerEffect && i > 0) || layer.Effect != 0 || layer.GetEffectParameter() != 0 || layer.RGBCoefficient != 0xFFFFFFFF;
		if (!hasLayerEffect)
		{
			DrawLayer(layer);
			continue;
		}

		Application::Instance().GetBackend()->graphics->BeginLayerDrawing();
		DrawLayer(layer);

		EffectInstance* effectInstance = nullptr;
		int effect = 0;
		int effectParameter = 0;
		int rgbCoefficient = 0xFFFFFF;

		if (hasLayerEffect)
		{
			effectInstance = layer.effectInstance;
			effect = layer.Effect;
			effectParameter = layer.GetEffectParameter();
			rgbCoefficient = layer.RGBCoefficient;

			if (layer.usePreviousLayerEffect && i > 0)
			{
				int previousLayerIndex = i;
				while (previousLayerIndex > 0 && Layers[previousLayerIndex].usePreviousLayerEffect)
				{
					previousLayerIndex--;
				}
				effect = Layers[previousLayerIndex].Effect;
				effectInstance = Layers[previousLayerIndex].effectInstance;
				effectParameter = Layers[previousLayerIndex].GetEffectParameter();
				rgbCoefficient = Layers[previousLayerIndex].RGBCoefficient;
			}
		}

		Application::Instance().GetBackend()->graphics->EndLayerDrawing(rgbCoefficient, effect, effectParameter, effectInstance);
	}
}

void Frame::SetScroll(const CValue& x, const CValue& y, int layer)
{
	int windowWidth = Application::Instance().GetAppData()->GetWindowWidth();
	int windowHeight = Application::Instance().GetAppData()->GetWindowHeight();

	float requestedX = static_cast<float>(x.GetIntValue());
	float requestedY = static_cast<float>(y.GetIntValue());

	requestedX -= windowWidth / 2.0f;
	requestedY -= windowHeight / 2.0f;

	if (layer != -1 && layer < static_cast<int>(Layers.size()))
	{
		if (Layers[layer].XCoefficient > 1.0f)
		{
			float dxf = (requestedX - scrollX);
			dxf /= Layers[layer].XCoefficient;
			requestedX = scrollX + dxf;
		}

		if (Layers[layer].YCoefficient > 1.0f)
		{
			float dyf = (requestedY - scrollY);
			dyf /= Layers[layer].YCoefficient;
			requestedY = scrollY + dyf;
		}
	}

	int intX = static_cast<int>(requestedX);
	int intY = static_cast<int>(requestedY);

	intX = std::max(0, intX);
	intY = std::max(0, intY);
	intX = std::min(std::max(0, Width - windowWidth), intX);
	intY = std::min(std::max(0, Height - windowHeight), intY);

	if (intX == scrollX && intY == scrollY) return;
	UpdateScrolling(intX, intY);
	scrollDirty = true;
}

void Frame::SetScrollX(const CValue& x)
{
	int windowWidth = Application::Instance().GetAppData()->GetWindowWidth();
	int intX = x.GetIntValue();
	intX -= windowWidth / 2;
	intX = std::max(0, intX);
	intX = std::min(std::max(0, Width - windowWidth), intX);
	if (intX == scrollX) return;
	UpdateScrolling(intX, scrollY);
	scrollDirty = true;
}

void Frame::SetScrollY(const CValue& y)
{
	int windowHeight = Application::Instance().GetAppData()->GetWindowHeight();
	int intY = y.GetIntValue();
	intY -= windowHeight / 2;
	intY = std::max(0, intY);
	intY = std::min(std::max(0, Height - windowHeight), intY);
	if (intY == scrollY) return;
	UpdateScrolling(scrollX, intY);
	scrollDirty = true;
}

void Frame::UpdateScrolling(int newX, int newY)
{
	int oldX = scrollX;
	int oldY = scrollY;
	int deltaX = newX - oldX;
	int deltaY = newY - oldY;

	scrollX = newX;
	scrollY = newY;

	if (deltaX == 0 && deltaY == 0) return;

	for (auto& [handle, instance] : ObjectInstances)
	{
		if (instance->Type == 0 || instance->Type == 1) continue;

		if (!instance->FollowFrame)
		{
			if (deltaX != 0)
				instance->SetX(CValue(instance->GetX().GetIntValue() + deltaX));
			if (deltaY != 0)
				instance->SetY(CValue(instance->GetY().GetIntValue() + deltaY));
			continue;
		}

		if (instance->Layer >= Layers.size()) continue;
		const Layer& layer = Layers[instance->Layer];

		int oldLayerDx = oldX;
		int oldLayerDy = oldY;
		int newLayerDx = newX;
		int newLayerDy = newY;

		oldLayerDx = static_cast<int>(layer.XCoefficient * oldLayerDx);
		newLayerDx = static_cast<int>(layer.XCoefficient * newLayerDx);
		oldLayerDy = static_cast<int>(layer.YCoefficient * oldLayerDy);
		newLayerDy = static_cast<int>(layer.YCoefficient * newLayerDy);

		int nX = (instance->GetX().GetIntValue() + oldLayerDx) - newLayerDx + deltaX;
		int nY = (instance->GetY().GetIntValue() + oldLayerDy) - newLayerDy + deltaY;

		instance->SetPosition(CValue(nX), CValue(nY));
	}
}

CValue Frame::GetXLeftEdge()
{
	return CValue(scrollX);
}

CValue Frame::GetXRightEdge()
{
	return CValue(scrollX + Application::Instance().GetAppData()->GetWindowWidth());
}

CValue Frame::GetYTopEdge()
{
	return CValue(scrollY);
}

CValue Frame::GetYBottomEdge()
{
	return CValue(scrollY + Application::Instance().GetAppData()->GetWindowHeight());
}

void Frame::DrawLayer(Layer& layer)
{
	const int viewWidth = Application::Instance().GetAppData()->GetWindowWidth();
	const int viewHeight = Application::Instance().GetAppData()->GetWindowHeight();

	for (auto& instance : layer.instances)
	{
		//cull objects outside of view
		if (instance->Type <= 3)
		{
			CollisionInstanceBounds bounds = GetInstanceBounds(instance);
			int offsetX = scrollX;
			int offsetY = scrollY;
			if (instance->Type <= 1)
			{
				offsetX = static_cast<int>(offsetX * layer.XCoefficient);
				offsetY = static_cast<int>(offsetY * layer.YCoefficient);
			}

			if (instance->Type == 2 && ((Active*)instance)->AutomaticRotation)
			{
				int dx = std::max<int>(bounds.hotspotX, bounds.width - bounds.hotspotX);
				int dy = std::max<int>(bounds.hotspotY, bounds.height - bounds.hotspotY);
				int radius = (int)std::ceil(std::sqrt((double)dx * dx + (double)dy * dy));
				int cx = bounds.centerX - offsetX;
				int cy = bounds.centerY - offsetY;

				if (cx + radius < 0 || cx - radius > viewWidth ||
					cy + radius < 0 || cy - radius > viewHeight)
					continue;
			}
			else if (bounds.maxX - offsetX < 0 || bounds.minX - offsetX > viewWidth ||
				bounds.maxY - offsetY < 0 || bounds.minY - offsetY > viewHeight)
				continue;
		}

		if (instance->Type == 1)
		{
			auto& imageBank = ImageBank::Instance();
			unsigned int imageId = ((Backdrop*)instance)->Image;

			Application::Instance().GetBackend()->graphics->DrawTexture(
				imageId, instance->GetX().GetIntValue() - (scrollX * layer.XCoefficient), instance->GetY().GetIntValue() - (scrollY * layer.YCoefficient),
				0, 0, 0, 1.0f, 1.0f, instance->RGBCoefficient, instance->Effect, instance->GetBlendCoefficient().GetIntValue(), instance->effectInstance);
		}
		else if (instance->Type == 2)
		{
			if (!((Active*)instance)->Visible) continue;
			if (((Active*)instance)->GetXScale() <= 0.0f || ((Active*)instance)->GetYScale() <= 0.0f) continue;

			auto& imageBank = ImageBank::Instance();
			auto& animations = ((Active*)instance)->animations;
			unsigned int imageId = animations.GetCurrentImageHandle();

			auto imageInfo = imageBank.GetImage(imageId);
			if (imageInfo)
			{
				int angle = ((Active*)instance)->GetAngle();
				if (((Active*)instance)->AutomaticRotation)
				{
					auto movement = ((Active*)instance)->movements.GetCurrentMovement();
					if (movement != nullptr && !animations.IsDirectionForced())
					{
						angle += movement->GetMovementDirection() * 180 / 16;
					}
					else
					{
						angle += animations.GetAutomaticRotationDirection() * 180 / 16;
					}
				}

				Application::Instance().GetBackend()->graphics->DrawTexture(
					imageId, instance->GetX().GetIntValue() - scrollX, instance->GetY().GetIntValue() - scrollY,
					imageInfo->HotspotX, imageInfo->HotspotY, 
					angle, ((Active*)instance)->GetXScale().GetDoubleValue(), ((Active*)instance)->GetYScale().GetDoubleValue(), instance->RGBCoefficient, instance->Effect, instance->GetBlendCoefficient().GetIntValue(), instance->effectInstance);
			}
		}
		else if (instance->Type == 3) // Text
		{
			if (!((StringObject*)instance)->Visible) continue;

			std::string text = ((StringObject*)instance)->GetText().GetStringValue();
			Application::Instance().GetBackend()->graphics->DrawText(
				FontBank::Instance().GetFont(((StringObject*)instance)->GetFont()),
				instance->GetX().GetIntValue() - scrollX,
				instance->GetY().GetIntValue() - scrollY,
				((StringObject*)instance)->Width,
				((StringObject*)instance)->Height,
				((StringObject*)instance)->GetHorizontalAlignment(),
				((StringObject*)instance)->GetVerticalAlignment(),
				((StringObject*)instance)->GetColor(),
				text,
				instance->Handle,
				instance->RGBCoefficient,
				instance->Effect,
				instance->GetBlendCoefficient().GetIntValue(),
				instance->effectInstance
			);
		}
		else if (instance->Type == 5 || instance->Type == 6 || instance->Type == 7) // Score, Lives, Counter
		{
			CounterBase* counter = (CounterBase*)instance;
			if (!counter->Visible) continue;
			

			//TODO: Add support for other display types
			if (counter->DisplayType == 1) // Numbers
			{				
				DrawCounterNumbers(counter, counter->GetValue().GetIntValue(), instance->GetX().GetIntValue() - scrollX, instance->GetY().GetIntValue() - scrollY);
			}
			else if (counter->DisplayType == 2 || counter->DisplayType == 3) // Bar
			{
				Application::Instance().GetBackend()->graphics->DrawCounterBar(instance->GetX().GetIntValue() - scrollX, instance->GetY().GetIntValue() - scrollY, (Counter*)counter);
			}
			else if (counter->DisplayType == 4) // Animation
			{
				if (instance->Type == 6) // Lives
				{
					int count = Application::Instance().GetAppData()->GetPlayerLives()[counter->Player];
					int imageWidth = ImageBank::Instance().GetImage(counter->Frames[0])->Width;
					int imageHeight = ImageBank::Instance().GetImage(counter->Frames[0])->Height;
					int x = instance->GetX().GetIntValue() - scrollX;
					int y = instance->GetY().GetIntValue() - scrollY;
					for (int i = 0; i < count; i++)
					{
						if (counter->Width > 0 && (x - (instance->GetX().GetIntValue() - scrollX)) >= counter->Width)
						{
							x = instance->GetX().GetIntValue() - scrollX; 
							y += imageHeight;
						}

						Application::Instance().GetBackend()->graphics->DrawTexture(counter->Frames[0], x, y, 0, 0, 0, 1.0f, 1.0f, instance->RGBCoefficient, instance->Effect, instance->GetBlendCoefficient().GetIntValue(), instance->effectInstance);
						x += imageWidth;
					}
				}
				else
				{
					int imageID = 0;
					int frameIndex = 0;
					int imageCount = counter->Frames.size();
					
					if (imageCount > 1)
					{
						int value = ((Counter*)instance)->GetValue().GetIntValue();
						int minValue = ((Counter*)instance)->MinValue.GetIntValue();
						int maxValue = ((Counter*)instance)->MaxValue.GetIntValue();

						if (value >= maxValue)
						{
							frameIndex = imageCount - 1;
						}
						else
						{
							frameIndex = (value - minValue) * (imageCount - 1) / (maxValue - minValue);
							frameIndex = std::clamp(frameIndex, 0, imageCount - 2);
						}
					}
					
					imageID = counter->Frames[frameIndex];

					Application::Instance().GetBackend()->graphics->DrawTexture(imageID, instance->GetX().GetIntValue() - scrollX, instance->GetY().GetIntValue() - scrollY, 0, 0, 0, 1.0f, 1.0f, instance->RGBCoefficient, instance->Effect, instance->GetBlendCoefficient().GetIntValue(), instance->effectInstance);
				}
			}
		}
		else if (instance->Type == 0) // Quick backdrop
		{
			int scrollXOffset = scrollX * layer.XCoefficient;
			int scrollYOffset = scrollY * layer.YCoefficient;
			Application::Instance().GetBackend()->graphics->DrawQuickBackdrop(instance->GetX().GetIntValue() - scrollXOffset, instance->GetY().GetIntValue() - scrollYOffset, ((QuickBackdrop*)instance)->Width, ((QuickBackdrop*)instance)->Height, &((QuickBackdrop*)instance)->shape);
		}
		else if (instance->Type >= 32) // Extension
		{
			if (!((Extension*)instance)->Visible) continue;
			((Extension*)instance)->Draw(scrollX, scrollY);
		}
	}
}

void Frame::DrawCounterNumbers(CounterBase *counter, int value, int x, int y)
{
	std::string valueString = std::to_string(value);
	int numDigits = valueString.size();

	if (counter->IntDigitCount > 0) numDigits = counter->IntDigitCount;

	bool negative = false;
	if (value < 0)
	{
		negative = true;
		valueString = valueString.substr(1);
	}

	//Fixed Size
	if (counter->IntDigitCount > 0)
	{
		if (counter->IntDigitCount > valueString.size()) //Add leading zeros
		{
			while (valueString.size() < counter->IntDigitCount)
			{
				valueString = "0" + valueString;
			}
		}
		else //Remove extra digits
		{
			valueString = valueString.substr(valueString.size() - counter->IntDigitCount);
		}
	}		

	if (negative)
	{
		valueString = "-" + valueString;
		numDigits++;
	}

	//Counter char map
	std::string charMap = "0123456789-+.e";

	//get the total width of the string to be displayed
	int totalWidth = 0;
	for (int i = 0; i < numDigits; i++)
	{
		if (i >= valueString.size())
		{
			break;
		}

		int imageIndex = charMap.find(valueString[i]);
		totalWidth += ImageBank::Instance().GetImage(counter->Frames[imageIndex])->Width;
	}

	//Get height of the tallest character displayed
	int MaxHeight = 0;
	for (int i = 0; i < numDigits; i++)
	{
		if (i >= valueString.size())
		{
			break;
		}

		int imageIndex = charMap.find(valueString[i]);
		MaxHeight = std::max((unsigned short)MaxHeight, ImageBank::Instance().GetImage(counter->Frames[imageIndex])->Height);
	}

	int currentX = x - totalWidth;

	//Draw
	for (int i = 0; i < numDigits; i++)
	{
		if (i >= valueString.size())
		{
			break;
		}

		int imageIndex = charMap.find(valueString[i]);
		auto imageInfo = ImageBank::Instance().GetImage(counter->Frames[imageIndex]);
		if (imageInfo)
		{
			Application::Instance().GetBackend()->graphics->DrawTexture(
				counter->Frames[imageIndex], currentX, y - MaxHeight,
				0, 0, 0, 1.0f, 1.0f, counter->RGBCoefficient, counter->Effect, counter->GetBlendCoefficient().GetIntValue(), counter->effectInstance);
			currentX += imageInfo->Width;
		}
	}
}

std::vector<unsigned int> Frame::GetImagesUsed()
{
	std::vector<unsigned int> imagesUsed;
	
	for (auto& [handle, instance] : ObjectInstances)
	{
		std::vector<unsigned int> instanceImages = instance->GetImagesUsed();
		for (unsigned int image : instanceImages)
		{
			if (std::find(imagesUsed.begin(), imagesUsed.end(), image) == imagesUsed.end())
			{
				imagesUsed.push_back(image);
			}
		}
	}

	return imagesUsed;
}

std::vector<unsigned int> Frame::GetFontsUsed()
{
	std::vector<unsigned int> fontsUsed;

	for (auto& [handle, instance] : ObjectInstances)
	{
		std::vector<unsigned int> instanceFonts = instance->GetFontsUsed();
		for (unsigned int font : instanceFonts)
		{
			if (std::find(fontsUsed.begin(), fontsUsed.end(), font) == fontsUsed.end())
			{
				fontsUsed.push_back(font);
			}
		}
	}

	//erase any duplicates
	std::sort(fontsUsed.begin(), fontsUsed.end());
	fontsUsed.erase(std::unique(fontsUsed.begin(), fontsUsed.end()), fontsUsed.end());

	return fontsUsed;
}

ObjectInstance* Frame::CreateInstance(ObjectInstance* createdInstance, short x, short y, unsigned int layer, short instanceValue, unsigned int objectInfoHandle, short angle, bool postInitialize, ObjectInstance* parentInstance)
{
	createdInstance->Handle = ++MaxObjectInstanceHandle;
	createdInstance->SetPosition(x, y);
	createdInstance->Layer = layer;
	createdInstance->InstanceValue = instanceValue;
	createdInstance->ObjectInfoHandle = objectInfoHandle;
	createdInstance->SetAngle(angle);
	createdInstance->FixedValue = (createdInstance->Handle << 16) | ((createdInstance->Handle-1) & 0xFFFF);

	//TODO: move this to a separate function
	// Load any textures needed for this instance
	if (postInitialize) // objs created on init have textures loaded in application
	{
		std::vector<unsigned int> texturesToLoad = createdInstance->GetImagesUsed();
		auto backend = Application::Instance().GetBackend();
		for (unsigned int textureId : texturesToLoad) {
			backend->graphics->LoadTexture(textureId);
		}
	}
	
	ObjectInstances[createdInstance->Handle] = createdInstance;
	for (ObjectSelector* selector : Selectors) {
		selector->Track(createdInstance);
	}
	if (parentInstance) {
		createdInstance->SetX(createdInstance->GetX() + parentInstance->GetX());
		createdInstance->SetY(createdInstance->GetY() + parentInstance->GetY());
		createdInstance->Layer = parentInstance->Layer;
	}

	//init movement and extensions
	if (createdInstance->Type == 2) // Common object
	{
		for (auto& [handle, movement] : ((Active*)createdInstance)->movements.items)
		{
			movement->Instance = createdInstance;
			movement->Initialize();

			if (handle == 0) movement->OnEnabled();
		}

		((Active*)createdInstance)->animations.AutomaticRotation = ((Active*)createdInstance)->AutomaticRotation;
	}
	else if (createdInstance->Type == 5 || createdInstance->Type == 6 || createdInstance->Type == 7) // Counter
	{
		for (auto& [handle, movement] : ((CounterBase*)createdInstance)->movements.items)
		{
			movement->Instance = createdInstance;
			movement->Initialize();

			if (handle == 0) movement->OnEnabled();
		}
	}
	else if (createdInstance->Type >= 32) // Extension
	{
		((Extension*)createdInstance)->Initialize();
	}

	Layers[createdInstance->Layer].instances.push_back(createdInstance);

	return createdInstance;
}

std::vector<ObjectGlobalData*> Frame::GetGlobalObjectData()
{
	std::map<unsigned int, std::vector<ObjectInstance*>> instancesByHandle;
	for (auto& [handle, instance] : ObjectInstances)
	{
		if (instance->global)
		{
			instancesByHandle[instance->ObjectInfoHandle].push_back(instance);
		}
	}
	
	for (auto& [objInfoHandle, instances] : instancesByHandle)
	{
		std::sort(instances.begin(), instances.end(), 
			[](ObjectInstance* a, ObjectInstance* b) { return a->Handle < b->Handle; });
	}
	
	std::vector<ObjectGlobalData*> result;
	for (auto& [objInfoHandle, instances] : instancesByHandle)
	{
		for (auto* instance : instances)
		{
			ObjectGlobalData* data = instance->CreateGlobalData();
			if (data != nullptr)
			{
				result.push_back(data);
			}
		}
	}
	
	return result;
}

void Frame::MoveObjectToLayer(ObjectInstance* instance, unsigned int layer)
{
	if (instance->Layer == layer) return;
	if (layer >= Layers.size()) return;

	Layers[instance->Layer].instances.erase(std::find(Layers[instance->Layer].instances.begin(), Layers[instance->Layer].instances.end(), instance));
	Layers[layer].instances.push_back(instance);
	instance->Layer = layer;
}

void Frame::MoveObjectToFront(ObjectInstance* instance)
{	
	Layers[instance->Layer].instances.erase(std::find(Layers[instance->Layer].instances.begin(), Layers[instance->Layer].instances.end(), instance));
	Layers[instance->Layer].instances.push_back(instance);
}

void Frame::MoveObjectToBack(ObjectInstance* instance)
{
	Layers[instance->Layer].instances.erase(std::find(Layers[instance->Layer].instances.begin(), Layers[instance->Layer].instances.end(), instance));
	Layers[instance->Layer].instances.insert(Layers[instance->Layer].instances.begin(), instance);
}

void Frame::MoveObjectInFrontOf(ObjectInstance* instance, unsigned int oiHandle)
{	
	int maxIndex = -1;
	for (int i = 0; i < Layers[instance->Layer].instances.size(); i++)
	{
		if (Layers[instance->Layer].instances[i]->ObjectInfoHandle == oiHandle)
		{
			maxIndex = i;
		}
	}

	if (maxIndex == -1) return;
	Layers[instance->Layer].instances.erase(std::find(Layers[instance->Layer].instances.begin(), Layers[instance->Layer].instances.end(), instance));
	Layers[instance->Layer].instances.insert(Layers[instance->Layer].instances.begin() + maxIndex + 1, instance);
}

void Frame::MoveObjectBehindOf(ObjectInstance* instance, unsigned int oiHandle)
{
	
	int minIndex = -1;
	for (int i = Layers[instance->Layer].instances.size() - 1; i >= 0; i--)
	{
		if (Layers[instance->Layer].instances[i]->ObjectInfoHandle == oiHandle)
		{
			minIndex = i;
		}
	}
	
	if (minIndex == -1) return;
	Layers[instance->Layer].instances.erase(std::find(Layers[instance->Layer].instances.begin(), Layers[instance->Layer].instances.end(), instance));
	Layers[instance->Layer].instances.insert(Layers[instance->Layer].instances.begin() + minIndex, instance);
}

int Frame::GetMouseX()
{
	return Application::Instance().GetBackend()->input->GetMouseX() + scrollX;
}

int Frame::GetMouseY()
{
	return Application::Instance().GetBackend()->input->GetMouseY() + scrollY;
}

void Frame::ApplyGlobalObjectData(std::vector<ObjectGlobalData*> savedData)
{
	std::map<unsigned int, std::vector<ObjectGlobalData*>> dataByHandle;
	for (auto* data : savedData)
	{
		dataByHandle[data->objectInfoHandle].push_back(data);
	}
	
	std::map<unsigned int, std::vector<ObjectInstance*>> instancesByHandle;
	for (auto& [handle, instance] : ObjectInstances)
	{
		if (instance->global)
		{
			ObjectGlobalData* testData = instance->CreateGlobalData();
			if (testData != nullptr)
			{
				delete testData;
				instancesByHandle[instance->ObjectInfoHandle].push_back(instance);
			}
		}
	}
	
	for (auto& [objInfoHandle, instances] : instancesByHandle)
	{
		std::sort(instances.begin(), instances.end(), 
			[](ObjectInstance* a, ObjectInstance* b) { return a->Handle < b->Handle; });
	}
	
	for (auto& [objInfoHandle, instances] : instancesByHandle)
	{
		if (dataByHandle.find(objInfoHandle) == dataByHandle.end())
		{
			continue;
		}
		
		auto& dataList = dataByHandle[objInfoHandle];
		int savedCount = dataList.size();
		int currentCount = instances.size();
		
		if (savedCount <= currentCount)
		{
			int startIndex = currentCount - savedCount;
			for (int i = 0; i < savedCount; i++)
			{
				instances[startIndex + i]->ApplyGlobalData(dataList[i]);
			}
		}
		else
		{
			int dataStartIndex = savedCount - currentCount;
			for (int i = 0; i < currentCount; i++)
			{
				instances[i]->ApplyGlobalData(dataList[dataStartIndex + i]);
			}
		}
	}
}

//Check if the object is colliding with any backdrop
bool Frame::IsCollidingWithBackground(ObjectInstance *instance)
{	
	// Check collision with all backdrop objects
	for (auto& backdropInstance : Layers[instance->Layer].instances)
	{
		// Check only against (Quick) backdrop objects
		if (backdropInstance->Type == 1)
		{
			Backdrop* backdrop = (Backdrop*)backdropInstance;
			
			// Check if this backdrop is an obstacle
			if (backdrop->ObstacleType > 0) {
				if (IsColliding(instance, backdrop)) return true;
			}
		}
		else if (backdropInstance->Type == 0)
		{
			QuickBackdrop* quickBackdrop = (QuickBackdrop*)backdropInstance;
			
			// Check if this backdrop is an obstacle
			if (quickBackdrop->ObstacleType > 0) {
				if (IsColliding(instance, quickBackdrop)) return true;
			}
		}
	}
	
	return false; // No collision with any backdrop
}

CollisionInstanceBounds Frame::GetInstanceBounds(ObjectInstance* instance) {
	if (instance->collisionBoundsDirty) {
		CollisionInstanceBounds bounds{};
		bounds.angle = instance->GetAngle();
		bounds.scaleX = 1.0f;
		bounds.scaleY = 1.0f;

		unsigned int imageId = 0;
		int drawX = instance->GetX().GetIntValue();
		int drawY = instance->GetY().GetIntValue();
		int hotspotX = 0, hotspotY = 0;

		if (instance->Type == 0) { // Quick backdrop
			imageId = ((QuickBackdrop*)instance)->shape.Image;
			bounds.width = ((QuickBackdrop*)instance)->Width;
			bounds.height = ((QuickBackdrop*)instance)->Height;
			bounds.maskWidth = bounds.width;
			bounds.maskHeight = bounds.height;
		} else if (instance->Type == 1) { // Backdrop
			imageId = ((Backdrop*)instance)->Image;
			auto imageInfo = ImageBank::Instance().GetImage(imageId);
			if (imageInfo) {
				bounds.width = imageInfo->Width;
				bounds.height = imageInfo->Height;
				bounds.maskWidth = imageInfo->Width;
				bounds.maskHeight = imageInfo->Height;
			}
		} else if (instance->Type == 3) { // String
			StringObject* stringObj = (StringObject*)instance;
			bounds.width = stringObj->Width;
			bounds.height = stringObj->Height;
			bounds.maskWidth = bounds.width;
			bounds.maskHeight = bounds.height;
		} else if (instance->Type == 5 || instance->Type == 6 || instance->Type == 7) { // Counters
			CounterBase* counter = (CounterBase*)instance;
			bounds.width = counter->GetWidth();
			bounds.height = counter->GetHeight();
			bounds.maskWidth = bounds.width;
			bounds.maskHeight = bounds.height;

			if (counter->DisplayType == 1) {
				drawX -= bounds.width;
				drawY -= bounds.height;
			}
		} else { // Active object
			Active* active = (Active*)instance;
			imageId = active->animations.GetCurrentImageHandle();
			bounds.scaleX = active->GetXScale().GetDoubleValue();
			bounds.scaleY = active->GetYScale().GetDoubleValue();
			auto imageInfo = ImageBank::Instance().GetImage(imageId);
			if (imageInfo) {
				bounds.maskWidth = imageInfo->Width;
				bounds.maskHeight = imageInfo->Height;
				hotspotX = (int)std::lround(imageInfo->HotspotX * bounds.scaleX);
				hotspotY = (int)std::lround(imageInfo->HotspotY * bounds.scaleY);
				bounds.width = std::max((int)std::lround(imageInfo->Width * bounds.scaleX), 1);
				bounds.height = std::max((int)std::lround(imageInfo->Height * bounds.scaleY), 1);
			}
		}

		bounds.centerX = drawX;
		bounds.centerY = drawY;
		bounds.imageId = imageId;
		bounds.hotspotX = hotspotX;
		bounds.hotspotY = hotspotY;

		int x1 = drawX - hotspotX;
		int y1 = drawY - hotspotY;
		int x2 = x1 + bounds.width;
		int y2 = y1;
		int x3 = x1;
		int y3 = y1 + bounds.height;
		int x4 = x2;
		int y4 = y3;

		if (bounds.angle != 0) {
			float rotationAngle = 360.0f - bounds.angle;
			float radians = rotationAngle * (PI / 180.0f);
			float cosA = cos(radians);
			float sinA = sin(radians);
			float dx1 = x1 - bounds.centerX, dy1 = y1 - bounds.centerY;
			float dx2 = x2 - bounds.centerX, dy2 = y2 - bounds.centerY;
			float dx3 = x3 - bounds.centerX, dy3 = y3 - bounds.centerY;
			float dx4 = x4 - bounds.centerX, dy4 = y4 - bounds.centerY;

			x1 = bounds.centerX + (int)(dx1 * cosA - dy1 * sinA);
			y1 = bounds.centerY + (int)(dx1 * sinA + dy1 * cosA);
			x2 = bounds.centerX + (int)(dx2 * cosA - dy2 * sinA);
			y2 = bounds.centerY + (int)(dx2 * sinA + dy2 * cosA);
			x3 = bounds.centerX + (int)(dx3 * cosA - dy3 * sinA);
			y3 = bounds.centerY + (int)(dx3 * sinA + dy3 * cosA);
			x4 = bounds.centerX + (int)(dx4 * cosA - dy4 * sinA);
			y4 = bounds.centerY + (int)(dx4 * sinA + dy4 * cosA);
		}

		bounds.minX = std::min({x1, x2, x3, x4});
		bounds.minY = std::min({y1, y2, y3, y4});
		bounds.maxX = std::max({x1, x2, x3, x4});
		bounds.maxY = std::max({y1, y2, y3, y4});

		instance->collisionBoundsDirty = false;
		instance->collisionBounds = bounds;
	}

	CollisionInstanceBounds result = instance->collisionBounds;
	result.scrollX = 0;
	result.scrollY = 0;
	if (instance->Layer < Layers.size())
	{
		if (instance->Type == 0 || instance->Type == 1) // Quick backdrop or backdrop
		{
			result.scrollX = static_cast<short>(scrollX * Layers[instance->Layer].XCoefficient);
			result.scrollY = static_cast<short>(scrollY * Layers[instance->Layer].YCoefficient);
		}
		else
		{
			result.scrollX = static_cast<short>(scrollX);
			result.scrollY = static_cast<short>(scrollY);
		}
	}
	return result;
}

bool Frame::IsPointInRotatedBox(int worldX, int worldY, const CollisionInstanceBounds& bounds) {
	float dx = worldX - bounds.centerX + bounds.scrollX;
	float dy = worldY - bounds.centerY + bounds.scrollY;
	if (bounds.angle != 0) {
		float rotationAngle = 360.0f - bounds.angle;
		float radians = -rotationAngle * (PI / 180.0f);
		float cosA = cos(radians);
		float sinA = sin(radians);
		float newX = dx * cosA - dy * sinA;
		float newY = dx * sinA + dy * cosA;
		dx = newX;
		dy = newY;
	}
	int localX = (int)(dx + bounds.hotspotX);
	int localY = (int)(dy + bounds.hotspotY);
	return localX >= 0 && localX < bounds.width && localY >= 0 && localY < bounds.height;
}

bool Frame::IsPixelSolid(const std::vector<uint8_t>& maskData, int width, int height, int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return false;
	
	int bytesPerRow = (width + 7) / 8;
	int byteIndex = y * bytesPerRow + (x / 8);
	int bitIndex = 7 - (x % 8);
	
	if (byteIndex >= (int)maskData.size()) return false;
	
	return (maskData[byteIndex] & (1 << bitIndex)) != 0;
}

static int ConvertScaledCoordToMaskCoord(int scaledCoord, float scale, int maskSize) {
	(void)maskSize;
	return (int)std::floor(scaledCoord / scale);
}

static inline void BuildQuad(const CollisionInstanceBounds& bounds, float sinAngle, float cosAngle, float* vertices)
{
	float left = -bounds.hotspotX;
	float top = -bounds.hotspotY;
	float right = left + bounds.width;
	float bottom = top + bounds.height;
	const Vector2D verts[4] = { { left, top }, { right, top }, {right, bottom}, {left, bottom} };
	float x = bounds.centerX - bounds.scrollX;
	float y = bounds.centerY - bounds.scrollY;
	for (int i = 0; i < 4; i++)
	{
		vertices[i * 2] = verts[i].x * cosAngle - verts[i].y * sinAngle + x;
		vertices[i * 2 + 1] =  verts[i].x * sinAngle + verts[i].y * cosAngle + y;
	}
}

static inline void BuildRect(const CollisionInstanceBounds& bounds, float* vertices)
{
	vertices[0] = bounds.centerX - bounds.scrollX - bounds.hotspotX;
	vertices[1] = bounds.centerY - bounds.scrollY - bounds.hotspotY;
	vertices[2] = bounds.width;
	vertices[3] = bounds.height;
}

static inline void BuildQuadFromPolygon(const Polygon& polygon, const CollisionInstanceBounds& bounds, float sinAngle, float cosAngle, float* vertices, bool local = false)
{
	const Vector2D& min = polygon.vertices[0];
	const Vector2D& max = polygon.vertices[1];

	const Vector2D verts[4] = { { min.x, min.y }, { max.x, min.y }, { max.x, max.y}, { min.x, max.y } };

	for (int i = 0; i < 4; i++)
	{
		Vector2D pos = verts[i];
		if (!local)
			pos = ConvertVertexToWorldSpace(verts[i], bounds, sinAngle, cosAngle);

		vertices[i * 2] = pos.x;
		vertices[i * 2 + 1] = pos.y;
	}
}

static inline void BuildRectFromPolygon(const Polygon& polygon, const CollisionInstanceBounds& bounds, float* vertices)
{
	const Vector2D& min = polygon.vertices[0];
	const Vector2D& max = polygon.vertices[1];

	vertices[0] = min.x * bounds.scaleX - bounds.hotspotX + bounds.centerX - bounds.scrollX;
	vertices[1] = min.y * bounds.scaleY - bounds.hotspotY + bounds.centerY - bounds.scrollY;
	vertices[2] = (max.x - min.x) * bounds.scaleX;
	vertices[3] = (max.y - min.y) * bounds.scaleY;
}

bool Frame::IsColliding(ObjectInstance *instance1, ObjectInstance *instance2)
{
	// Check if the objects are on the same layer
	if (instance1->Layer != instance2->Layer) return false;

	CollisionInstanceBounds bounds1 = GetInstanceBounds(instance1);
	CollisionInstanceBounds bounds2 = GetInstanceBounds(instance2);

	if( bounds1.scaleX <= 0.0f || bounds1.scaleY <= 0.0f || bounds2.scaleX <= 0.0f || bounds2.scaleY <= 0.0f ) return false;
	
	if (bounds1.maxX - bounds1.scrollX <= bounds2.minX - bounds2.scrollX || bounds1.minX - bounds1.scrollX >= bounds2.maxX - bounds2.scrollX ||
		bounds1.maxY - bounds1.scrollY <= bounds2.minY - bounds2.scrollY || bounds1.minY - bounds1.scrollY >= bounds2.maxY - bounds2.scrollY)
		return false;
	
	bool useMask1 = (instance1->Type == 1 || (instance1->Type == 2 && ((Active*)instance1)->FineDetection));
	bool useMask2 = (instance2->Type == 1 || (instance2->Type == 2 && ((Active*)instance2)->FineDetection));
	const PolygonCollisionData *polygonData1 = nullptr;
	const PolygonCollisionData *polygonData2 = nullptr;
	Backend* backend = Application::Instance().GetBackend().get();
	if (useMask1)
		polygonData1 = backend->platform->GetCollisionPolygonData(bounds1.imageId);
	if (useMask2)
		polygonData2 = backend->platform->GetCollisionPolygonData(bounds2.imageId);

	useMask1 = polygonData1 != nullptr;
	useMask2 = polygonData2 != nullptr;

	float cos1 = 1.0f, sin1 = 0.0f;
	float cos2 = 1.0f, sin2 = 0.0f;
	if (bounds1.angle != 0) {
		float radians = -bounds1.angle * (PI / 180.0f);
		cos1 = cos(radians);
		sin1 = sin(radians);
	}
	if (bounds2.angle != 0) {
		float radians = -bounds2.angle * (PI / 180.0f);
		cos2 = cos(radians);
		sin2 = sin(radians);
	}
	// Skip polygon calculations if there's no rotation present
	if (bounds1.angle == 0 && bounds2.angle == 0)
	{
		// Polygon to polygon
		if(useMask1 && useMask2)
		{
			for(int i = 0; i < polygonData1->Polygons.size(); i++)
			{
				const Polygon& polygon = polygonData1->Polygons[i];
				float rect[4];
				BuildRectFromPolygon(polygon, bounds1, rect);
				for(int k = 0; k < polygonData2->Polygons.size(); k++)
				{
					const Polygon& polygon2 = polygonData2->Polygons[k];
					float rect2[4];
					BuildRectFromPolygon(polygon2, bounds2, rect2);
					//sc2d_check_rects
					if(sc2d_check_rects(rect[0],rect[1],rect[2],rect[3],rect2[0],rect2[1],rect2[2],rect2[3]))
					{
						return true;
					}
				}
			}
		}
		
		// Polygon to bbox
		else if(useMask1 && !useMask2)
		{
			float rect[4];
			BuildRect(bounds2, rect);
			for(int i = 0; i < polygonData1->Polygons.size(); i++)
			{
				const Polygon& polygon = polygonData1->Polygons[i];
				float rect2[4];
				BuildRectFromPolygon(polygon, bounds1, rect2);
				if(sc2d_check_rects(rect[0],rect[1],rect[2],rect[3],rect2[0],rect2[1],rect2[2],rect2[3]))
				{
					return true;
				}
				
			}
		}
		
		// bbox to Polygon
		else if(!useMask1 && useMask2)
		{
			float rect[4];
			BuildRect(bounds1, rect);
			for(int i = 0; i < polygonData2->Polygons.size(); i++)
			{
				const Polygon& polygon = polygonData2->Polygons[i];
				float rect2[4];
				BuildRectFromPolygon(polygon, bounds2, rect2);
				if(sc2d_check_rects(rect[0],rect[1],rect[2],rect[3],rect2[0],rect2[1],rect2[2],rect2[3]))
				{
					return true;
				}
			}
		}

		else if(!useMask1 && !useMask2)
		{
			float rect[4];
			BuildRect(bounds1, rect);
			float rect2[4];
			BuildRect(bounds2, rect2);
			return (sc2d_check_rects(rect[0],rect[1],rect[2],rect[3],rect2[0],rect2[1],rect2[2],rect2[3]));
		}
		return false;
	}
	else
	{
		// Polygon to polygon
		if(useMask1 && useMask2)
		{
			for(int i = 0; i < polygonData1->Polygons.size(); i++)
			{
				const Polygon& polygon = polygonData1->Polygons[i];
				float quad[8];
				BuildQuadFromPolygon(polygon, bounds1, sin1, cos1, quad);
				for(int k = 0; k < polygonData2->Polygons.size(); k++)
				{
					const Polygon& polygon2 = polygonData2->Polygons[k];
					float quad2[8];
					BuildQuadFromPolygon(polygon2, bounds2, sin2, cos2, quad2);
					float outX, outY;
					if(sc2d_check_poly2d(0.0f,0.0f, quad, 4, 0.0f,0.0f, quad2, 4, &outX, &outY))
					{
						return true;
					}
				}
			}
		}
		
		// Polygon to bbox
		else if(useMask1 && !useMask2)
		{
			float quad[8];
			BuildQuad(bounds2, sin2, cos2, quad);
			for(int i = 0; i < polygonData1->Polygons.size(); i++)
			{
				const Polygon& polygon = polygonData1->Polygons[i];
				float quad2[8];
				BuildQuadFromPolygon(polygon, bounds1, sin1, cos1, quad2);
				float outX, outY;
				if(sc2d_check_poly2d(0.0f,0.0f, quad2, 4, 0.0f,0.0f, quad, 4, &outX, &outY))
				{
					return true;
				}
				
			}
		}
		
		// bbox to Polygon
		else if(!useMask1 && useMask2)
		{
			float quad[8];
			BuildQuad(bounds1, sin1, cos1, quad);
			for(int i = 0; i < polygonData2->Polygons.size(); i++)
			{
				const Polygon& polygon = polygonData2->Polygons[i];
				float quad2[8];
				BuildQuadFromPolygon(polygon, bounds2, sin2, cos2, quad2);
				float outX, outY;
				if(sc2d_check_poly2d(0.0f,0.0f, quad2, 4, 0.0f,0.0f, quad, 4, &outX, &outY))
				{
					return true;
				}
			}
		}

		else if(!useMask1 && !useMask2)
		{
			float quad1[8];
			BuildQuad(bounds1, sin1, cos1, quad1);
			float quad2[8];
			BuildQuad(bounds2, sin2, cos2, quad2);
			float outX, outY;
			return (sc2d_check_poly2d(0.0f,0.0f, quad1, 4, 0.0f,0.0f, quad2, 4, &outX, &outY));
		}
		
			
	}
	
	return false;
}


bool Frame::IsColliding(ObjectInstance *instance, int x, int y)
{
	x -= scrollX;
	y -= scrollY;

	CollisionInstanceBounds bounds = GetInstanceBounds(instance);
	if (x < bounds.minX - bounds.scrollX || x > bounds.maxX - bounds.scrollX || y < bounds.minY - bounds.scrollY || y > bounds.maxY - bounds.scrollY)
		return false;

	if (bounds.scaleX <= 0.0f || bounds.scaleY <= 0.0f)
		return false;

	unsigned int imageId = bounds.imageId;
	bool fineDetection = false;

	const PolygonCollisionData *polygonData1 = nullptr;

	fineDetection = (instance->Type == 1 || (instance->Type == 2 && ((Active*)instance)->FineDetection));

	Backend* backend = Application::Instance().GetBackend().get();
	if (fineDetection)
		polygonData1 = backend->platform->GetCollisionPolygonData(bounds.imageId);

	fineDetection = polygonData1 != nullptr;

	float cos1 = 1.0f, sin1 = 0.0f;
	if (bounds.angle != 0) {
		float radians = -bounds.angle * (PI / 180.0f);
		cos1 = cos(radians);
		sin1 = sin(radians);
	}

	// Polygon
	if(fineDetection)
	{
		Vector2D Position = ConvertVertexToLocalSpace(Vector2D{static_cast<float>(x),static_cast<float>(y)}, bounds, sin1, cos1);
		for(int i = 0; i < polygonData1->Polygons.size(); i++)
		{
			const Polygon& polygon = polygonData1->Polygons[i];

			const Vector2D& min = polygon.vertices[0];
			const Vector2D& max = polygon.vertices[1];

			if (Position.x >= min.x && Position.x < max.x && Position.y >= min.y && Position.y < max.y) return true;
		}
	}
	else
	{
		if (bounds.angle == 0)
		{
			float rect[4];
			BuildRect(bounds, rect);

			return sc2d_check_point_rect(x, y, rect[0], rect[1], rect[2], rect[3]);
		}

		float quad[8];
		BuildQuad(bounds, sin1, cos1, quad);

		return sc2d_check_point_poly2d(x, y, quad, 4);
	}
	return false;
}