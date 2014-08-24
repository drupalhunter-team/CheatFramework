#include "Cheats.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <thread>

#pragma comment( lib, "psapi" )

namespace Cheats
{
	bool Active = true;
	std::vector<Hotkey> Hotkeys;
	HHOOK KeyboardHook = 0;
	HANDLE Process = NULL;
	std::string ProcessName;

	bool setProcess(const std::string& sName, bool bBlock)
	{
		do
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			unsigned int procID = getPID(sName);

			ProcessName = sName;
			Process = OpenProcess(PROCESS_ALL_ACCESS, false, procID);

		}while (bBlock && Process == NULL);

		return Process != NULL;
	}

	LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		KBDLLHOOKSTRUCT *pKbdLLHookStruct = (KBDLLHOOKSTRUCT *)lParam;
		if (nCode >= 0)
		{
			if (wParam == WM_KEYUP)
			{
				for (Hotkey& hotkey : Hotkeys)
				{
					if (hotkey.Key == pKbdLLHookStruct->vkCode)
					{
						if(!hotkey.Callback())
							MessageBox(0, "Callback error", "Failed", 0);
					}
				}
			}
		}

		return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
	}

	void run(bool bList)
	{
		if (bList)
		{
			for (auto& i : Hotkeys)
			{
				std::cout << i.toString() << std::endl;
			}
		}

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	bool writeMemory(unsigned int address, void* data, int size, bool unprotect)
	{
		if (!Process)
			return false;

		SIZE_T written;
		DWORD old;

		if (unprotect && !VirtualProtectEx(Process, data, size, PAGE_EXECUTE_READWRITE, &old))
		{
			std::cout << "Error: VirtualProtectEx failed: " << GetLastError() << std::endl;
			return false;
		}
		BOOL result = WriteProcessMemory(Process, (LPVOID)address, data, size, &written);
		if (unprotect && !VirtualProtectEx(Process, data, size, old, &old))
		{
			std::cout << "Error: VirtualProtectEx failed: " << GetLastError() << std::endl;
			return false;
		}

		return result && written == size;
	}

	bool readMemory(unsigned int address, void* data, int size, bool unprotect)
	{
		if (!Process)
			return false;

		SIZE_T read;
		DWORD old;

		if (unprotect && !VirtualProtectEx(Process, data, size, PAGE_EXECUTE_READWRITE, &old))
		{
			std::cout << "Error: VirtualProtectEx failed: " << GetLastError() << std::endl;
			return false;
		}
		BOOL result = ReadProcessMemory(Process, (LPVOID)address, data, size, &read);
		if (unprotect && !VirtualProtectEx(Process, data, size, old, &old))
		{
			std::cout << "Error: VirtualProtectEx failed: " << GetLastError() << std::endl;
			return false;
		}

		return result && read == size;
	}

	Hotkey* registerHotkey(int vkCode, Hotkey::CallbackT callback, std::string sDesc)
	{
		for (Hotkey& hotkey : Hotkeys)
		{
			if (hotkey.Key == vkCode)
			{
				std::cout << "Warning: Virtual code: " << vkCode << " is already registered!" << std::endl;
				break;
			}
		}


		Hotkey hotkey;
		hotkey.Callback = callback;
		hotkey.Key = vkCode;
		hotkey.Description = sDesc;
		Hotkeys.push_back(hotkey);

		if (KeyboardHook == 0)
		{
			KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(0), 0);
			TOKEN_PRIVILEGES   tp;
			HANDLE             hToken;

			if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
			{
				if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
				{
					if (hToken != INVALID_HANDLE_VALUE)
					{
						tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
						tp.PrivilegeCount = 1;
						AdjustTokenPrivileges(hToken, FALSE, &tp, 0, 0, 0);
						CloseHandle(hToken);
					}
				}
			}
		}

		return &Hotkeys.back();
	}

	Hotkey* getHotkey(int vkCode)
	{
		for (Hotkey& hotkey : Hotkeys)
		{
			if (hotkey.Key == vkCode)
			{
				return &hotkey;
			}
		}

		return nullptr;
	}

	DWORD getModuleBase(const std::string& sModuleName, HANDLE hProc)
	{
		HMODULE *hModules = nullptr;
		char buffer[64];
		DWORD modules_size;
		DWORD result = -1;

		if (!EnumProcessModules(hProc, hModules, 0, &modules_size))
		{
			std::cout << "Error: EnumProcessModules failed: " << GetLastError() << std::endl;
			return 0;
		}
		hModules = new HMODULE[modules_size / sizeof(HMODULE)];

		if (EnumProcessModules(hProc, hModules, modules_size / sizeof(HMODULE), &modules_size))
		{
			for (unsigned int i = 0; i < modules_size / sizeof(HMODULE); i++)
			{
				if (GetModuleBaseName(hProc, hModules[i], buffer, sizeof(buffer)))
				{
					if (strcmp(sModuleName.c_str(), buffer) == 0)
					{
						result = (DWORD)hModules[i];
						break;
					}
				}
			}
		}

		delete[] hModules;

		return result;
	}

	unsigned int getPID(const std::string& sName)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = NULL;
		snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == NULL)
		{
			std::cout << "Error: CreateToolhelp32Snapshot failed: " << GetLastError() << std::endl;
			return 0;
		}
		if (!Process32First(snapshot, &procEntry))
		{
			std::cout << "Error: Process32First failed: " << GetLastError() << std::endl;
			return 0;
		}
		if (!strcmp(procEntry.szExeFile, sName.c_str()))
		{
			CloseHandle(snapshot);
			return procEntry.th32ProcessID;
		}

		while (Process32Next(snapshot, &procEntry))
		{
			if (!strcmp(procEntry.szExeFile, sName.c_str()))
			{
				CloseHandle(snapshot);
				return procEntry.th32ProcessID;
			}
		}
		CloseHandle(snapshot);

		return 0;
	}

}