#include "Application.h"
#include "FrameFactory.h"
#include "Frame.h"
#include <set>

#ifdef NUCLEAR_BACKEND_SDL3
#include "SDL3Backend.h"
#elif defined(NUCLEAR_BACKEND_SDL2)
#include "SDL2Backend.h"
#endif

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif

Application::Application() = default;
Application::~Application() = default;

void Application::Initialize()
{
	appData = std::make_shared<AppData>();
	appData->Initialize();
	std::cout << "Initialized AppData" << std::endl;

	#ifdef NUCLEAR_BACKEND_SDL3
		backend = std::make_shared<SDL3Backend>();
	#elif NUCLEAR_BACKEND_SDL2
		backend = std::make_shared<SDL2Backend>();
	#else
		backend = std::make_shared<Backend>();
	#endif

	backend->platform->Log("Initialized Backend: " + backend->platform->GetName());

	input = std::make_shared<Input>();
	input->Reset();
	backend->platform->Log("Initialized Input");

	QueueStateChange(GameState::RestartApplication);
	
	srand(time(0));
}

void Application::Update()
{
	RunState();

	if (currentFrame == nullptr)
	{
		return;
	}

	input->Update();
	backend->audio->UpdateSample();
	currentFrame->Update();
}

void Application::Draw()
{
	if (currentFrame == nullptr)
	{
		return;
	}

	backend->graphics->BeginDrawing();
	currentFrame->Draw();
	backend->graphics->EndDrawing();
}

void Application::Shutdown()
{
	backend->platform->Log("Shutting down...");
}

static void Loop()
{
	Application::Instance().Update();
	Application::Instance().Draw();
}

void Application::Run()
{
#ifdef PLATFORM_WEB
	emscripten_set_main_loop(Loop, appData->GetTargetFPS(), 1);
#else
	unsigned int frameStart;
	int frameTime;
	
	while (!backend->platform->ShouldQuit())
	{
		int targetFPS = appData->GetTargetFPS();
		float targetFrameTime = 1000.0f / targetFPS;
		
		frameStart = backend->platform->GetTicks();
		
		Loop();
		
		frameTime = backend->platform->GetTicks() - frameStart;
		
		if (frameTime < targetFrameTime)
		{
			backend->platform->Delay(targetFrameTime - frameTime);
		}

		if (currentState == GameState::EndApplication || currentFrame == nullptr)
		{
			break;
		}
		#ifdef PLATFORM_3DS
			SDL_Delay(1);
		#endif
	}
#endif

	Shutdown();
}

void Application::QueueStateChange(GameState newState, int frameIndex)
{
	if (newState == GameState::JumpToFrame) 
	{
		if (frameIndex < 0)
		{
			return;
		}
		else if (frameIndex >= FrameFactory::GetFrameCount())
		{
			frameIndex = FrameFactory::GetFrameCount() - 1;
		}
	}

	currentState = newState;
	newFrameIndex = frameIndex;
}

short Application::Random(short max)
{
	return rand() % (max + 1);
}

short Application::RandomRange(short min, short max)
{
	return rand() % (max - min + 1) + min;
}

