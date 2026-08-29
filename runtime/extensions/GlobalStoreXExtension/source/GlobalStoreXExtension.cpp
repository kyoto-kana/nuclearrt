#include "GlobalStoreXExtension.h"

#include <filesystem>
#include <iostream>

#include "BinaryTools/BinaryReader.h"
#include "BinaryTools/BinaryWriter.h"
#include "mini/ini.h"

bool GlobalStoreXExtension::initialized = false;
StoreData GlobalStoreXExtension::data;

void GlobalStoreXExtension::Initialize()
{
    if (initialized) return;

    data.integers.resize(data.integerCount);
    data.strings.resize(data.stringCount);
    data.bools.resize(data.boolCount);
    data.shorts.resize(data.shortCount);

    initialized = true;
}

void GlobalStoreXExtension::SetInteger(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.integerBase;
    if (indexValue < 0 || indexValue >= data.integers.size()) return;

    data.integers[indexValue] = value.GetIntValue();
}

void GlobalStoreXExtension::SetString(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.stringBase;
    if (indexValue < 0 || indexValue >= data.strings.size()) return;

    data.strings[indexValue] = value.GetStringValue();
}

void GlobalStoreXExtension::SetBool(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.boolBase;
    if (indexValue < 0 || indexValue >= data.bools.size()) return;

    data.bools[indexValue] = value.GetIntValue() != 0;
}

void GlobalStoreXExtension::SetShort(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.shortBase;
    if (indexValue < 0 || indexValue >= data.shorts.size()) return;

    data.shorts[indexValue] = value.GetIntValue();
}

void GlobalStoreXExtension::AddInteger(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.integerBase;
    if (indexValue < 0 || indexValue >= data.integers.size()) return;

    data.integers[indexValue] += value.GetIntValue();
}

void GlobalStoreXExtension::AddShort(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.shortBase;
    if (indexValue < 0 || indexValue >= data.shorts.size()) return;

    data.shorts[indexValue] += value.GetIntValue();
}

void GlobalStoreXExtension::AddString(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.stringBase;
    if (indexValue < 0 || indexValue >= data.strings.size()) return;

    data.strings[indexValue] += value.GetStringValue();
}

void GlobalStoreXExtension::SubtractInteger(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.integerBase;
    if (indexValue < 0 || indexValue >= data.integers.size()) return;

    data.integers[indexValue] -= value.GetIntValue();
}

void GlobalStoreXExtension::SubtractShort(CValue index, CValue value)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.shortBase;
    if (indexValue < 0 || indexValue >= data.shorts.size()) return;

    data.shorts[indexValue] -= value.GetIntValue();
}

void GlobalStoreXExtension::SetRangeInteger(CValue startIndex, CValue endIndex, CValue value)
{
    if (data.integerCount == 0) return;

    int startIndexValue = startIndex.GetIntValue();
    int endIndexValue = endIndex.GetIntValue();
    startIndexValue -= data.integerBase;
    endIndexValue -= data.integerBase;

    if (startIndexValue < 0) startIndexValue = 0;
    if (endIndexValue >= (int)data.integerCount) endIndexValue = (int)data.integerCount - 1;
    if (startIndexValue >= (int)data.integerCount || endIndexValue < startIndexValue) return;

    for (int i = startIndexValue; i <= endIndexValue; i++)
    {
        data.integers[i] = value.GetIntValue();
    }
}

void GlobalStoreXExtension::SetRangeString(CValue startIndex, CValue endIndex, CValue value)
{
    if (data.stringCount == 0) return;

    int startIndexValue = startIndex.GetIntValue();
    int endIndexValue = endIndex.GetIntValue();
    startIndexValue -= data.stringBase;
    endIndexValue -= data.stringBase;

    if (startIndexValue < 0) startIndexValue = 0;
    if (endIndexValue >= (int)data.stringCount) endIndexValue = (int)data.stringCount - 1;
    if (startIndexValue >= (int)data.stringCount || endIndexValue < startIndexValue) return;

    for (int i = startIndexValue; i <= endIndexValue; i++)
    {
        data.strings[i] = value.GetStringValue();
    }
}

