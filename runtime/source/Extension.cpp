#include "Extension.h"

#include "Application.h"
#include "Frame.h"

void Extension::GenerateEvent(int conditionId)
{
	auto& frame = Application::Instance().GetCurrentFrame();
	if (frame)
	{
		frame->GenerateEvent(static_cast<int>(Type), -(80 + conditionId), this);
	}
}