void Application::LoadFrame(int frameIndex)
{
	backend->platform->Log("LoadFrame: entered with frameIndex=" + std::to_string(frameIndex));

	if (frameIndex < 0)
	{
		backend->platform->Log("LoadFrame: frameIndex < 0, clamping to 0");
		frameIndex = 0;
	}

	backend->platform->Log("LoadFrame: Loading frame " + std::to_string(frameIndex));

	std::vector<unsigned int> oldImagesUsed;
	std::vector<unsigned int> oldFontsUsed;

	backend->platform->Log("LoadFrame: checking currentFrame");

	if (currentFrame != nullptr)
	{
		backend->platform->Log("LoadFrame: currentFrame exists, collecting old resources");

		oldImagesUsed = currentFrame->GetImagesUsed();
		backend->platform->Log("LoadFrame: oldImagesUsed count=" + std::to_string(oldImagesUsed.size()));

		oldFontsUsed = currentFrame->GetFontsUsed();
		backend->platform->Log("LoadFrame: oldFontsUsed count=" + std::to_string(oldFontsUsed.size()));

		backend->platform->Log("LoadFrame: getting global object data from current frame");
		std::vector<ObjectGlobalData*> frameData = currentFrame->GetGlobalObjectData();

		backend->platform->Log("LoadFrame: frame global object data count=" + std::to_string(frameData.size()));

		backend->platform->Log("LoadFrame: merging global object data");
		MergeGlobalObjectData(frameData);
		backend->platform->Log("LoadFrame: finished merging global object data");
	}
	else
	{
		backend->platform->Log("LoadFrame: no currentFrame, first frame load");
	}

	backend->platform->Log("LoadFrame: before FrameFactory::CreateFrame");
	currentFrame = FrameFactory::CreateFrame(frameIndex);
	backend->platform->Log("LoadFrame: after FrameFactory::CreateFrame");

	if (currentFrame == nullptr)
	{
		backend->platform->Log("LoadFrame ERROR: FrameFactory::CreateFrame returned nullptr");
		QueueStateChange(GameState::EndApplication);
		backend->platform->Log("LoadFrame: queued EndApplication");
		return;
	}

	backend->platform->Log("LoadFrame: currentFrame created successfully");

	backend->platform->Log("LoadFrame: before currentFrame->Initialize()");
	currentFrame->Initialize();
	backend->platform->Log("LoadFrame: after currentFrame->Initialize()");

	backend->platform->Log("LoadFrame: globalObjectData count=" + std::to_string(globalObjectData.size()));

	if (!globalObjectData.empty())
	{
		backend->platform->Log("LoadFrame: before ApplyGlobalObjectData");
		currentFrame->ApplyGlobalObjectData(globalObjectData);
		backend->platform->Log("LoadFrame: after ApplyGlobalObjectData");
	}
	else
	{
		backend->platform->Log("LoadFrame: no global object data to apply");
	}

	backend->platform->Log("LoadFrame: before ShowMouseCursor");
	backend->input->ShowMouseCursor();
	backend->platform->Log("LoadFrame: after ShowMouseCursor");

	backend->platform->Log("LoadFrame: getting new images used");
	std::vector<unsigned int> newImagesUsed = currentFrame->GetImagesUsed();
	backend->platform->Log("LoadFrame: newImagesUsed count=" + std::to_string(newImagesUsed.size()));

	for (size_t i = 0; i < newImagesUsed.size(); ++i)
	{
		backend->platform->Log("LoadFrame: newImagesUsed[" + std::to_string(i) + "]=" + std::to_string(newImagesUsed[i]));
	}

	backend->platform->Log("LoadFrame: getting new fonts used");
	std::vector<unsigned int> newFontsUsed = currentFrame->GetFontsUsed();
	backend->platform->Log("LoadFrame: newFontsUsed count=" + std::to_string(newFontsUsed.size()));

	for (size_t i = 0; i < newFontsUsed.size(); ++i)
	{
		backend->platform->Log("LoadFrame: newFontsUsed[" + std::to_string(i) + "]=" + std::to_string(newFontsUsed[i]));
	}

	std::vector<unsigned int> imagesToUnload;
	std::vector<unsigned int> fontsToUnload;

	backend->platform->Log("LoadFrame: calculating imagesToUnload");

	for (unsigned int image : oldImagesUsed)
	{
		backend->platform->Log("LoadFrame: checking old image " + std::to_string(image));

		if (std::find(newImagesUsed.begin(), newImagesUsed.end(), image) == newImagesUsed.end())
		{
			backend->platform->Log("LoadFrame: image will be unloaded " + std::to_string(image));
			imagesToUnload.push_back(image);
		}
	}

	backend->platform->Log("LoadFrame: imagesToUnload count=" + std::to_string(imagesToUnload.size()));

	backend->platform->Log("LoadFrame: calculating fontsToUnload");

	for (unsigned int font : oldFontsUsed)
	{
		backend->platform->Log("LoadFrame: checking old font " + std::to_string(font));

		if (std::find(newFontsUsed.begin(), newFontsUsed.end(), font) == newFontsUsed.end())
		{
			backend->platform->Log("LoadFrame: font will be unloaded " + std::to_string(font));
			fontsToUnload.push_back(font);
		}
	}

	backend->platform->Log("LoadFrame: fontsToUnload count=" + std::to_string(fontsToUnload.size()));

	backend->platform->Log("LoadFrame: unloading old images");

	for (unsigned int image : imagesToUnload)
	{
		backend->platform->Log("LoadFrame: before UnloadTexture image=" + std::to_string(image));
		backend->graphics->UnloadTexture(image);
		backend->platform->Log("LoadFrame: after UnloadTexture image=" + std::to_string(image));
	}

	backend->platform->Log("LoadFrame: loading new images");

	for (unsigned int image : newImagesUsed)
	{
		backend->platform->Log("LoadFrame: before LoadTexture image=" + std::to_string(image));
		backend->graphics->LoadTexture(image);
		backend->platform->Log("LoadFrame: after LoadTexture image=" + std::to_string(image));
	}

	backend->platform->Log("LoadFrame: loading new fonts");

	for (unsigned int font : newFontsUsed)
	{
		backend->platform->Log("LoadFrame: before LoadFont font=" + std::to_string(font));
		backend->graphics->LoadFont(font);
		backend->platform->Log("LoadFrame: after LoadFont font=" + std::to_string(font));
	}

	backend->platform->Log("LoadFrame: unloading old fonts");

	for (unsigned int font : fontsToUnload)
	{
		backend->platform->Log("LoadFrame: before UnloadFont font=" + std::to_string(font));
		backend->graphics->UnloadFont(font);
		backend->platform->Log("LoadFrame: after UnloadFont font=" + std::to_string(font));
	}

	backend->platform->Log("LoadFrame: Loaded frame " + std::to_string(frameIndex));

	backend->platform->Log("LoadFrame: checking SampleOverFrame");
	bool sampleOverFrame = GetAppData()->GetSampleOverFrame();
	backend->platform->Log("LoadFrame: SampleOverFrame=" + std::to_string(sampleOverFrame ? 1 : 0));

	if (!sampleOverFrame)
	{
		backend->platform->Log("LoadFrame: before StopSample(-1, false)");
		backend->audio->StopSample(-1, false);
		backend->platform->Log("LoadFrame: after StopSample(-1, false)");
	}

	backend->platform->Log("LoadFrame: finished");
}

