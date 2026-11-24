#pragma once

#include "Application.h"
#include "Extension.h"
#include "ObjectSelector.h"
#include <string>
#include <vector>

class ListExtension : public Extension
{
public:
	ListExtension(unsigned int objectInfoHandle, int type, std::string name, int flags, int indexOffset, std::vector<std::string> lines)
		: Extension(objectInfoHandle, type, name), Flags(flags), IndexOffset(indexOffset), Strings(lines) {}

    //A
	void Load(CValue value);
	void LoadDrives(CValue value);
	void LoadDirectory(CValue value);
	void LoadFiles(CValue value);
	void Save(CValue value);
	void Reset() { Strings.clear(); Datas.clear(); };
	void SetLineData(CValue value, CValue value2);
	void ChangeLine(CValue value, CValue value2);
	void AddLine(CValue value);
	void InsertLine(CValue value, CValue value2);
	void DeleteLine(CValue value);
    //C
    //E
	CValue GetLastIndex() { return CValue(Strings.size() - 1); };
	CValue GetNbLine() { return CValue(Strings.size()); };
	
	CValue GetLineText(CValue index);
	CValue FindString(CValue search, CValue index);
	CValue FindStringExact(CValue search, CValue index);
	CValue GetLineData(CValue index);
private:
    int Flags = 0;
    int IndexOffset = 0;
    short LineNumbers = 0;
    std::vector<std::string> Strings;
    std::vector<int> Datas;
	
	std::string WriteList();

	int FindStringInt(std::string search, int startIndex);
	int FindStringExactInt(std::string search, int startIndex);
};