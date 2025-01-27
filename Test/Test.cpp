#include "driver.h"

int main()
{
	init();
	auto pid = GetCurrentProcessId();
	auto base = get_base(pid);
	std::cout << std::hex << base << "\n";

	return 0;
}

