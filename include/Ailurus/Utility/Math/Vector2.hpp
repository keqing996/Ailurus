#pragma once

#include <cmath>
#include <cstdint>

namespace Ailurus
{
    template<typename T>
    struct Vector2
    {
        union {
            struct {
                T x, y;
            };
            T data[2];
        };

        Vector2(): x(0), y(0) { }
        Vector2(T x, T y) : x(x), y(y) { }

        T* GetDataPtr()
        {
            return data;
        }

        const T* GetDataPtr() const
        {
            return data;
        }

        T& operator[](size_t index)
        {
            return data[index];
        }

        const T& operator[](size_t index) const
        {
            return data[index];
        }

        template <typename F>
        explicit operator Vector2<F>() const
        {
            return Vector2<F>(static_cast<F>(x), static_cast<F>(y));
        }

        T Magnitude() const
        {
            return std::sqrt(x * x + y * y);
        }

        Vector2 Normalize() const
        {
            float mag = Magnitude();
            if (mag > 0)
                return *this * (1.0f / mag);

            return *this;
        }
    };

    template <typename T>
    Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x + right.x, left.y + right.y);
    }

    template <typename T>
    Vector2<T>& operator+=(Vector2<T>& left, const Vector2<T>& right)
    {
        left.x += right.x;
        left.y += right.y;
        return left;
    }

    template <typename T>
    Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x - right.x, left.y - right.y);
    }

    template <typename T>
    Vector2<T>& operator-=(Vector2<T>& left, const Vector2<T>& right)
    {
        left.x -= right.x;
        left.y -= right.y;
        return left;
    }

    template <typename T>
    T operator*(const Vector2<T>& left, const Vector2<T>& right)
    {
        return left.x * right.x + left.y * right.y;
    }

    template <typename T>
    Vector2<T> operator*(const Vector2<T>& left, T right)
    {
        return Vector2<T>(left.x * right, left.y * right);
    }

    template <typename T>
    Vector2<T> operator*(T left, const Vector2<T>& right)
    {
        return Vector2<T>(right.x * left, right.y * left);
    }

    template <typename T>
    Vector2<T>& operator*=(Vector2<T>& left, T right)
    {
        left.x *= right;
        left.y *= right;
        return left;
    }

    template <typename T>
    Vector2<T> operator/(const Vector2<T>& left, T right)
    {
        return Vector2<T>(left.x / right, left.y / right);
    }

    template <typename T>
    Vector2<T>& operator/=(Vector2<T>& left, T right)
    {
        left.x /= right;
        left.y /= right;
        return left;
    }

    template <typename T>
    bool operator==(const Vector2<T>& left, const Vector2<T>& right)
    {
        return left.x == right.x && left.y == right.y;
    }

    template <typename T>
    bool operator!=(const Vector2<T>& left, const Vector2<T>& right)
    {
        return !(left == right);
    }

    using Vector2i = Vector2<int32_t>;
    using Vector2u = Vector2<uint32_t>;
    using Vector2f = Vector2<float>;
    using Vector2d = Vector2<double>;
}



