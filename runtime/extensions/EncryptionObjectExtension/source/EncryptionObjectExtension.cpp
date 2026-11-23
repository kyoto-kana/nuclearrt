#include "EncryptionObjectExtension.h"
#include <cstdint>

static const char charSet[] = "b3C5oS1hk8MZ9N0GeUjL4rRvlma6Pqf2OQcdJK7nABpDFtgEsXYWTyHIViwzux";

CValue EncryptionObjectExtension::Encrypt(CValue String, CValue Key)
{
    std::string string = String.GetStringValue();
    std::string key = Key.GetStringValue();

    if (key.empty()) return CValue(std::string());
    
    std::string out(string.size() * 2, '\0');

    size_t keyStart = 0;
    size_t keyCurrent = 0;

    for (size_t i = 0; i < string.size(); ++i)
    {
        uint8_t x = static_cast<uint8_t>(key[keyCurrent] ^ string[i]);

        out[2 * i] = charSet[x % 62];
        out[2 * i + 1] = charSet[x / 62];

        if (++keyCurrent >= key.size())
        {
            if (++keyStart >= key.size())
            {
                keyStart = 0;
            }
            keyCurrent = keyStart;
        }
    }

    return CValue(out);
}

CValue EncryptionObjectExtension::Decrypt(CValue String, CValue Key)
{
    std::string string = String.GetStringValue();
    std::string key = Key.GetStringValue();

    if (key.empty()) return CValue(std::string());
    
    size_t outLength = string.size() >> 1;
    std::string out(outLength, '\0');
    size_t keyStart = 0;
    size_t keyCurrent = 0;

    for (size_t i = 0; i < outLength; ++i)
    {
        uint8_t high = 63;
        for (uint8_t j = 0; j < 62; ++j)
        {
            if (charSet[j] == string[2 * i + 1])
            {
                high = j;
                break;
            }
        }
        
        uint8_t low = 63;
        for (uint8_t k = 0; k < 62; ++k)
        {
            if (charSet[k] == string[2 * i])
            {
                low = k;
                break;
            }
        }

        out[i] = static_cast<char>(key[keyCurrent] ^ (low + 62 * high));

        if (++keyCurrent >= key.size())
        {
            if (++keyStart >= key.size())
            {
                keyStart = 0;
            }
            keyCurrent = keyStart;
        }
    }

    return CValue(out);
}