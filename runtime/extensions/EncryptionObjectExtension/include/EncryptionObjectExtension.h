#pragma once

#include "Extension.h"
#include "ObjectInstance.h"
	
class EncryptionObjectExtension : public Extension {
public:
	EncryptionObjectExtension(unsigned int objectInfoHandle, int type, std::string name)
		: Extension(objectInfoHandle, type, name) {}

	// Expressions
    CValue Encrypt(CValue String, CValue Key);
    CValue Decrypt(CValue String, CValue Key);
};