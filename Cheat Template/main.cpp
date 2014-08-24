#include <Cheats.h>

#include <iostream>

bool feature(bool enable)
{
	int value = enable ? 0 : 1;
	unsigned int address = Cheats::getModuleBase("some_process.exe") + 0xADD2E55;
	return Cheats::writeMemory(address, &value, 4);
}

int main()
{
	std::cout << "some nasty hack by nasty haxxor" << std::endl;
	std::cout << "Attaching to process..." << std::endl;
	Cheats::attachProcess("some_process.exe", true);

	std::cout << "Attached!" << std::endl;
	Cheats::registerHotkey('1', std::bind(feature, true), "feature ON");
	Cheats::registerHotkey('2', std::bind(feature, false), "feature OFF");
	
	Cheats::run(true);
	return 0;
}