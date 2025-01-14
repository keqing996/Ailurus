#pragma once

#include <array>

namespace Ailurus
{
    template<typename T, size_t size, size_t SegmentSize>
    class SegmentArray
    {
        static_assert(SegmentSize > 0, "Segment size must larger than 0");
        static constexpr bool SegmentPowerOfTwo = SegmentSize > 0 && (SegmentSize & (SegmentSize - 1)) == 0;

    public:
        T& operator[](uint32_t index)
        {
            return _array[CalculateRealIndex(index)];
        }

        const T& operator[](uint32_t index) const
        {
            return _array[CalculateRealIndex(index)];
        }

        const std::array<T, size>& GetRawArray() const
        {
            return _array;
        }

    private:
        static uint32_t CalculateRealIndex(uint32_t index)
        {
            uint32_t segmentIndex = index / SegmentSize;

            uint32_t inSegmentIndex;
            if constexpr (SegmentPowerOfTwo)
                inSegmentIndex = index & (SegmentSize - 1);
            else
                inSegmentIndex = index % SegmentSize;

            return segmentIndex * SegmentSize + inSegmentIndex;
        }

    private:
        std::array<T, size> _array;
    };
}