void GlobalStoreXExtension::SetRangeBool(CValue startIndex, CValue endIndex, CValue value)
{
    if (data.boolCount == 0) return;

    int startIndexValue = startIndex.GetIntValue();
    int endIndexValue = endIndex.GetIntValue();
    startIndexValue -= data.boolBase;
    endIndexValue -= data.boolBase;

    if (startIndexValue < 0) startIndexValue = 0;
    if (endIndexValue >= (int)data.boolCount) endIndexValue = (int)data.boolCount - 1;
    if (startIndexValue >= (int)data.boolCount || endIndexValue < startIndexValue) return;

    for (int i = startIndexValue; i <= endIndexValue; i++)
    {
        data.bools[i] = value.GetIntValue() != 0;
    }
}

void GlobalStoreXExtension::SetRangeShort(CValue startIndex, CValue endIndex, CValue value)
{
    if (data.shortCount == 0) return;

    int startIndexValue = startIndex.GetIntValue();
    int endIndexValue = endIndex.GetIntValue();
    startIndexValue -= data.shortBase;
    endIndexValue -= data.shortBase;

    if (startIndexValue < 0) startIndexValue = 0;
    if (endIndexValue >= (int)data.shortCount) endIndexValue = (int)data.shortCount - 1;
    if (startIndexValue >= (int)data.shortCount || endIndexValue < startIndexValue) return;

    for (int i = startIndexValue; i <= endIndexValue; i++)
    {
        data.shorts[i] = value.GetIntValue();
    }
}

void GlobalStoreXExtension::ClearIntegers()
{
    data.integers.clear();
    data.integers.resize(data.integerCount);
}

void GlobalStoreXExtension::ClearStrings()
{
    data.strings.clear();
    data.strings.resize(data.stringCount);
}

void GlobalStoreXExtension::ClearBools()
{
    data.bools.clear();
    data.bools.resize(data.boolCount);
}

void GlobalStoreXExtension::ClearShorts()
{
    data.shorts.clear();
    data.shorts.resize(data.shortCount);
}

void GlobalStoreXExtension::ResizeIntegers(CValue size)
{
    data.integers.clear();
    data.integers.resize(size.GetIntValue());
    data.integerCount = size.GetIntValue();
}

void GlobalStoreXExtension::ResizeStrings(CValue size)
{
    data.strings.clear();
    data.strings.resize(size.GetIntValue());
    data.stringCount = size.GetIntValue();
}

void GlobalStoreXExtension::ResizeBools(CValue size)
{
    data.bools.clear();
    data.bools.resize(size.GetIntValue());
    data.boolCount = size.GetIntValue();
}

void GlobalStoreXExtension::ResizeShorts(CValue size)
{
    data.shorts.clear();
    data.shorts.resize(size.GetIntValue());
    data.shortCount = size.GetIntValue();
}

void GlobalStoreXExtension::ExpandIntegers(CValue size)
{
    data.integers.resize(size.GetIntValue());
    data.integerCount = size.GetIntValue();
}

void GlobalStoreXExtension::ExpandStrings(CValue size)
{
    data.strings.resize(size.GetIntValue());
    data.stringCount = size.GetIntValue();
}

void GlobalStoreXExtension::ExpandBools(CValue size)
{
    data.bools.resize(size.GetIntValue());
    data.boolCount = size.GetIntValue();
}

void GlobalStoreXExtension::ExpandShorts(CValue size)
{
    data.shorts.resize(size.GetIntValue());
    data.shortCount = size.GetIntValue();
}

CValue GlobalStoreXExtension::GetInteger(CValue index)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.integerBase;
    if (indexValue < 0 || indexValue >= data.integers.size()) return CValue(0);

    return CValue(data.integers[indexValue]);
}

