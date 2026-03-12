#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "Color.h"

namespace Ailurus
{
    // Y axis zero on top
    class Image
    {
        using uint = unsigned int;
    public:
        Image(uint width, uint height, Color pixel);
        Image(uint width, uint height, std::uint8_t* pData);

        // Load form file
        explicit Image(const std::string& filePath);

        // Load from memory
        Image(const void* pMemoryData, size_t dataSize);

    public:
        std::pair<uint, uint> GetPixelSize() const;

        Color GetPixel(uint x, uint y) const;

        void SetPixel(uint x, uint y, Color pixel);

        void VerticalFlip();

        const std::uint8_t* GetBytesData() const;
        const std::uint32_t* GetPixelsData() const;

    private:
        void CopyFromData(uint width, uint height, std::uint8_t* pData);

    private:
        std::vector<std::uint8_t> _data;
        uint _width = 0; // pixels
        uint _height = 0; // pixels
    };
}