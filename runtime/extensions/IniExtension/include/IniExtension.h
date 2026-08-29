#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"
#include <string>
#include <filesystem>
#include "mini/ini.h"
	
class IniExtension : public Extension {
public:
	IniExtension(unsigned int objectInfoHandle, int type, std::string name, int flags, std::string fileName)
		: Extension(objectInfoHandle, type, name), Flags(flags), Name(fileName) {}

	void Initialize() override;

	void SetFileName(CValue name);

	void SetCurrentGroup(CValue group);
	void SetCurrentItem(CValue item);

	void SetValue(CValue value);
	void SetValue(CValue item, CValue value);
	void SetValue(CValue group, CValue item, CValue value);

	void SetString(CValue value);
	void SetString(CValue item, CValue value);
	void SetString(CValue group, CValue item, CValue value);

	void SavePosition(ObjectInstance* object);
	void LoadPosition(ObjectInstance* object);

	CValue GetValue();
	CValue GetValue(CValue item);
	CValue GetValue(CValue group, CValue item);

	CValue GetString();
	CValue GetString(CValue item);
	CValue GetString(CValue group, CValue item);
	
	void DeleteGroup(CValue group);
	void DeleteItem(CValue group, CValue item);
	void DeleteItem(CValue item);
private:
	int Flags;
	std::string Name;

	std::string CurrentGroup;
	std::string CurrentItem;

	std::filesystem::path GetPlatformSaveDirectory();

	std::unique_ptr<mINI::INIFile> iniFile;
	mINI::INIStructure ini;
};