void Application::MergeGlobalObjectData(std::vector<ObjectGlobalData*> frameData)
{
	std::map<unsigned int, std::vector<ObjectGlobalData*>> existingByHandle;
	for (auto* data : globalObjectData)
	{
		existingByHandle[data->objectInfoHandle].push_back(data);
	}
	
	std::map<unsigned int, std::vector<ObjectGlobalData*>> frameByHandle;
	for (auto* data : frameData)
	{
		frameByHandle[data->objectInfoHandle].push_back(data);
	}
	
	globalObjectData.clear();
	
	std::set<unsigned int> allHandles;
	for (auto& [handle, dataList] : existingByHandle)
	{
		allHandles.insert(handle);
	}
	for (auto& [handle, dataList] : frameByHandle)
	{
		allHandles.insert(handle);
	}
	
	for (unsigned int handle : allHandles)
	{
		bool hasExisting = existingByHandle.find(handle) != existingByHandle.end();
		bool hasFrame = frameByHandle.find(handle) != frameByHandle.end();
		
		if (hasFrame)
		{
			int frameCount = frameByHandle[handle].size();
			
			if (hasExisting)
			{
				int savedCount = existingByHandle[handle].size();
				
				if (frameCount <= savedCount)
				{
					auto& savedList = existingByHandle[handle];
					auto& frameList = frameByHandle[handle];
					
					for (int i = 0; i < savedCount - frameCount; i++)
					{
						globalObjectData.push_back(savedList[i]);
					}
					for (int i = 0; i < frameCount; i++)
					{
						globalObjectData.push_back(frameList[i]);
					}
				}
				else
				{
					for (auto* data : frameByHandle[handle])
					{
						globalObjectData.push_back(data);
					}
				}
			}
			else
			{
				for (auto* data : frameByHandle[handle])
				{
					globalObjectData.push_back(data);
				}
			}
		}
		else
		{
			for (auto* data : existingByHandle[handle])
			{
				globalObjectData.push_back(data);
			}
		}
	}
}

void Application::RunState()
{
	switch (currentState)
	{
		case GameState::Running:
			break;
		case GameState::RestartApplication:
			globalObjectData.clear();
			LoadFrame(0);
			currentState = GameState::StartOfFrame;
			break;
		case GameState::StartOfFrame:
			currentState = GameState::Running;
			break;
		case GameState::NextFrame:
			LoadFrame(currentFrame->Index + 1);
			currentState = GameState::StartOfFrame;
			break;
		case GameState::PreviousFrame:
			LoadFrame(currentFrame->Index - 1);
			currentState = GameState::StartOfFrame;
			break;
		case GameState::JumpToFrame:
			LoadFrame(newFrameIndex);
			currentState = GameState::StartOfFrame;
			break;
		case GameState::RestartFrame:
			LoadFrame(currentFrame->Index);
			currentState = GameState::StartOfFrame;
			break;
		case GameState::EndApplication:
			break;
	}
}
