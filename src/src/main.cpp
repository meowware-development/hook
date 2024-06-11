#include "../hook.hpp"

#include <iostream>

#include <Windows.h>

hook::TrampolineHook x;
typedef int (WINAPI* TdefOldMessageBoxA)(HWND hWnd, LPCSTR lpText, LPCTSTR lpCaption, UINT uType);

int WINAPI NewMessageBoxA(HWND hWnd, LPCSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	printf("MessageBoxA called!\ntitle: %s\ntext: %s\n\n", lpCaption, lpText);
	return static_cast<decltype(&NewMessageBoxA)>(x.m_OriginalFunction)(hWnd, lpText, lpCaption, uType);
}

int main() {
	x.Create("a", reinterpret_cast<unsigned char*>(MessageBoxA), reinterpret_cast<unsigned char*>(NewMessageBoxA));

	MessageBoxA(NULL, "Test", "Test", MB_OK);

	x.Remove();

	MessageBoxA(NULL, "TEST1", "TEST", MB_ICONERROR);
}