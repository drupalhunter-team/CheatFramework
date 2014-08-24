#pragma once

#include <Windows.h>
#include <functional>
#include <vector>
#include <string>

#include "Hotkey.h"

namespace Cheats
{
	extern HANDLE Process;
	extern std::string ProcessName;
	extern std::vector<Hotkey> Hotkeys;


	/*
	Attaches to process
	writeMemory and readMemory won't work without it
	if bBlock is set to true, then the function will wait until successfully attaching to process
	*/
	bool attachProcess(const std::string& sName, bool bBlock = false);

	/*
	Registers hotkey
	*/
	Hotkey* registerHotkey(int vkCode, Hotkey::CallbackT callback, std::string sDesc = "<NO DESCRIPTION>");

	/*
	Returns hotkey by it's key
	*/
	Hotkey* getHotkey(int vkCode);
	

	/*
	Writes memory to attached process from data
	if unprotect is true, then the memory page rights are set to PAGE_EXECUTE_READWRITE
	*/
	bool writeMemory(unsigned int address, void* data, int size, bool unprotect = false);

	/*
	Reads memory from attached process to data
	if unprotect is true, then the memory page rights are set to PAGE_EXECUTE_READWRITE
	*/
	bool readMemory(unsigned int address, void* data, int size, bool unprotect = false);

	/*
	Returns first process' ID with given name
	*/
	unsigned int getPID(const std::string& sName);

	/*
	Returns base address of given module (default: main executable of process) within given process (default: attached process)
	*/
	DWORD getModuleBase(const std::string& sModuleName = ProcessName, HANDLE hProc = Process);

	/*
	Runs the main loop that receives key presses
	if bList is true, then it lists hotkeys to stdout
	*/
	void run(bool bList = false);
	
};

