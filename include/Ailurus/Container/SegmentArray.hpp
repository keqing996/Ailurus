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

    public:
        class iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            iterator(SegmentArray* array, size_t index) : _array(array), _index(index)
            {
            }

            reference operator*() { return (*_array)[_index]; }
            pointer operator->() { return &(*_array)[_index]; }

            iterator& operator++()
            {
                ++_index;
                return *this;
            }

            iterator operator++(int)
            {
                iterator temp = *this;
                ++_index;
                return temp;
            }

            iterator& operator--()
            {
                --_index;
                return *this;
            }

            iterator operator--(int)
            {
                iterator temp = *this;
                --_index;
                return temp;
            }

            iterator operator+(size_t n) const
            {
                return iterator(_array, _index + n);
            }

            iterator operator-(size_t n) const
            {
                return iterator(_array, _index - n);
            }

            difference_type operator-(const iterator& other) const
            {
                return _index - other._index;
            }

            reference operator[](size_t n) { return (*_array)[_index + n]; }

            bool operator==(const iterator& other) const { return _index == other._index; }
            bool operator!=(const iterator& other) const { return _index != other._index; }
            bool operator<(const iterator& other) const { return _index < other._index; }
            bool operator>(const iterator& other) const { return _index > other._index; }
            bool operator<=(const iterator& other) const { return _index <= other._index; }
            bool operator>=(const iterator& other) const { return _index >= other._index; }

        private:
            SegmentArray* _array;
            size_t _index;
        };

        class const_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            const_iterator(const SegmentArray* array, size_t index) : _array(array), _index(index)
            {
            }

            reference operator*() const { return (*_array)[_index]; }
            pointer operator->() const { return &(*_array)[_index]; }

            const_iterator& operator++()
            {
                ++_index;
                return *this;
            }

            const_iterator operator++(int)
            {
                const_iterator temp = *this;
                ++_index;
                return temp;
            }

            const_iterator& operator--()
            {
                --_index;
                return *this;
            }

            const_iterator operator--(int)
            {
                const_iterator temp = *this;
                --_index;
                return temp;
            }

            const_iterator operator+(size_t n) const
            {
                return const_iterator(_array, _index + n);
            }

            const_iterator operator-(size_t n) const
            {
                return const_iterator(_array, _index - n);
            }

            difference_type operator-(const const_iterator& other) const
            {
                return _index - other._index;
            }

            reference operator[](size_t n) const { return (*_array)[_index + n]; }

            bool operator==(const const_iterator& other) const { return _index == other._index; }
            bool operator!=(const const_iterator& other) const { return _index != other._index; }
            bool operator<(const const_iterator& other) const { return _index < other._index; }
            bool operator>(const const_iterator& other) const { return _index > other._index; }
            bool operator<=(const const_iterator& other) const { return _index <= other._index; }
            bool operator>=(const const_iterator& other) const { return _index >= other._index; }

        private:
            const SegmentArray* _array;
            size_t _index;
        };

        iterator begin() { return iterator(this, 0); }
        iterator end() { return iterator(this, size); }
        const_iterator begin() const { return const_iterator(this, 0); }
        const_iterator end() const { return const_iterator(this, size); }

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
