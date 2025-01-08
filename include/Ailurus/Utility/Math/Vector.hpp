#pragma once

#include <cmath>
#include <cstdint>

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

        template<typename... Args>
        explicit Vector(Args... args)
        {
            static_assert(sizeof...(Args) == Dimension, "Parameter count must match vector's dimension");

            size_t index = 0;
            (..., (_data[index++] = static_cast<ElementType>(args)));
        }

        template <typename T>
        explicit operator Vector<T, Dimension>() const
        {
            Vector<T, Dimension> result;

            for (auto i = 0; i < Dimension; i++)
                result._data[i] = static_cast<T>(_data[i]);

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
            ElementType total;
            for (auto i = 0; i < Dimension; i++)
                total = _data[i] * _data[i];

            return total;
        }

        ElementType Magnitude() const
        {
            return std::sqrt(SquareMagnitude());
        }

        Vector Normalize() const
        {
            static_assert(std::is_floating_point_v<ElementType>, "Normalize() only support floating elements.");

            ElementType mag = Magnitude();
            if (mag > 0)
            {
                for (auto i = 0; i < Dimension; i++)
                    _data[i] = _data[i] / mag;
            }

            return *this;
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 1), ElementType&>
        x()
        {
            return _data[0];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 1), ElementType>
        x() const
        {
            return _data[0];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 2), ElementType&>
        y()
        {
            return _data[1];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 2), ElementType>
        y() const
        {
            return _data[1];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 3), ElementType&>
        z()
        {
            return _data[2];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 3), ElementType>
        z() const
        {
            return _data[2];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 4), ElementType&>
        w()
        {
            return _data[3];
        }

        template <size_t D = Dimension>
        typename std::enable_if_t<(D >= 4), ElementType>
        w() const
        {
            return _data[3];
        }

    private:
        ElementType _data[Dimension];
    };

    namespace _internal
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

        template<typename E, size_t D, Op Operation, std::size_t... Indices>
        Vector<E, D> VectorOperationFoldExpression(
            const Vector<E, D>& left,
            E right,
            std::index_sequence<Indices...>)
        {
            if constexpr (Operation == Op::Add)
                return Vector<E, D>(left[Indices] + right...);
            else if constexpr (Operation == Op::Sub)
                return Vector<E, D>(left[Indices] - right...);
            else if constexpr (Operation == Op::Multiply)
                return Vector<E, D>(left[Indices] * right...);

            return {};
        }
    }

    template<typename E, size_t D>
    Vector<E, D> operator+(const Vector<E, D>& left, const Vector<E, D>& right)
    {
        return _internal::VectorOperationFoldExpression<E, D, _internal::Op::Add>(
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
        return _internal::VectorOperationFoldExpression<E, D, _internal::Op::Sub>(
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
        E result;
        for (auto i = 0; i < D; i++)
            result += left[i] * right[i];
        return result;
    }

    template<typename E, size_t D>
    Vector<E, D> operator*(const Vector<E, D>& left, E right)
    {
        return _internal::VectorOperationFoldExpression<E, D, _internal::Op::Multiply>(
            left, right, std::make_index_sequence<D>());
    }

    template<typename E, size_t D>
    Vector<E, D> operator*(E left, const Vector<E, D>& right)
    {
        return _internal::VectorOperationFoldExpression<E, D, _internal::Op::Multiply>(
            right, left, std::make_index_sequence<D>());
    }

    template<typename E, size_t D>
    Vector<E, D>& operator*=(Vector<E, D>& left, const Vector<E, D>& right)
    {
        for (auto i = 0; i < D; i++)
            left[i] *= right[i];
        return left;
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
