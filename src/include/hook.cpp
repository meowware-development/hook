#include "hook.hpp"

#include <Windows.h>
#include "hde/hde32.h"

void hook::TrampolineHook::Create(std::string_view name, unsigned char* target, unsigned char* hooked)
{
	m_Name = name;
	m_TargetFunction = target;
	m_HookedFunction = hooked;

	unsigned char jump[5] = {
		0xE9, // JMP opcode
		0x00, 0x00, 0x00, 0x00 // Function address to fill
	};

	// Calculate the amount of bytes needed to back up 
	hde32s disasm;
	while (m_TrampolineLength < 5) {
		void* code = reinterpret_cast<void*>((uintptr_t)m_TargetFunction + m_TrampolineLength);
		m_TrampolineLength += hde32_disasm(code, &disasm);
	}

	// Allocate at least length + 5 bytes
	m_OriginalFunction = VirtualAlloc(NULL, m_TrampolineLength + 0x5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// Copy the first 5 bytes to call original or unhook later
	memcpy_s(m_OriginalFunction, m_TrampolineLength, m_TargetFunction, m_TrampolineLength);

	// Fill the rest of the jump instruction
	*reinterpret_cast<uintptr_t*>(jump + 0x1) = ((uintptr_t)m_TargetFunction + m_TrampolineLength) - ((uintptr_t)m_OriginalFunction - 0x5);

	// Copy the current byte array into the original function + trampolineLength
	memcpy_s(reinterpret_cast<void*>((uintptr_t)m_OriginalFunction + m_TrampolineLength), 0x5, jump, 0x5);

	DWORD currentProtection;

	VirtualProtect(reinterpret_cast<void*>(m_TargetFunction), 0x5, PAGE_EXECUTE_READWRITE, &currentProtection);

	// Get the RVA of the function that we want to hook 
	*(uintptr_t*)(jump + 0x1) = (uintptr_t)m_HookedFunction - (uintptr_t)m_TargetFunction - 0x5;

	// Overwrite the bytes
	memcpy_s(m_TargetFunction, 0x5, jump, 0x5);

	VirtualProtect(m_TargetFunction, 5, currentProtection, &currentProtection);

	/*
	From Microsoft docs:
	Applications should call FlushInstructionCache if they generate or modify code in memory.
	The CPU cannot detect the change, and may execute the old code it cached.
	*/
	FlushInstructionCache(GetCurrentProcess(), m_TargetFunction, m_TrampolineLength);
}

void hook::TrampolineHook::Remove()
{
	DWORD currentProtection;

	VirtualProtect(reinterpret_cast<void*>(m_TargetFunction), 0x5, PAGE_EXECUTE_READWRITE, &currentProtection);

	// Restore the original bytes to the target function
	memcpy_s(m_TargetFunction, m_TrampolineLength, m_OriginalFunction, m_TrampolineLength);

	VirtualProtect(m_TargetFunction, 5, currentProtection, &currentProtection);

	/*
	From Microsoft docs:
	Applications should call FlushInstructionCache if they generate or modify code in memory.
	The CPU cannot detect the change, and may execute the old code it cached.
	*/
	FlushInstructionCache(GetCurrentProcess(), m_TargetFunction, m_TrampolineLength);

	// Free the code we allocated earlier
	VirtualFree(m_OriginalFunction, 0, MEM_RELEASE);
}
