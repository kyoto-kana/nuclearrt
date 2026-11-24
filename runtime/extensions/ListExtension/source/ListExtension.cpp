#include "ListExtension.h"
#include "Active.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdlib>
#ifdef NUCLEAR_BACKEND_SDL3
#include <SDL3/SDL_stdinc.h>
#elif NUCLEAR_BACKEND_SDL2
#include <SDL_stdic.h>
#endif

#pragma region Actions

std::string ListExtension::WriteList()
{
    std::string string;
    size_t size = 0;

    // pre allocate
    for (int i = 0; i < Strings.size(); i++)
    {
        size += Strings[i].size() + sizeof('\0');
    }
    string.reserve(size);
    

    for (int i = 0; i < Strings.size(); i++)
    {
        string.append(Strings[i]);
        string.push_back('\n');
    }
    return string;
}

void ListExtension::Load(CValue value)
{
    std::ifstream file(value.GetStringValue(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return;

    std::streamsize size = file.tellg();
    if (size <= 0)
        return;

    file.seekg(0, std::ios::beg);
    std::string buffer((size_t)size, '\0');
    if (!file.read(&buffer[0], size))
        return;

    // decode
    std::string text;
    auto byteAt = [&](size_t i) { return (unsigned char)buffer[i]; };

    // I feel like this should be replaced with an SDL-less method, maybe uftcpp?
    if (buffer.size() >= 2 && byteAt(0) == 0xFF && byteAt(1) == 0xFE) {
        char* out = SDL_iconv_string("UTF-8", "UTF-16LE", buffer.data() + 2, buffer.size() - 2);
        if (out) {
            text = out;
            SDL_free(out);
        }
    }
    else if (buffer.size() >= 3 && byteAt(0) == 0xEF && byteAt(1) == 0xBB && byteAt(2) == 0xBF) {
        // utf8
        text = buffer.substr(3);
    }
    else {
        // ascii
        text = std::move(buffer);
    }

    Strings.clear();
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        Strings.push_back(std::move(line));
    }

    Datas.assign(Strings.size(), 0);
}


void ListExtension::Save(CValue value)
{
    std::string list = WriteList();
    std::ofstream file(value.GetStringValue(), std::ios::binary);
    file.write(list.data(), list.size());
}

void ListExtension::ChangeLine(CValue value, CValue value2)
{
    int index = value.GetIntValue() + IndexOffset;
    if (index >= 0)
    {
        Strings[index] = value2.GetStringValue();
    }
}

void ListExtension::DeleteLine(CValue value)
{
    int index = value.GetIntValue() + IndexOffset;
    if (index < 0)
    {
        return;
    }
    if (index >= Strings.size())
    {
        return;
    }
    if (index < Strings.size()) {
        Strings.erase(Strings.begin() + index); 
    }
    if (index < Datas.size()) {
        Datas.erase(Datas.begin() + index); 
    }
}

void ListExtension::InsertLine(CValue value, CValue value2)
{
    int index = value.GetIntValue() + IndexOffset;
    if (index < 0)
    {
        return;
    }
    if (index >= Strings.size())
    {
        return;
    }
    Strings[Strings.size()] = value.GetStringValue();
    Datas[Strings.size()] = 0;
}

void ListExtension::AddLine(CValue value)
{
    Strings[Strings.size()] = value.GetStringValue();
}

void ListExtension::SetLineData(CValue value, CValue value2)
{
    int index = value.GetIntValue() + IndexOffset;
    // Must be an existing element
    if (index < 0)
    {
        return;
    }
    if (index >= Strings.size())
    {
        return;
    }
    Datas[index] = value2.GetIntValue();
}

#pragma endregion Actions

#pragma region Conditions

#pragma endregion Conditions

#pragma region Expressions

CValue ListExtension::GetLineData(CValue index)
{
    int indexv = index.GetIntValue() + IndexOffset;
    if ((indexv < 0) || (indexv >= Datas.size()))
    {
        return CValue(0);
    }
    return CValue((int)(Datas[indexv]));
}

CValue ListExtension::GetLineText(CValue index)
{
    int indexv = index.GetIntValue() + IndexOffset;
    if ((indexv < 0) || (indexv >= Datas.size()))
    {
        return CValue("");
    }
    return CValue((Strings[indexv]));
}


int ListExtension::FindStringInt(std::string search, int startIndex)
{
    if (search.size() > 0)
    {
        int tries = Strings.size();
        int i = startIndex;
        while (tries > 0)
        {
            tries--;
            i++;
            // Wrap around
            if (i >= Strings.size())
            {
                i = 0;
            }
            std::string cmp = (Strings[i]);
            if (cmp.find(search)==0)
            {
                // Found a line
                return i;
            }
        }
    }
    return -1;
}
int ListExtension::FindStringExactInt(std::string search, int startIndex)
{
    if (search.size() > 0)
    {
        int tries = Strings.size();
        int i = startIndex;
        while (tries > 0)
        {
            tries--;
            i++;
            // Wrap around
            if (i >= Strings.size())
            {
                i = 0;
            }
            std::string cmp = (Strings[i]);
            if (cmp.find(search)==0)
            {
                // Found a line
                return i;
            }
        }
    }
    return -1;			
}

CValue ListExtension::FindString(CValue search, CValue index)
{
    std::string searchv = search.GetStringValue();
    int startIndexv = index.GetIntValue();
    if (startIndexv != -1)
    {
        startIndexv += IndexOffset;
    }
    if ((startIndexv < 0) || (startIndexv >= Strings.size()))
    {
        startIndexv = -1;
    }
    int ret = FindStringInt(searchv, startIndexv);
    if (ret>=0)
    {
        ret -= IndexOffset;
    }
    return CValue(ret);	       
}

CValue ListExtension::FindStringExact(CValue search, CValue index)
{
    std::string searchv = search.GetStringValue();
    int startIndexv = index.GetIntValue();
    if (startIndexv != -1)
    {
        startIndexv += IndexOffset;
    }
    if ((startIndexv < 0) || (startIndexv >= Strings.size()))
    {
        startIndexv = -1;
    }
    int ret = FindStringExactInt(searchv, startIndexv);
    if (ret>=0)
    {
        ret -= IndexOffset;
    }
    return CValue(ret);	    
}

#pragma endregion Expressions