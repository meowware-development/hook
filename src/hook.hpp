#pragma once

#include <cstdint>
#include <string_view>

namespace hook {
	class TrampolineHook {
	public:
		TrampolineHook() = default;
		~TrampolineHook() = default;

		void Create(std::string_view name, unsigned char* target, unsigned char* hooked);
		void Remove();

		template <typename Type>
		Type GetOriginal()
		{
			return reinterpret_cast<Type>(m_OriginalFunction);
		}

	private:
		unsigned char* m_TargetFunction, * m_HookedFunction, * m_OriginalFunction;
		std::string_view m_Name;
		unsigned long m_OldProtection;
	};
}