#pragma once

#include <cstdint>

namespace Ailurus
{
    class Color
    {
    public:
        Color();
        explicit Color(uint32_t packColor);
        Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue);
        Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
        Color(float red, float green, float blue, float alpha);

        Color(const Color& other);

        Color& operator=(const Color& other);
        bool operator==(const Color& other) const;
        bool operator!=(const Color& other) const;

    public:
        uint32_t Pack() const;

    private:
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
    };
}