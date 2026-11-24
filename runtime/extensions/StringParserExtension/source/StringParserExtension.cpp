#include "StringParserExtension.h"
#include <cstdint>

CValue StringParserExtension::SP_middle(CValue i, CValue length)
{
    int lengthv = std::max(0, length.GetIntValue());
    int iv = i.GetIntValue();
    if ((iv >= 0) && (iv + lengthv <= source.size()))
    {
        return CValue(source.substr(iv, lengthv));
    }
    return CValue("");
}