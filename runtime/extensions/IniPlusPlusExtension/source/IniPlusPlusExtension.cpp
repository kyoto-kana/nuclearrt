#include "IniPlusPlusExtension.h"
#include "Application.h"
#include <memory>
#include <string>
#include <cstdlib>

std::unordered_map<std::string, INIInstance*> IniPlusPlusExtension::globalDataMap;

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <shlobj.h>
#include <KnownFolders.h>
#endif

void IniPlusPlusExtension::Initialize()
{
	if (GlobalData)
	{
		auto it = globalDataMap.find(GlobalDataKey);
		if (it != globalDataMap.end())
		{
			currentData = it->second;
		}
		else
		{
			currentData = new INIInstance();
			globalDataMap[GlobalDataKey] = currentData;
		}
	}

	if (DefaultFile) {
		std::filesystem::path defaultPath(DefaultFilePath);
		std::string filename = defaultPath.filename().string();
		std::string subDir = defaultPath.has_parent_path() ? defaultPath.parent_path().string() : std::string();

		std::filesystem::path saveDir = GetBaseSaveDirectory(BaseFolder, subDir);
		if (!saveDir.empty() && !std::filesystem::exists(saveDir) && CreateFolders) {
			std::filesystem::create_directories(saveDir);
		}

		currentData = new INIInstance();
		currentData->FilePath = (saveDir / filename).string();
		currentData->ReadOnly = ReadOnly;
		currentData->Encrypted = UseEncryption;
		currentData->Compressed = UseCompression;
		currentData->EncryptionKey = EncryptionKey;
		currentData->NewLineText = NewLine ? NewLineText : "\n";
		currentData->CaseSensitive = CaseSensitive;
		currentData->UndoBufferSize = UndoBufferSize;
		currentData->RedoBufferSize = RedoBufferSize;

		if (!InitialData.empty()) {
			currentData->ReadIniData(InitialData);
		}
		
		currentData->LoadIniFromPath(currentData->FilePath);
	}
}

void IniPlusPlusExtension::Update(float deltaTime)
{
	if (EnableAutoSave && currentData && currentData->dirty) {
		currentData->SaveINI();
	}
}

void IniPlusPlusExtension::SetCurrentGroup(CValue group)
{
	CurrentGroup = group.GetStringValue();
}

void IniPlusPlusExtension::SetCurrentGroupItemValue(CValue item, CValue type, CValue value)
{
	SetItemValue(CurrentGroup, item, type, value);
}

void IniPlusPlusExtension::SetCurrentGroupItemString(CValue item, CValue value)
{
	SetItemString(CurrentGroup, item, value);
}

void IniPlusPlusExtension::SetItemValue(CValue group, CValue item, CValue type, CValue value)
{
	if (!currentData) return;
	
	if (type == 0) // int
		currentData->SetValue(group.GetStringValue(), item.GetStringValue(), std::to_string(value.GetIntValue()));
	else if (type == 1) // double
		currentData->SetValue(group.GetStringValue(), item.GetStringValue(), std::to_string(value.GetDoubleValue()));
}

void IniPlusPlusExtension::SetItemString(CValue group, CValue item, CValue value)
{
	if (!currentData) return;
	
	currentData->SetValue(group.GetStringValue(), item.GetStringValue(), value.GetStringValue());
}

void IniPlusPlusExtension::DeleteCurrentGroup()
{
	DeleteGroup(CurrentGroup);
}

void IniPlusPlusExtension::DeleteGroup(CValue group)
{
	if (!currentData) return;
	currentData->DeleteGroup(group.GetStringValue());
}

void IniPlusPlusExtension::DeleteCurrentGroupItem(CValue item)
{
	DeleteGroupItem(CurrentGroup, item);
}

void IniPlusPlusExtension::DeleteGroupItem(CValue group, CValue item)
{
	if (!currentData) return;
	currentData->DeleteGroupItem(group.GetStringValue(), item.GetStringValue());
}

void IniPlusPlusExtension::ClearINI()
{
	if (!currentData) return;
	currentData->Clear();
}

void IniPlusPlusExtension::Save()
{
	if (currentData) currentData->SaveINI();
}

bool IniPlusPlusExtension::CurrentGroupExists() const
{
	return GroupExists(CurrentGroup);
}

bool IniPlusPlusExtension::GroupExists(CValue group) const
{
	if (!currentData) return false;
	return currentData->GroupExists(group.GetStringValue());
}

bool IniPlusPlusExtension::CurrentGroupItemExists(CValue item) const
{
	return GroupItemExists(CurrentGroup, item);
}

bool IniPlusPlusExtension::GroupItemExists(CValue group, CValue item) const
{
	if (!currentData) return false;
	return currentData->GroupItemExists(group.GetStringValue(), item.GetStringValue());
}

CValue IniPlusPlusExtension::GetCurrentGroupItemValue(CValue item, CValue defaultValue)
{
	return GetItemValue(CurrentGroup, item, CValue(defaultValue.GetIntValue()));
}

CValue IniPlusPlusExtension::GetCurrentGroupItemString(CValue item, CValue defaultValue)
{
	return GetItemString(CurrentGroup, item, CValue(defaultValue.GetStringValue()));
}

CValue IniPlusPlusExtension::GetItemValue(CValue group, CValue item, CValue defaultValue)
{
	if (!currentData) return defaultValue;

	std::string valueStr = currentData->GetValue(group.GetStringValue(), item.GetStringValue(), std::to_string(defaultValue.GetIntValue()));
	return CValue(std::stoi(valueStr));
}

CValue IniPlusPlusExtension::GetItemString(CValue group, CValue item, CValue defaultValue)
{
	if (!currentData) return defaultValue;

	return CValue(currentData->GetValue(group.GetStringValue(), item.GetStringValue(), defaultValue.GetStringValue()));
}


std::filesystem::path IniPlusPlusExtension::GetBaseSaveDirectory(int8_t baseFolder, const std::string &defaultFilePath)
{
#if defined (PLATFORM_WEB)
	return std::filesystem::path("/disk/") / defaultFilePath;
#else
	return std::filesystem::path(defaultFilePath);
#endif
}