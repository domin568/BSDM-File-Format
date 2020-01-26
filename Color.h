#ifndef  COLOR_H
#define COLOR_H
#include <iostream>
#pragma pack(1)
struct RGB_color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	friend bool operator == (const RGB_color& checked, const RGB_color& thisclass)
	{
		if (checked.r == thisclass.r && checked.g == thisclass.g && checked.b == thisclass.b)
		{
			return true;
		}
		return false;
	}
	friend std::ostream& operator<<(std::ostream& out, RGB_color& color) //wypisuje tablicê (z numerami pól), pozostawia puste dla wolnych pól
	{
		out << std::hex << "r : " << (int)color.r << " " << "g : " << (int)color.g << " " << "b : " << (int)color.b;
		return out;
	}
};


struct BSDM_PALETTE
{
	uint8_t numberOfColors;
	RGB_color* colors;
};
#endif // ! COLOR_h
