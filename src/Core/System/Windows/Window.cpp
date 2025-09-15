#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Platform/Windows/Window.h"
#include "Ailurus/Platform/Windows/WindowsDefine.h"

namespace Ailurus
{
	bool FixProcessDpiBySetProcessDpiAwareness()
	{
		HINSTANCE pShcodeDll = ::LoadLibraryW(L"Shcore.dll");

		if (pShcodeDll == nullptr)
			return false;

		ScopeGuard pShcodeDllReleaseGuard = [&pShcodeDll]()
		{
			::FreeLibrary(pShcodeDll);
		};

		void* pFuncSetProcessDpiAwareness = reinterpret_cast<void*>(GetProcAddress(pShcodeDll, "SetProcessDpiAwareness"));
		if (pFuncSetProcessDpiAwareness == nullptr)
			return false;

		enum ProcessDpiAwareness
		{
			ProcessDpiUnaware         = 0,
			ProcessSystemDpiAware     = 1,
			ProcessPerMonitorDpiAware = 2
		};

		using SetProcessDpiAwarenessFuncType = HRESULT (WINAPI*)(ProcessDpiAwareness);

		auto setProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(pFuncSetProcessDpiAwareness);
		if (setProcessDpiAwarenessFunc(ProcessPerMonitorDpiAware) == E_INVALIDARG)
			return false;

		return true;
	}

	bool FixProcessDpiBySetProcessDPIAware()
	{
		HINSTANCE pUser32Dll = ::LoadLibraryW(L"user32.dll");

		if (pUser32Dll == nullptr)
			return false;

		ScopeGuard pShcodeDllReleaseGuard = [&pUser32Dll]()
		{
			::FreeLibrary(pUser32Dll);
		};

		void* pFuncSetProcessDPIAware = reinterpret_cast<void*>(GetProcAddress(pUser32Dll, "SetProcessDPIAware"));
		if (pFuncSetProcessDPIAware == nullptr)
			return false;

		using SetProcessDPIAwareFuncType = BOOL (WINAPI*)(void);

		auto setProcessDpiAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(pFuncSetProcessDPIAware);
		if (!setProcessDpiAwareFunc())
			return false;

		return true;
	}

	bool NativeWindowUtility::FixProcessDpi()
	{
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

		// Try SetProcessDpiAwareness first
		if (FixProcessDpiBySetProcessDpiAwareness())
			return true;

		// Fall back to SetProcessDPIAware
		if (FixProcessDpiBySetProcessDPIAware())
			return true;

		return false;
	}
}

#endif