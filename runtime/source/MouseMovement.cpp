#include "MouseMovement.h"

#include <algorithm>

#include "Application.h"
#include "InputBackend.h"

void MouseMovement::Initialize() {
	initialX = Instance->GetX().GetIntValue();
	initialY = Instance->GetY().GetIntValue();
}

void MouseMovement::OnEnabled() {
	Application::Instance().GetBackend()->input->HideMouseCursor();

	disabledCursorX = Application::Instance().GetBackend()->input->GetMouseX();
	disabledCursorY = Application::Instance().GetBackend()->input->GetMouseY();
}

void MouseMovement::OnDisabled() {
	Application::Instance().GetBackend()->input->ShowMouseCursor();
}

void MouseMovement::Update(float deltaTime) {
	int mouseX = Application::Instance().GetInput()->GetMouseX();
	int mouseY = Application::Instance().GetInput()->GetMouseY();

	int xDifference = mouseX - disabledCursorX;
	int yDifference = mouseY - disabledCursorY;
	
	Application::Instance().GetBackend()->input->SetMouseX(disabledCursorX);
	Application::Instance().GetBackend()->input->SetMouseY(disabledCursorY);

	Instance->SetX(CValue(Instance->GetX().GetIntValue() + xDifference));
	Instance->SetY(CValue(Instance->GetY().GetIntValue() + yDifference));

	Instance->SetX(CValue(std::clamp(Instance->GetX().GetIntValue() - initialX, MinX, MaxX) + initialX));
	Instance->SetY(CValue(std::clamp(Instance->GetY().GetIntValue() - initialY, MinY, MaxY) + initialY));
}