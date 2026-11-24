#pragma once

#include "Extension.h"
#include "ObjectInstance.h"
	
class StringParserExtension : public Extension {
public:
	StringParserExtension(unsigned int objectInfoHandle, int type, std::string name, std::string Source, bool caseSensitive, bool wildCards, int comparison )
		: Extension(objectInfoHandle, type, name), source(Source), CaseSensitive(caseSensitive), Wildcards(wildCards), Comparison(comparison) {}

	// Actions
    void SetString(CValue String) { source = String.GetStringValue(); };
	// Expressions
    CValue SP_middle(CValue i, CValue length);
private:
	std::string source;
	bool CaseSensitive;
	bool Wildcards;
	int Comparison;
};