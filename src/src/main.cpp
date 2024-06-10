#include "../hook.hpp"

#include <iostream>

hook::TrampolineHook x;

int __stdcall a() {
	std::cout << "1\n";
	return 0;
}

int __stdcall b() {
	std::cout << "2\n";

	auto original = x.GetOriginal<decltype(&b)>();

	return 0;
	//return original();
}

int main() {
	a();

	x.Create("a", reinterpret_cast<unsigned char*>(a), reinterpret_cast<unsigned char*>(b));

	a();
	a();

	x.Remove();

	a();
}