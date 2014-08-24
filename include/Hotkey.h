#pragma once

#include <functional>
#include <string>

class Hotkey
{
	public:
	typedef std::function<bool()> CallbackT;

	Hotkey();
	Hotkey(int key, CallbackT callback);
	~Hotkey();

	std::string toString();

	int Key;
	CallbackT Callback;
	bool Active;
	std::string Description;
};

