#pragma once

#include <cstring>
#include <cmath>
#include <cstdint>
#include "Internal/MathInternal.hpp"
#include "Ailurus/Assert.h"

namespace Ailurus
{
    template<typename ElementType, size_t Dimension>
    class Vector
    {
    public:
        Vector()
        {
            ::memset(_data, 0, Dimension * sizeof(ElementType));
        }

        Vector(std::initializer_list<ElementType> list)
        {
            ASSERT_MSG(list.size() == Dimension, "Initializer list size does not match vector dimension");
            std::copy(list.begin(), list.end(), _data);
        }

        template<typename... Args> requires ((sizeof...(Args) == Dimension))
        explicit Vector(Args... args)
        {
            size_t index = 0;
            (..., (_data[index++] = static_cast<ElementType>(args)));
        }

        Vector(const Vector& other)
        {
            ::memcpy(_data, other._data, Dimension * sizeof(ElementType));
        }

        Vector(Vector&& other) noexcept
        {
            ::memcpy(_data, other._data, Dimension * sizeof(ElementType));
        }

        Vector& operator=(const Vector& other)
        {
            if (this != &other)
                ::memcpy(_data, other._data, Dimension * sizeof(ElementType));

            return *this;
        }

        Vector& operator=(Vector&& other) noexcept
        {
            if (this != &other)
                ::memcpy(_data, other._data, Dimension * sizeof(ElementType));

            return *this;
        }

        template <typename T>
        explicit operator Vector<T, Dimension>() const
        {
            Vector<T, Dimension> result;

            for (auto i = 0; i < Dimension; i++)
                result[i] = static_cast<T>(_data[i]);

            return result;
        }

        ElementType& operator[](size_t index)
        {
            return _data[index];
        }

        const ElementType& operator[](size_t index) const
        {
            return _data[index];
        }

    public:
        ElementType* GetDataPtr()
        {
            return _data;
        }

        const ElementType* GetDataPtr() const
        {
            return _data;
        }

        size_t GetDimension() const
        {
            return Dimension;
        }

        ElementType SquareMagnitude() const
        {
            ElementType total = 0;
            for (auto i = 0; i < Dimension; i++)
                total += _data[i] * _data[i];

            return total;
        }

        ElementType Magnitude() const
        {
            return std::sqrt(SquareMagnitude());
        }

        template<typename T = ElementType>
        requires _internal::CanNormalize<T>
        void Normalize()
        {
            ElementType mag = Magnitude();
            if (mag > 0)
            {
                for (auto i = 0; i < Dimension; i++)
                    _data[i] = _data[i] / mag;
            }
        }

