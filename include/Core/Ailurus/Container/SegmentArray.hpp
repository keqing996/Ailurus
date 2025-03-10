#pragma once

#include <array>

namespace Ailurus
{
    template<typename T, size_t Size, size_t SegmentSize>
    class SegmentArray
    {
        static_assert(SegmentSize > 0, "Segment size must larger than 0");

        static constexpr size_t SegmentPowerOfTwo = SegmentSize > 0 && (SegmentSize & (SegmentSize - 1)) == 0;

        static constexpr size_t CalculateUpPaddingArraySize()
        {
            size_t remainder;
            if constexpr (SegmentPowerOfTwo)
                remainder = Size & (SegmentSize - 1);
            else
                remainder = Size % SegmentSize;

            if (remainder == 0)
                return Size;

            return Size + (SegmentSize - remainder);
        }

        static constexpr size_t ArraySize = CalculateUpPaddingArraySize();
        static constexpr size_t SegmentCount = ArraySize / SegmentSize;
        static constexpr size_t SegmentCountPowerOfTwo = SegmentCount > 0 && (SegmentCount & (SegmentCount - 1)) == 0;

    public:
        SegmentArray()
            : _array()
        {
        }

        SegmentArray(std::initializer_list<T> init)
        {
            size_t index = 0;
            for (const T& value: init)
            {
                if (index < Size)
                {
                    _array[MappingIndexToSegment(index)] = value;
                    ++index;
                } else
                    break;
            }
        }

        SegmentArray(const SegmentArray& other)
            : _array(other._array)
        {
        }

        SegmentArray& operator=(const SegmentArray& other)
        {
            if (this != &other)
                _array = other._array;

            return *this;
        }

        SegmentArray(SegmentArray&& other) noexcept
            : _array(std::move(other._array))
        {
        }

        SegmentArray& operator=(SegmentArray&& other) noexcept
        {
            if (this != &other)
                _array = std::move(other._array);

            return *this;
        }

    public:
        size_t size() const
        {
            return Size;
        }

        T& operator[](uint32_t index)
        {
            return _array[MappingIndexToSegment(index)];
        }

        const T& operator[](uint32_t index) const
        {
            return _array[MappingIndexToSegment(index)];
        }

        const T* GetRawArray() const
        {
            return _array.data();
        }

        size_t GetInternalArraySize() const
        {
            return _array.size();
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
            reference operator*() { return (*_array)[_index]; }
            pointer operator->() { return &(*_array)[_index]; }

            bool operator==(const iterator& other) const { return _index == other._index; }
            bool operator!=(const iterator& other) const { return !(*this == other); }
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
            reference operator*() const { return (*_array)[_index]; }
            pointer operator->() const { return &(*_array)[_index]; }

            bool operator==(const const_iterator& other) const { return _index == other._index; }
            bool operator!=(const const_iterator& other) const { return !(*this == other); }
            bool operator<(const const_iterator& other) const { return _index < other._index; }
            bool operator>(const const_iterator& other) const { return _index > other._index; }
            bool operator<=(const const_iterator& other) const { return _index <= other._index; }
            bool operator>=(const const_iterator& other) const { return _index >= other._index; }

        private:
            const SegmentArray* _array;
            size_t _index;
        };

        class reverse_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            reverse_iterator(SegmentArray* array, size_t index) : _array(array), _index(index)
            {
            }

            reverse_iterator& operator++()
            {
                --_index;
                return *this;
            }

            reverse_iterator operator++(int)
            {
                reverse_iterator temp = *this;
                --_index;
                return temp;
            }

            reverse_iterator& operator--()
            {
                ++_index;
                return *this;
            }

            reverse_iterator operator--(int)
            {
                reverse_iterator temp = *this;
                ++_index;
                return temp;
            }

            reverse_iterator operator+(size_t n) const
            {
                return reverse_iterator(_array, _index - n);
            }

