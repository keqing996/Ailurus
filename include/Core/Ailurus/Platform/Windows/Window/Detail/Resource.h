#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include <cstdint>

namespace Ailurus
{
    struct DataResource
    {
        void* pData;
        uint32_t size;
    };

    struct CursorResource
    {
        void* hCursor;
    };

    struct BitmapResource
    {
        void* hBitmap;
    };

    struct IconResource
    {
        void* hIcon;
    };

    class Resource
    {
    public:
        Resource() = delete;

    public:
        template<typename T>
        T LoadResource(int id);
    };
}

#endif

