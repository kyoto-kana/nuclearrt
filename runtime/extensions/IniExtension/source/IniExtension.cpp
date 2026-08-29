#include "IniExtension.h"
#include "Application.h"
#include <memory>
#include <string>
#include <cstdlib>

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <shlobj.h>
#include <KnownFolders.h>
#elif defined(PLATFORM_IOS) && defined(NUCLEAR_BACKEND_SDL3)
#include <SDL3/SDL_filesystem.h>
#endif

void IniExtension::Initialize()
{
	CurrentGroup = "";
	CurrentItem = "";

	SetFileName(Name);
}	

void IniExtension::SetFileName(CValue name)
{
	Name = name.GetStringValue();
	
	std::filesystem::path path = GetPlatformSaveDirectory();
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directories(path);
	}

	path /= Name;

	iniFile = std::make_unique<mINI::INIFile>(path);
	iniFile->read(ini);
}

void IniExtension::SetCurrentGroup(CValue group)
{
	CurrentGroup = group.GetStringValue();
}

void IniExtension::SetCurrentItem(CValue item)
{
	CurrentItem = item.GetStringValue();
}

void IniExtension::SetValue(CValue value)
{
	switch (value.GetType())
	{
		case 0:
			ini[CurrentGroup][CurrentItem] = std::to_string(value.GetIntValue());
			break;
		case 1:
			ini[CurrentGroup][CurrentItem] = std::to_string(value.GetDoubleValue());
			break;
	}
	iniFile->write(ini);
}

void IniExtension::SetValue(CValue item, CValue value)
{
	switch (value.GetType())
	{
		case 0:
			ini[CurrentGroup][item.GetStringValue()] = std::to_string(value.GetIntValue());
			break;
		case 1:
			ini[CurrentGroup][item.GetStringValue()] = std::to_string(value.GetDoubleValue());
			break;
	}
	iniFile->write(ini);
}

void IniExtension::SetValue(CValue group, CValue item, CValue value)
{
	switch (value.GetType())
	{
		case 0:
			ini[group.GetStringValue()][item.GetStringValue()] = std::to_string(value.GetIntValue());
			break;
		case 1:
			ini[group.GetStringValue()][item.GetStringValue()] = std::to_string(value.GetDoubleValue());
			break;
	}
	iniFile->write(ini);
}

void IniExtension::SetString(CValue value)
{
	if (value.GetType() == 2) 
		ini[CurrentGroup][CurrentItem] = value.GetStringValue();
	iniFile->write(ini);
}

void IniExtension::SetString(CValue item, CValue value)
{
	if (value.GetType() == 2) 
		ini[CurrentGroup][item.GetStringValue()] = value.GetStringValue();
	iniFile->write(ini);
}

void IniExtension::SetString(CValue group, CValue item, CValue value)
{
	if (value.GetType() == 2) 
		ini[group.GetStringValue()][item.GetStringValue()] = value.GetStringValue();
	iniFile->write(ini);
}

void IniExtension::SavePosition(ObjectInstance* object)
{
	std::string item = "pos." + object->Name;
	ini[CurrentGroup].set(item, std::to_string(object->GetX().GetIntValue()) + "," + std::to_string(object->GetY().GetIntValue()));
	iniFile->write(ini);
}

void IniExtension::LoadPosition(ObjectInstance* object)
{
	std::string item = "pos." + object->Name;
	std::string value = ini[CurrentGroup][item];
	if (value.empty())
	{
		return;
	}

	std::string xValue = value.substr(0, value.find(','));
	std::string yValue = value.substr(value.find(',') + 1);

	object->SetX(CValue(std::stoi(xValue)));
	object->SetY(CValue(std::stoi(yValue)));
}

CValue IniExtension::GetValue()
{
	std::string value = ini[CurrentGroup][CurrentItem];
	return value.empty() ? CValue(0) : CValue(std::stoi(value));
}

CValue IniExtension::GetValue(CValue item)
{
	std::string value = ini[CurrentGroup][item.GetStringValue()];
	return value.empty() ? CValue(0) : CValue(std::stoi(value));
}

CValue IniExtension::GetValue(CValue group, CValue item)
{
	std::string value = ini[group.GetStringValue()][item.GetStringValue()];
	return value.empty() ? CValue(0) : CValue(std::stoi(value));
}

CValue IniExtension::GetString()
{
	return CValue(ini[CurrentGroup][CurrentItem]);
}

CValue IniExtension::GetString(CValue item)
{
	return CValue(ini[CurrentGroup][item.GetStringValue()]);
}

CValue IniExtension::GetString(CValue group, CValue item)
{
	return CValue(ini[group.GetStringValue()][item.GetStringValue()]);
}

void IniExtension::DeleteGroup(CValue group)
{
	ini.remove(group.GetStringValue());
	iniFile->write(ini);
}

void IniExtension::DeleteItem(CValue item)
{
	ini[CurrentGroup].remove(item.GetStringValue());
	iniFile->write(ini);
}

void IniExtension::DeleteItem(CValue group, CValue item)
{
	ini[group.GetStringValue()].remove(item.GetStringValue());
	iniFile->write(ini);
}

std::filesystem::path IniExtension::GetPlatformSaveDirectory()
{
#if defined(PLATFORM_WINDOWS)
	PWSTR path_tmp = nullptr;
	HRESULT hres = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path_tmp);

	if (SUCCEEDED(hres))
	{
		std::wstring appdata_path_w(path_tmp);
		CoTaskMemFree(path_tmp);
		return std::filesystem::path(appdata_path_w) / "NuclearApplications";
    }
	else
	{
		return std::filesystem::path();
	}
#elif defined(PLATFORM_MACOS)
	const char* home = std::getenv("HOME");
	if (home)
	{
		return std::filesystem::path(home) / "Library" / "Application Support" / "NuclearApplications";
	}
	return std::filesystem::path();
#elif defined(PLATFORM_LINUX)
	const char* xdg_data_home = std::getenv("XDG_DATA_HOME");
	if (xdg_data_home)
	{
		return std::filesystem::path(xdg_data_home) / "NuclearApplications";
	}
	
	const char* home = std::getenv("HOME");
	if (home)
	{
		return std::filesystem::path(home) / ".local" / "share" / "NuclearApplications";
	}
	return std::filesystem::path();
#elif defined(PLATFORM_WEB)
	return std::filesystem::path("/disk/AppData/Roaming/NuclearApplications");
#elif defined(PLATFORM_PSP)
	return std::filesystem::path("ms0:/PSP/NuclearApplications");
#elif defined(PLATFORM_VITA)
	return std::filesystem::path("ux0:/data/NuclearApplications");
#elif defined(PLATFORM_WIIU)
	return std::filesystem::path("/vol/external01/wiiu/NuclearApplications");
#elif defined(PLATFORM_3DS)
	return std::filesystem::path("sdmc:/3ds/NuclearApplications");
#elif defined(PLATFORM_IOS)
	return std::filesystem::path(SDL_GetPrefPath("NuclearApplications", "saves"));
#else
	return std::filesystem::path();
#endif
}