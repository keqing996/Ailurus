#pragma once

#include <array>

namespace Ailurus
{
    /**
     * @brief A segmented array container that provides cache-friendly memory layout
     * 
     * SegmentArray is a fixed-size container template that organizes elements in segments
     * to optimize memory access patterns. Unlike standard arrays, it maps logical indices
     * to a segmented physical layout, which can improve cache locality for certain access
     * patterns, particularly beneficial in scenarios with interleaved data access.
     * 
     * Key Features:
     * - Compile-time fixed size with automatic padding for segment alignment
     * - Cache-optimized memory layout through segmentation
     * - Full iterator support (forward, reverse, const variants)
     * - Optimized operations for power-of-2 segment sizes using bit operations
     * - STL-compatible interface with random access semantics
     * 
     * Memory Layout:
     * The container internally uses a std::array with size padded up to the nearest
     * multiple of SegmentSize. Elements are accessed through a mapping function that
     * converts linear indices to segmented positions.
     * 
     * Use Cases:
     * - Matrix operations with block-wise access patterns
     * - Cache-sensitive algorithms requiring spatial locality
     * - Data structures benefiting from segment-based memory organization
     * 
     * @tparam T Element type
     * @tparam Size Number of logical elements in the array
     * @tparam SegmentSize Size of each memory segment (must be > 0)
     * 
     * @note For best performance, use power-of-2 segment sizes to enable
     *       bit manipulation optimizations
     */
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
        /**
         * @brief Default constructor - creates an empty segmented array
         * 
         * Initializes all elements with their default constructor.
         * For primitive types, elements are zero-initialized.
         */
        SegmentArray()
            : _array()
        {
        }

        /**
         * @brief Initializer list constructor
         * 
         * Constructs the segmented array with elements from an initializer list.
         * If the initializer list has fewer elements than Size, remaining elements
         * are default-initialized. Extra elements beyond Size are ignored.
         * 
         * @param init Initializer list containing initial values
         * 
         * Example:
         * @code
         * SegmentArray<int, 5, 2> arr{1, 2, 3, 4, 5};
         * @endcode
         */
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

        /**
         * @brief Copy constructor
         * 
         * Creates a new SegmentArray as a copy of another SegmentArray.
         * Performs deep copy of all elements.
         * 
         * @param other The SegmentArray to copy from
         */
        SegmentArray(const SegmentArray& other)
            : _array(other._array)
        {
        }

        /**
         * @brief Copy assignment operator
         * 
         * Assigns the contents of another SegmentArray to this one.
         * Performs deep copy with self-assignment protection.
         * 
         * @param other The SegmentArray to copy from
         * @return Reference to this SegmentArray
         */
        SegmentArray& operator=(const SegmentArray& other)
        {
            if (this != &other)
                _array = other._array;

            return *this;
        }

        /**
         * @brief Move constructor
         * 
         * Constructs the SegmentArray by moving from another SegmentArray.
         * The source object is left in a valid but unspecified state.
         * 
         * @param other The SegmentArray to move from
         */
        SegmentArray(SegmentArray&& other) noexcept
            : _array(std::move(other._array))
        {
        }

        /**
         * @brief Move assignment operator
         * 
         * Moves the contents of another SegmentArray to this one.
         * The source object is left in a valid but unspecified state.
         * 
         * @param other The SegmentArray to move from
         * @return Reference to this SegmentArray
         */
        SegmentArray& operator=(SegmentArray&& other) noexcept
        {
            if (this != &other)
                _array = std::move(other._array);

            return *this;
        }

    public:
        /**
         * @brief Returns the number of elements in the array
         * 
         * @return The logical size of the array (template parameter Size)
         * 
         * @note This returns the logical size, not the internal padded array size.
         *       Use GetInternalArraySize() to get the actual allocated size.
         */
        size_t size() const
        {
            return Size;
        }

        /**
         * @brief Access element at specified index (non-const version)
         * 
         * Provides direct access to the element at the given logical index.
         * The index is automatically mapped to the appropriate segment position.
         * 
         * @param index Logical index of the element (0-based)
         * @return Reference to the element at the specified index
         * 
         * @warning No bounds checking is performed. Accessing out-of-bounds
         *          indices results in undefined behavior.
         */
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