CValue GlobalStoreXExtension::GetString(CValue index)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.stringBase;
    if (indexValue < 0 || indexValue >= data.strings.size()) return CValue("");

    return CValue(data.strings[indexValue]);
}

CValue GlobalStoreXExtension::GetBool(CValue index)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.boolBase;
    if (indexValue < 0 || indexValue >= data.bools.size()) return CValue(false);

    return CValue(data.bools[indexValue]);
}

CValue GlobalStoreXExtension::GetShort(CValue index)
{
    int indexValue = index.GetIntValue();
    indexValue -= data.shortBase;
    if (indexValue < 0 || indexValue >= data.shorts.size()) return CValue(0);

    return CValue(data.shorts[indexValue]);
}

void GlobalStoreXExtension::SaveAllINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    file.read(ini);

    std::string groupValue = group.GetStringValue();

    ini[groupValue + "Ints"].clear();
    ini[groupValue + "Ints"]["NoOfValues"] = std::to_string(data.integerCount);
    ini[groupValue + "Ints"]["Base"] = std::to_string(data.integerBase);
    for (int i = 0; i < data.integerCount; i++)
    {
        ini[groupValue + "Ints"][std::to_string(i)] = std::to_string(data.integers[i]);
    }

    ini[groupValue + "Strings"].clear();
    ini[groupValue + "Strings"]["NoOfStrings"] = std::to_string(data.stringCount);
    ini[groupValue + "Strings"]["Base"] = std::to_string(data.stringBase);
    for (int i = 0; i < data.stringCount; i++)
    {
        ini[groupValue + "Strings"][std::to_string(i)] = data.strings[i];
    }

    ini[groupValue + "Bools"].clear();
    ini[groupValue + "Bools"]["NoOfBools"] = std::to_string(data.boolCount);
    ini[groupValue + "Bools"]["Base"] = std::to_string(data.boolBase);
    for (int i = 0; i < data.boolCount; i++)
    {
        ini[groupValue + "Bools"][std::to_string(i)] = std::to_string(data.bools[i]);
    }
    
    ini[groupValue + "Shorts"].clear();
    ini[groupValue + "Shorts"]["NoOfShorts"] = std::to_string(data.shortCount);
    ini[groupValue + "Shorts"]["Base"] = std::to_string(data.shortBase);
    for (int i = 0; i < data.shortCount; i++)
    {
        ini[groupValue + "Shorts"][std::to_string(i)] = std::to_string(data.shorts[i]);
    }

    file.write(ini);
}

void GlobalStoreXExtension::LoadAllINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    if (!file.read(ini)) return;

    std::string groupValue = group.GetStringValue();

    data.integerBase = ini[groupValue + "Ints"]["Base"] == "0" ? 0 : 1;
    data.stringBase = ini[groupValue + "Strings"]["Base"] == "0" ? 0 : 1;
    data.boolBase = ini[groupValue + "Bools"]["Base"] == "0" ? 0 : 1;
    data.shortBase = ini[groupValue + "Shorts"]["Base"] == "0" ? 0 : 1;

    data.integerCount = std::stoi(ini[groupValue + "Ints"]["NoOfValues"]);
    data.integers.clear();
    data.integers.resize(data.integerCount);
    for (int i = 0; i < data.integerCount; i++)
    {
        data.integers[i] = std::stoi(ini[groupValue + "Ints"][std::to_string(i)]);
    }

    data.stringCount = std::stoi(ini[groupValue + "Strings"]["NoOfStrings"]);
    data.strings.clear();
    data.strings.resize(data.stringCount);
    for (int i = 0; i < data.stringCount; i++)
    {
        data.strings[i] = ini[groupValue + "Strings"][std::to_string(i)];
    }

    data.boolCount = std::stoi(ini[groupValue + "Bools"]["NoOfBools"]);
    data.bools.clear();
    data.bools.resize(data.boolCount);
    for (int i = 0; i < data.boolCount; i++)
    {
        data.bools[i] = std::stoi(ini[groupValue + "Bools"][std::to_string(i)]) != 0;
    }

    data.shortCount = std::stoi(ini[groupValue + "Shorts"]["NoOfShorts"]);
    data.shorts.clear();
    data.shorts.resize(data.shortCount);
    for (int i = 0; i < data.shortCount; i++)
    {
        data.shorts[i] = std::stoi(ini[groupValue + "Shorts"][std::to_string(i)]);
    }
}

