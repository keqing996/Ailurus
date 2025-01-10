#include "Ailurus/Utility/SpinPause.h"
#include "Ailurus/PlatformDefine.h"

#if AILURUS_CHIP_X64 || AILURUS_CHIP_X86
#include <emmintrin.h>
#endif

namespace Ailurus
{
#if AILURUS_CHIP_X64 || AILURUS_CHIP_X86
    void SpinPause()
    {
        _mm_pause();
    }
#elif AILURUS_CHIP_ARM64
    void SpinPause()
    {
        asm volatile ("yield" ::: "memory");
    }
#elif AILURUS_CHIP_ARM32
    void SpinPause()
    {
        asm volatile ("nop" ::: "memory");
    }
#else
    void SpinPause() { }
#endif

}
