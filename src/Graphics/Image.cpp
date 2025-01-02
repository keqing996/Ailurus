
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Graphics/Image.h"

#include <algorithm>

#include "stb/stb_image.h"

namespace Ailurus
{
    Image::Pixel::Pixel()
        : Pixel(0, 0, 0, 0)
    {
    }

    Image::Pixel::Pixel(uint32_t packColor)
    {
        auto pData = reinterpret_cast<std::uint8_t*>(&packColor);
        r = pData[0];
        g = pData[1];
        b = pData[2];
        a = pData[3];
    }

    Image::Pixel::Pixel(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
        : Pixel(red, green, blue, 255)
    {
    }

    Image::Pixel::Pixel(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha)
        : r(red), g(green), b(blue), a(alpha)
    {
    }

    uint32_t Image::Pixel::Pack() const
    {
        uint32_t result;

        uint8_t* pColor = reinterpret_cast<uint8_t*>(&result);
        pColor[0] = r;
        pColor[1] = g;
        pColor[2] = b;
        pColor[3] = a;

        return result;
    }

    Image::Image(uint width, uint height, Pixel pixel)
    {
        if (width == 0 || height == 0)
        {
            _width = 0;
            _height = 0;
            _data.clear();
            return;
        }

       _width = width;
       _height = height;

        uint32_t color = pixel.Pack();
        size_t pixelSize = static_cast<std::size_t>(_width) * static_cast<std::size_t>(_height);

        std::vector<std::uint8_t> newData(pixelSize * 4);
        uint32_t* pData = reinterpret_cast<uint32_t*>(newData.data());
        for (auto i = 0; i < pixelSize; i++)
            *(pData + i) = color;

        // create buffer and swap, incase exception
        _data.swap(newData);
    }

    Image::Image(uint width, uint height, std::uint8_t* pData)
    {
        CopyFromData(width, height, pData);
    }

    Image::Image(const std::string& filePath)
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        stbi_uc* pStbImage = ::stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (pStbImage == nullptr)
            return;

        // in case exception
        ScopeGuard fileStreamGuard = [&] { ::stbi_image_free(pStbImage); };

        CopyFromData(width, height, pStbImage);
    }

    Image::Image(const void* pMemoryData, size_t dataSize)
    {
       if (pMemoryData == nullptr || dataSize == 0)
           return;

       int width = 0;
       int height = 0;
       int channels = 0;
       stbi_uc* pStbImage = ::stbi_load_from_memory(
           static_cast<const unsigned char*>(pMemoryData),
           static_cast<int>(dataSize), &width, &height,
           &channels, STBI_rgb_alpha);

       if (pStbImage == nullptr)
           return;

       // in case exception
       ScopeGuard fileStreamGuard = [&] { ::stbi_image_free(pStbImage); };

       CopyFromData(width, height, pStbImage);
    }

    std::pair<Image::uint, Image::uint> Image::GetPixelSize() const
    {
       return { _width, _height };
    }

    Image::Pixel Image::GetPixel(uint x, uint y) const
    {
        if (x >= _width || y >= _height)
            return {};

        auto pData = reinterpret_cast<const uint32_t*>(_data.data());
        return pData[x + y * _width];
    }

    void Image::SetPixel(uint x, uint y, Pixel pixel)
    {
        if (x >= _width || y >= _height)
            return;

        auto pData = reinterpret_cast<uint32_t*>(_data.data());
        pData[x + y * _width] = pixel.Pack();
    }

    void Image::VerticalFlip()
    {
        if (!_data.empty())
        {
            auto rowSizeInByte = _width * 4;

            auto top = _data.begin();
            auto bottom = _data.end() - rowSizeInByte;

            for (std::size_t i = 0; i < _height / 2; i++)
            {
                std::swap_ranges(top, top + rowSizeInByte, bottom);

                top += rowSizeInByte;
                bottom -= rowSizeInByte;
            }
        }
    }

    const std::uint8_t* Image::GetBytesData() const
    {
        return _data.data();
    }

    const std::uint32_t* Image::GetPixelsData() const
    {
        return reinterpret_cast<const std::uint32_t*>(_data.data());
    }

    void Image::CopyFromData(uint width, uint height, std::uint8_t* pData)
    {
        if (pData == nullptr)
        {
            _width = 0;
            _height = 0;
            _data.clear();
            return;
        }

        _width = width;
        _height = height;

        size_t pixelSize = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
        std::vector<std::uint8_t> newData(pData, pData + pixelSize * 4);

        // create buffer and swap, incase exception
        _data.swap(newData);
    }
}
