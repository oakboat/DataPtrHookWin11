#include "driver.h"

int main()
{
	init();
	auto pid = GetCurrentProcessId();
	auto base = get_base(pid);
	std::cout << std::hex << base << "\n";
	int x = 0x123;
	std::cout << read<int>(pid, (ULONG64)&x) << "\n";
	write(pid, (ULONG64)&x, 0x456u);
	std::cout << x << "\n";
	return 0;
}

