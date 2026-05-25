#pragma once

#include <string>

class Paragraph
{
public:
	Paragraph(unsigned short font, int color, std::string text, unsigned char horizontalAlignment, unsigned char verticalAlignment)
		: Font(font), Color(color), Text(text), HorizontalAlignment(horizontalAlignment), VerticalAlignment(verticalAlignment) {}
	
	unsigned short Font;
	unsigned char HorizontalAlignment;
	unsigned char VerticalAlignment;
	int Color;

	std::string Text;
};