void GlobalStoreXExtension::SaveIntegerINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    file.read(ini);

    std::string groupValue = group.GetStringValue();

    ini[groupValue + "Ints"].clear();
    ini[groupValue + "Ints"]["NoOfValues"] = std::to_string(data.integerCount);
    ini[groupValue + "Ints"]["Base"] = std::to_string(data.integerBase);
    for (int i = 0; i < data.integerCount; i++)
    {
        ini[groupValue + "Ints"][std::to_string(i)] = std::to_string(data.integers[i]);
    }

    file.write(ini);
}

void GlobalStoreXExtension::LoadIntegerINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    if (!file.read(ini)) return;
    
    std::string groupValue = group.GetStringValue();

    data.integerBase = ini[groupValue + "Ints"]["Base"] == "0" ? 0 : 1;

    data.integerCount = std::stoi(ini[groupValue + "Ints"]["NoOfValues"]);
    data.integers.clear();
    data.integers.resize(data.integerCount);
    for (int i = 0; i < data.integerCount; i++)
    {
        data.integers[i] = std::stoi(ini[groupValue + "Ints"][std::to_string(i)]);
    }
}

void GlobalStoreXExtension::SaveStringINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    file.read(ini);

    std::string groupValue = group.GetStringValue();

    ini[groupValue + "Strings"].clear();
    ini[groupValue + "Strings"]["NoOfStrings"] = std::to_string(data.stringCount);
    ini[groupValue + "Strings"]["Base"] = std::to_string(data.stringBase);
    for (int i = 0; i < data.stringCount; i++)
    {
        ini[groupValue + "Strings"][std::to_string(i)] = data.strings[i];
    }

    file.write(ini);
}

void GlobalStoreXExtension::LoadStringINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    if (!file.read(ini)) return;

    std::string groupValue = group.GetStringValue();

    data.stringBase = ini[groupValue + "Strings"]["Base"] == "0" ? 0 : 1;

    data.stringCount = std::stoi(ini[groupValue + "Strings"]["NoOfStrings"]);
    data.strings.clear();
    data.strings.resize(data.stringCount);

    for (int i = 0; i < data.stringCount; i++)
    {
        data.strings[i] = ini[groupValue + "Strings"][std::to_string(i)];
    }
}

void GlobalStoreXExtension::SaveBoolINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    file.read(ini);

    std::string groupValue = group.GetStringValue();

    ini[groupValue + "Bools"].clear();
    ini[groupValue + "Bools"]["NoOfBools"] = std::to_string(data.boolCount);
    ini[groupValue + "Bools"]["Base"] = std::to_string(data.boolBase);
    for (int i = 0; i < data.boolCount; i++)
    {
        ini[groupValue + "Bools"][std::to_string(i)] = std::to_string(data.bools[i]);
    }

    file.write(ini);
}

void GlobalStoreXExtension::LoadBoolINI(CValue fileName, CValue group)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    if (!file.read(ini)) return;
    
    std::string groupValue = group.GetStringValue();

    data.boolBase = ini[groupValue + "Bools"]["Base"] == "0" ? 0 : 1;

    data.boolCount = std::stoi(ini[groupValue + "Bools"]["NoOfBools"]);
    data.bools.clear();
    data.bools.resize(data.boolCount);
    for (int i = 0; i < data.boolCount; i++)
    {
        data.bools[i] = std::stoi(ini[groupValue + "Bools"][std::to_string(i)]) != 0;
    }
}