        template<typename T = ElementType>
        requires _internal::CanNormalize<T>
        Vector Normalized()
        {
            Vector result = *this;
            result.Normalize<>();
            return result;
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 1>
        ElementType& x()
        {
            return _data[0];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 1>
        ElementType x() const
        {
            return _data[0];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 2>
        ElementType& y()
        {
            return _data[1];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 2>
        ElementType y() const
        {
            return _data[1];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 3>
        ElementType& z()
        {
            return _data[2];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 3>
        ElementType z() const
        {
            return _data[2];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 4>
        ElementType& w()
        {
            return _data[3];
        }

        template <size_t D = Dimension> requires _internal::DimensionAtLeast<D, 4>
        ElementType w() const
        {
            return _data[3];
        }

        template<typename T = ElementType, size_t D = Dimension>
        requires _internal::CanDoCross<T, D>
        Vector Cross(const Vector& other) const
        {
            return Vector(y() * other.z() - z() * other.y(),
                          z() * other.x() - x() * other.z(),
                          x() * other.y() - y() * other.x());
        }

        ElementType
        Dot(const Vector& other) const
        {
            ElementType result = 0;
            for (auto i = 0; i < Dimension; i++)
                result += _data[i] * other[i];
            return result;
        }

    public:
        static const Vector Zero;
        static const Vector One;

    private:
        ElementType _data[Dimension];
    };

    template<typename ElementType, size_t Dimension>
    const Vector<ElementType, Dimension> Vector<ElementType, Dimension>::Zero = Vector<ElementType, Dimension>();

    template<typename ElementType, size_t Dimension>
    const Vector<ElementType, Dimension> Vector<ElementType, Dimension>::One = []
    {
        Vector vec;
        for (size_t i = 0; i < Dimension; ++i)
            vec[i] = static_cast<ElementType>(1);

        return vec;
    }();

    namespace _internal::Vector
    {
        enum class Op
        {
            Add,
            Sub,
            Multiply
        };

        template<typename E, size_t D, Op Operation, std::size_t... Indices>
        Vector<E, D> VectorOperationFoldExpression(
            const Vector<E, D>& left,
            const Vector<E, D>& right,
            std::index_sequence<Indices...>)
        {
            if constexpr (Operation == Op::Add)
                return Vector<E, D>(left[Indices] + right[Indices]...);
            else if constexpr (Operation == Op::Sub)
                return Vector<E, D>(left[Indices] - right[Indices]...);
            else if constexpr (Operation == Op::Multiply)
                return Vector<E, D>(left[Indices] * right[Indices]...);

            return {};
        }

        template<typename E1, typename E2, size_t D, Op Operation, std::size_t... Indices>
        Vector<E1, D> VectorOperationFoldExpression(
            const Vector<E1, D>& left,
            E2 right,
            std::index_sequence<Indices...>)
        {
            if constexpr (Operation == Op::Add)
                return Vector<E1, D>(left[Indices] + right...);
            else if constexpr (Operation == Op::Sub)
                return Vector<E1, D>(left[Indices] - right...);
            else if constexpr (Operation == Op::Multiply)
                return Vector<E1, D>(left[Indices] * right...);

            return {};
        }
    }

    template<typename E, size_t D>
    Vector<E, D> operator+(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        return _internal::Vector::VectorOperationFoldExpression<E, D, _internal::Vector::Op::Add>(
            left, right, std::make_index_sequence<D>());
    }

    template<typename E, size_t D>
    Vector<E, D>& operator+=(Vector<E, D>& left, const Vector<E, D>& right)
    {
        for (auto i = 0; i < D; i++)
            left[i] += right[i];
        return left;
    }

    template<typename E, size_t D>
    Vector<E, D> operator-(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        return _internal::Vector::VectorOperationFoldExpression<E, D, _internal::Vector::Op::Sub>(
            left, right, std::make_index_sequence<D>());
    }

    template<typename E, size_t D>
    Vector<E, D>& operator-=(Vector<E, D>& left, const Vector<E, D>& right)
    {
        for (auto i = 0; i < D; i++)
            left[i] -= right[i];
        return left;
    }

    template<typename E, size_t D>
    E operator*(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        return left.Dot(right);
    }

    template<typename E1, typename E2, size_t D>
    Vector<E1, D> operator*(const Vector<E1, D>& left, E2 right)
    {
        return _internal::Vector::VectorOperationFoldExpression<E1, E2, D, _internal::Vector::Op::Multiply>(
            left, right, std::make_index_sequence<D>());
    }

    template<typename E1, typename E2, size_t D>
    Vector<E1, D> operator*(E2 left, const Vector<E1, D>& right)
    {
        return _internal::Vector::VectorOperationFoldExpression<E1, E2, D, _internal::Vector::Op::Multiply>(
            right, left, std::make_index_sequence<D>());
    }

    template<typename E1, typename E2, size_t D>
    Vector<E1, D>& operator*=(Vector<E1, D>& left, E2 right)
    {
        for (auto i = 0; i < D; i++)
            left[i] *= right;
        return left;
    }

    template<typename E, size_t D> requires _internal::CanDoCross<E, D>
    Vector<E, D> operator^(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        return left.Cross(right);
    }

    template<typename E, size_t D>
    bool operator==(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        for (auto i = 0; i < D; i++)
        {
            if (left[i] != right[i])
                return false;
        }

        return true;
    }

    template<typename E, size_t D>
    bool operator!=(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        return !(left == right);
    }

    template <typename E>
    using Vector2 = Vector<E, 2>;

    template <typename E>
    using Vector3 = Vector<E, 3>;

    template <typename E>
    using Vector4 = Vector<E, 4>;

    using Vector2i = Vector<int32_t , 2>;
    using Vector2u = Vector<uint32_t, 2>;
    using Vector2f = Vector<float   , 2>;
    using Vector2d = Vector<double  , 2>;

    using Vector3i = Vector<int32_t , 3>;
    using Vector3u = Vector<uint32_t, 3>;
    using Vector3f = Vector<float   , 3>;
    using Vector3d = Vector<double  , 3>;

    using Vector4i = Vector<int32_t , 4>;
    using Vector4u = Vector<uint32_t, 4>;
    using Vector4f = Vector<float   , 4>;
    using Vector4d = Vector<double  , 4>;
}
