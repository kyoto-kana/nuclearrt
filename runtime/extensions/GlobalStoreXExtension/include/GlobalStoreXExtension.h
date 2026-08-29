#pragma once

#include <cstdint>

#include "Application.h"
#include "Extension.h"
#include "ObjectInstance.h"

struct StoreData {
	uint32_t integerCount;
	uint8_t integerBase;
	std::vector<int> integers;

	uint32_t boolCount;
	uint8_t boolBase;
	std::vector<bool> bools;

	uint32_t shortCount;
	uint8_t shortBase;
	std::vector<short> shorts;

	uint32_t stringCount;
	uint8_t stringBase;
	std::vector<std::string> strings;
};

class GlobalStoreXExtension : public Extension {
public:
	GlobalStoreXExtension(unsigned int objectInfoHandle, int type, std::string name, int integerCount, int stringCount, int boolCount, int shortCount, unsigned char flags)
		: Extension(objectInfoHandle, type, name) {
		if (!initialized) {
			GlobalStoreXExtension::data.integerCount = integerCount;
			GlobalStoreXExtension::data.stringCount = stringCount;
			GlobalStoreXExtension::data.boolCount = boolCount;
			GlobalStoreXExtension::data.shortCount = shortCount;
			GlobalStoreXExtension::data.integerBase = flags & 0x01 ? 0 : 1;
			GlobalStoreXExtension::data.stringBase = flags & 0x02 ? 0 : 1;
			GlobalStoreXExtension::data.boolBase = flags & 0x04 ? 0 : 1;
			GlobalStoreXExtension::data.shortBase = flags & 0x08 ? 0 : 1;
		}
	}

	void Initialize() override;

	void SetInteger(CValue index, CValue value);
	void SetString(CValue index, CValue value);
	void SetBool(CValue index, CValue value);
	void SetShort(CValue index, CValue value);

	void AddInteger(CValue index, CValue value);
	void AddShort(CValue index, CValue value);
	void AddString(CValue index, CValue value);

	void SubtractInteger(CValue index, CValue value);
	void SubtractShort(CValue index, CValue value);

	void SetRangeInteger(CValue startIndex, CValue endIndex, CValue value);
	void SetRangeString(CValue startIndex, CValue endIndex, CValue value);
	void SetRangeBool(CValue startIndex, CValue endIndex, CValue value);
	void SetRangeShort(CValue startIndex, CValue endIndex, CValue value);

	void ClearIntegers();
	void ClearStrings();
	void ClearBools();
	void ClearShorts();

	void ResizeIntegers(CValue size);
	void ResizeStrings(CValue size);
	void ResizeBools(CValue size);
	void ResizeShorts(CValue size);

	void ExpandIntegers(CValue size);
	void ExpandStrings(CValue size);
	void ExpandBools(CValue size);
	void ExpandShorts(CValue size);

	CValue GetInteger(CValue index);
	CValue GetString(CValue index);
	CValue GetBool(CValue index);
	CValue GetShort(CValue index);

	unsigned char GetIntegerBase() const { return GlobalStoreXExtension::data.integerBase; }
	unsigned char GetStringBase() const { return GlobalStoreXExtension::data.stringBase; }
	unsigned char GetBoolBase() const { return GlobalStoreXExtension::data.boolBase; }
	unsigned char GetShortBase() const { return GlobalStoreXExtension::data.shortBase; }

	int GetIntegerArraySize() const { return GlobalStoreXExtension::data.integerCount; }
	int GetStringArraySize() const { return GlobalStoreXExtension::data.stringCount; }
	int GetBoolArraySize() const { return GlobalStoreXExtension::data.boolCount; }
	int GetShortArraySize() const { return GlobalStoreXExtension::data.shortCount; }

	void SaveAllINI(CValue fileName, CValue group);
	void LoadAllINI(CValue fileName, CValue group);

	void SaveIntegerINI(CValue fileName, CValue group);
	void LoadIntegerINI(CValue fileName, CValue group);
	void SaveStringINI(CValue fileName, CValue group);
	void LoadStringINI(CValue fileName, CValue group);
	void SaveBoolINI(CValue fileName, CValue group);
	void LoadBoolINI(CValue fileName, CValue group);
	void SaveShortINI(CValue fileName); // for some reason shorts can't be saved to a specific group with ini - shishkabob
	void LoadShortINI(CValue fileName);

	void SaveAllBinary(CValue fileName);
	void LoadAllBinary(CValue fileName);

	void SaveIntegerBinary(CValue fileName);
	void LoadIntegerBinary(CValue fileName);
	void SaveStringBinary(CValue fileName);
	void LoadStringBinary(CValue fileName);
	void SaveBoolBinary(CValue fileName);
	void LoadBoolBinary(CValue fileName);
	void SaveShortBinary(CValue fileName);
	void LoadShortBinary(CValue fileName);

private:
	static std::string GetNormalizedPath(const std::string& path);
	static StoreData data;

	static bool initialized;
};