void GlobalStoreXExtension::SaveShortINI(CValue fileName)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;

    file.read(ini);

    ini["Shorts"].clear();
    ini["Shorts"]["NoOfShorts"] = std::to_string(data.shortCount);
    ini["Shorts"]["Base"] = std::to_string(data.shortBase);
    for (int i = 0; i < data.shortCount; i++)
    {
        ini["Shorts"][std::to_string(i)] = std::to_string(data.shorts[i]);
    }

    file.write(ini);
}

void GlobalStoreXExtension::LoadShortINI(CValue fileName)
{
    mINI::INIFile file(GetNormalizedPath(fileName.GetStringValue()));
    mINI::INIStructure ini;
    
    if (!file.read(ini)) return;

    data.shortBase = ini["Shorts"]["Base"] == "0" ? 0 : 1;

    data.shortCount = std::stoi(ini["Shorts"]["NoOfShorts"]);
    data.shorts.clear();
    data.shorts.resize(data.shortCount);
    for (int i = 0; i < data.shortCount; i++)
    {
        data.shorts[i] = std::stoi(ini["Shorts"][std::to_string(i)]);
    }
}

void GlobalStoreXExtension::SaveAllBinary(CValue fileName)
{
    BinaryWriter writer(GetNormalizedPath(fileName.GetStringValue()));

    writer.WriteUint16(3); // type of file type. 0 - ints; 1 - strings; 2 - bools; 3 - all data; shorts - 4

    writer.WriteUint32(data.integerCount);
    writer.WriteUint8(data.integerBase);
    for (int i = 0; i < data.integerCount; i++)
    {
        writer.WriteInt32(data.integers[i]);
    }

    writer.WriteUint32(data.boolCount);
    writer.WriteUint8(data.boolBase);
    for (int i = 0; i < data.boolCount; i++)
    {
        writer.WriteUint8(data.bools[i] ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
    }

    writer.WriteUint32(data.shortCount);
    writer.WriteUint8(data.shortBase);
    for (int i = 0; i < data.shortCount; i++)
    {
        writer.WriteInt16(data.shorts[i]);
    }

    writer.WriteUint32(data.stringCount);
    writer.WriteUint8(data.stringBase);
    for (int i = 0; i < data.stringCount; i++)
    {
        writer.WriteNullTerminatedString(data.strings[i]);
    }

    writer.Flush();
}

void GlobalStoreXExtension::LoadAllBinary(CValue fileName)
{
    if (!std::filesystem::exists(GetNormalizedPath(fileName.GetStringValue()))) return;

    BinaryReader reader(GetNormalizedPath(fileName.GetStringValue()));

    if (reader.ReadUint16() != 3) return;

    data.integerCount = reader.ReadUint32();
    data.integerBase = reader.ReadUint8();
    data.integers.resize(data.integerCount);
    for (int i = 0; i < data.integerCount; i++)
    {
        data.integers[i] = reader.ReadInt32();
    }

    data.boolCount = reader.ReadUint32();
    data.boolBase = reader.ReadUint8();
    data.bools.resize(data.boolCount);
    for (int i = 0; i < data.boolCount; i++)
    {
        data.bools[i] = reader.ReadUint8() != 0;
    }

    data.shortCount = reader.ReadUint32();
    data.shortBase = reader.ReadUint8();
    data.shorts.resize(data.shortCount);
    for (int i = 0; i < data.shortCount; i++)
    {
        data.shorts[i] = reader.ReadInt16();
    }

    data.stringCount = reader.ReadUint32();
    data.stringBase = reader.ReadUint8();
    data.strings.resize(data.stringCount);
    for (int i = 0; i < data.stringCount; i++)
    {
        data.strings[i] = reader.ReadNullTerminatedString();
    }
}

void GlobalStoreXExtension::SaveIntegerBinary(CValue fileName)
{
    BinaryWriter writer(GetNormalizedPath(fileName.GetStringValue()));

    writer.WriteUint16(0);
    writer.WriteUint32(data.integerCount);
    writer.WriteUint8(data.integerBase);
    for (int i = 0; i < data.integerCount; i++)
    {
        writer.WriteInt32(data.integers[i]);
    }

    writer.Flush();
}

void GlobalStoreXExtension::LoadIntegerBinary(CValue fileName)
{
    if (!std::filesystem::exists(GetNormalizedPath(fileName.GetStringValue()))) return;

    BinaryReader reader(GetNormalizedPath(fileName.GetStringValue()));

    if (reader.ReadUint16() != 0) return;

    data.integerCount = reader.ReadUint32();
    data.integerBase = reader.ReadUint8();
    data.integers.resize(data.integerCount);
    for (int i = 0; i < data.integerCount; i++)
    {
        data.integers[i] = reader.ReadInt32();
    }
}

void GlobalStoreXExtension::SaveStringBinary(CValue fileName)
{
    BinaryWriter writer(GetNormalizedPath(fileName.GetStringValue()));

    writer.WriteUint16(1);
    writer.WriteUint32(data.stringCount);
    writer.WriteUint8(data.stringBase);
    for (int i = 0; i < data.stringCount; i++)
    {
        writer.WriteNullTerminatedString(data.strings[i]);
    }

    writer.Flush();
}

void GlobalStoreXExtension::LoadStringBinary(CValue fileName)
{
    if (!std::filesystem::exists(GetNormalizedPath(fileName.GetStringValue()))) return;

    BinaryReader reader(GetNormalizedPath(fileName.GetStringValue()));

    if (reader.ReadUint16() != 1) return;

    data.stringCount = reader.ReadUint32();
    data.stringBase = reader.ReadUint8();
    data.strings.resize(data.stringCount);
    for (int i = 0; i < data.stringCount; i++)
    {
        data.strings[i] = reader.ReadNullTerminatedString();
    }
}

void GlobalStoreXExtension::SaveBoolBinary(CValue fileName)
{
    BinaryWriter writer(GetNormalizedPath(fileName.GetStringValue()));

    writer.WriteUint16(2);
    writer.WriteUint32(data.boolCount);
    writer.WriteUint8(data.boolBase);
    for (int i = 0; i < data.boolCount; i++)
    {
        writer.WriteUint8(data.bools[i] ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
    }

    writer.Flush();
}

void GlobalStoreXExtension::LoadBoolBinary(CValue fileName)
{
    if (!std::filesystem::exists(GetNormalizedPath(fileName.GetStringValue()))) return;

    BinaryReader reader(GetNormalizedPath(fileName.GetStringValue()));

    if (reader.ReadUint16() != 2) return;

    data.boolCount = reader.ReadUint32();
    data.boolBase = reader.ReadUint8();
    data.bools.resize(data.boolCount);
    for (int i = 0; i < data.boolCount; i++)
    {
        data.bools[i] = reader.ReadUint8() != 0;
    }
}

void GlobalStoreXExtension::SaveShortBinary(CValue fileName)
{
    BinaryWriter writer(GetNormalizedPath(fileName.GetStringValue()));

    writer.WriteUint16(4);
    writer.WriteUint32(data.shortCount);
    writer.WriteUint8(data.shortBase);
    for (int i = 0; i < data.shortCount; i++)
    {
        writer.WriteInt16(data.shorts[i]);
    }

    writer.Flush();
}

void GlobalStoreXExtension::LoadShortBinary(CValue fileName)
{
    if (!std::filesystem::exists(GetNormalizedPath(fileName.GetStringValue()))) return;

    BinaryReader reader(GetNormalizedPath(fileName.GetStringValue()));

    if (reader.ReadUint16() != 4) return;

    data.shortCount = reader.ReadUint32();
    data.shortBase = reader.ReadUint8();
    data.shorts.resize(data.shortCount);
    for (int i = 0; i < data.shortCount; i++)
    {
        data.shorts[i] = reader.ReadInt16();
    }
}

std::string GlobalStoreXExtension::GetNormalizedPath(const std::string& path)
{
#if defined(PLATFORM_WEB)
    return "/disk/" + path;
#else
    return path;
#endif
}