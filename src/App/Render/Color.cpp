
#include "Ailurus/Graphics/Color.h"

namespace Ailurus
{
    Color::Color()
        : r(0), g(0), b(0), a(255)
    {
    }

    Color::Color(uint32_t packColor)
    {
        auto pData = reinterpret_cast<std::uint8_t*>(&packColor);
        r = pData[0];
        g = pData[1];
        b = pData[2];
        a = pData[3];
    }

    Color::Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
        : Color(red, green, blue, 255)
    {
    }

    Color::Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha)
        : r(red), g(green), b(blue), a(alpha)
    {
    }

    Color::Color(float red, float green, float blue, float alpha)
        : r(static_cast<uint8_t>(red * 255))
        , g(static_cast<uint8_t>(green * 255))
        , b(static_cast<uint8_t>(blue * 255))
        , a(static_cast<uint8_t>(alpha * 255))
    {
    }

    Color::Color(const Color& other) = default;

    Color& Color::operator=(const Color& other)
    {
        if (this != &other)
        {
            r = other.r;
            g = other.g;
            b = other.b;
            a = other.a;
        }
        return *this;
    }

    bool Color::operator==(const Color& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    bool Color::operator!=(const Color& other) const
    {
        return !(*this == other);
    }

    uint32_t Color::Pack() const
    {
        uint32_t result;

        uint8_t* pColor = reinterpret_cast<uint8_t*>(&result);
        pColor[0] = r;
        pColor[1] = g;
        pColor[2] = b;
        pColor[3] = a;

        return result;
    }
}