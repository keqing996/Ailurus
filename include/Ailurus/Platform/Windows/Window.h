#pragma once

#include "../../PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

namespace Ailurus
{
	class NativeWindowUtility
	{
	public:
		static bool FixProcessDpi();
	};
}

#endif