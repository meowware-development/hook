#include "hook.hpp"

#include <Windows.h>

void hook::TrampolineHook::Create(std::string_view name, unsigned char* target, unsigned char* hooked)
{
	m_Name = name;
	m_TargetFunction = target;
	m_HookedFunction = hooked;

	m_OriginalFunction = reinterpret_cast<unsigned char*>(VirtualAlloc(NULL, 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

	// Copy the first 5 bytes
	memcpy_s(m_OriginalFunction, 0x5, m_TargetFunction, 0x5);

	// Get the RVA of the targeted function to be able to call it back
	uintptr_t relativeOffset = m_TargetFunction - m_OriginalFunction - 0x5;

	// Set JMP instruction to call original function
	*reinterpret_cast<uintptr_t*>(m_OriginalFunction + 0x5) = 0xE9;
	*reinterpret_cast<uintptr_t*>(m_OriginalFunction + 0x6) = relativeOffset;

	DWORD currentProtection;

	VirtualProtect(reinterpret_cast<void*>(m_TargetFunction), 0x5, PAGE_EXECUTE_READWRITE, &currentProtection);

	// Get the RVA of the function that we want to hook 
	uintptr_t detourRelativeOffset = m_HookedFunction - m_TargetFunction - 0x5;

	// Set JMP instruction to call hooked function
	*reinterpret_cast<uintptr_t*>(m_TargetFunction) = 0xE9;
	*reinterpret_cast<uintptr_t*>(m_TargetFunction + 0x1) = detourRelativeOffset;

	VirtualProtect(m_TargetFunction, 5, currentProtection, &currentProtection);
}

void hook::TrampolineHook::Remove()
{
	DWORD currentProtection;

	VirtualProtect(reinterpret_cast<void*>(m_TargetFunction), 0x5, PAGE_EXECUTE_READWRITE, &currentProtection);

	// Restore the original bytes to the target function
	memcpy_s(m_TargetFunction, 0x5, m_OriginalFunction, 0x5);

	VirtualProtect(m_TargetFunction, 5, currentProtection, &currentProtection);
}