            reverse_iterator operator-(size_t n) const
            {
                return reverse_iterator(_array, _index + n);
            }

            difference_type operator-(const reverse_iterator& other) const
            {
                return other._index - _index;
            }

            reference operator[](size_t n) { return (*_array)[_index - n]; }
            reference operator*() { return (*_array)[_index]; }
            pointer operator->() { return &(*_array)[_index]; }

            bool operator==(const reverse_iterator& other) const { return _index == other._index; }
            bool operator!=(const reverse_iterator& other) const { return !(*this == other); }
            bool operator<(const reverse_iterator& other) const { return _index > other._index; }
            bool operator>(const reverse_iterator& other) const { return _index < other._index; }
            bool operator<=(const reverse_iterator& other) const { return _index >= other._index; }
            bool operator>=(const reverse_iterator& other) const { return _index <= other._index; }

        private:
            SegmentArray* _array;
            size_t _index;
        };

        class const_reverse_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            const_reverse_iterator(const SegmentArray* array, size_t index) : _array(array), _index(index)
            {
            }

            const_reverse_iterator& operator++()
            {
                --_index;
                return *this;
            }

            const_reverse_iterator operator++(int)
            {
                const_reverse_iterator temp = *this;
                --_index;
                return temp;
            }

            const_reverse_iterator& operator--()
            {
                ++_index;
                return *this;
            }

            const_reverse_iterator operator--(int)
            {
                const_reverse_iterator temp = *this;
                ++_index;
                return temp;
            }

            const_reverse_iterator operator+(size_t n) const
            {
                return const_reverse_iterator(_array, _index - n);
            }

            const_reverse_iterator operator-(size_t n) const
            {
                return const_reverse_iterator(_array, _index + n);
            }

            difference_type operator-(const const_reverse_iterator& other) const
            {
                return other._index - _index;
            }

            reference operator[](size_t n) const { return (*_array)[_index - n]; }
            reference operator*() const { return (*_array)[_index]; }
            pointer operator->() const { return &(*_array)[_index]; }

            bool operator==(const const_reverse_iterator& other) const { return _index == other._index; }
            bool operator!=(const const_reverse_iterator& other) const { return !(*this == other); }
            bool operator<(const const_reverse_iterator& other) const { return _index > other._index; }
            bool operator>(const const_reverse_iterator& other) const { return _index < other._index; }
            bool operator<=(const const_reverse_iterator& other) const { return _index >= other._index; }
            bool operator>=(const const_reverse_iterator& other) const { return _index <= other._index; }

        private:
            const SegmentArray* _array;
            size_t _index;
        };

        iterator                begin() { return iterator(this, 0); }
        iterator                end()   { return iterator(this, Size); }

        const_iterator          cbegin() const { return const_iterator(this, 0); }
        const_iterator          cend()   const { return const_iterator(this, Size); }

        const_iterator          begin() const { return cbegin(); }
        const_iterator          end()   const { return cend(); }

        reverse_iterator        rbegin() { return reverse_iterator(this, Size - 1); }
        reverse_iterator        rend()   { return reverse_iterator(this, static_cast<size_t>(-1)); }

        const_reverse_iterator  crbegin() const { return const_reverse_iterator(this, Size - 1); }
        const_reverse_iterator  crend()   const { return const_reverse_iterator(this, static_cast<size_t>(-1)); }

        const_reverse_iterator  rbegin() const { return crbegin(); }
        const_reverse_iterator  rend()   const { return crend(); }

    private:
        static uint32_t MappingIndexToSegment(uint32_t index)
        {
            uint32_t localIndex = index / SegmentCount;

            uint32_t segmentIndex;
            if constexpr (SegmentCountPowerOfTwo)
                segmentIndex = index & (SegmentCount - 1);
            else
                segmentIndex = index % SegmentCount;

            return segmentIndex * SegmentSize + localIndex;
        }

    private:
        std::array<T, ArraySize> _array;
    };
}
