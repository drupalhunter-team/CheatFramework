#include <Windows.h>
#include <sstream>
#include "Hotkey.h"


Hotkey::Hotkey()
{
	Key = 0;
	Callback = nullptr;
	Active = true;
	Description = "<NO DESCRIPTION>";
}

Hotkey::Hotkey(int key, CallbackT callback)
{
	Key = key;
	Callback = callback;
	Active = true;
	Description = "<NO DESCRIPTION>";
}

Hotkey::~Hotkey()
{
	Active = false;
}

std::string Hotkey::toString()
{
	struct
	{
		short u1;
		char scancode;
		bool extended : 1;
		bool dontcare : 1;
		int u2 : 6;
	} lparam;


	lparam.u1 = 0;
	lparam.scancode = MapVirtualKey(Key, MAPVK_VK_TO_VSC);
	lparam.extended = 0;
	lparam.dontcare = 1;
	lparam.u2 = 0;

	char buffer[16];
	GetKeyNameText(*((long*)(&lparam)), buffer, 16);

	std::stringstream ss;
	ss << buffer << " - " << Description;

	return ss.str();
}
