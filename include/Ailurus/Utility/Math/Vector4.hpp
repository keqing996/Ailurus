#pragma once

#include <cmath>
#include <cstdint>

namespace Ailurus
{
    template<typename T>
    struct Vector4
    {
        union {
            struct {
                T x, y, z, w;
            };
            T data[4];
        };

        Vector4(): x(0), y(0), z(0), w(0) { }
        Vector4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) { }

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
        explicit operator Vector4<F>() const
        {
            return Vector4<F>(static_cast<F>(x), static_cast<F>(y)
                , static_cast<F>(z), static_cast<F>(w));
        }

        T Magnitude() const
        {
            return std::sqrt(x * x + y * y + z * z + w * w);
        }

        Vector4 Normalize() const
        {
            float mag = Magnitude();
            if (mag > 0)
                return *this * (1.0f / mag);

            return *this;
        }
    };

    template <typename T>
    Vector4<T> operator+(const Vector4<T>& left, const Vector4<T>& right)
    {
        return Vector4<T>(left.x + right.x,
            left.y + right.y,
            left.z + right.z,
            left.w + right.w);
    }

    template <typename T>
    Vector4<T>& operator+=(Vector4<T>& left, const Vector4<T>& right)
    {
        left.x += right.x;
        left.y += right.y;
        left.z += right.z;
        left.w += right.w;
        return left;
    }

    template <typename T>
    Vector4<T> operator-(const Vector4<T>& left, const Vector4<T>& right)
    {
        return Vector4<T>(left.x - right.x,
            left.y - right.y,
            left.z - right.z,
            left.w - right.w);
    }

    template <typename T>
    Vector4<T>& operator-=(Vector4<T>& left, const Vector4<T>& right)
    {
        left.x -= right.x;
        left.y -= right.y;
        left.z -= right.z;
        left.w -= right.w;
        return left;
    }

    template <typename T>
    T operator*(const Vector4<T>& left, const Vector4<T>& right)
    {
        return left.x * right.x
            + left.y * right.y
            + left.z * right.z
            + left.w * right.w;
    }

    template <typename T>
    Vector4<T> operator*(const Vector4<T>& left, T right)
    {
        return Vector4<T>(left.x * right,
            left.y * right,
            left.z * right,
            left.w * right.w);
    }

    template <typename T>
    Vector4<T> operator*(T left, const Vector4<T>& right)
    {
        return Vector4<T>(right.x * left,
            right.y * left,
            right.z * left,
            right.w * left);
    }

    template <typename T>
    Vector4<T>& operator*=(Vector4<T>& left, T right)
    {
        left.x *= right;
        left.y *= right;
        left.z *= right;
        left.w *= right;
        return left;
    }

    template <typename T>
    Vector4<T> operator/(const Vector4<T>& left, T right)
    {
        return Vector4<T>(left.x / right,
            left.y / right,
            left.z / right,
            left.w / right);
    }

    template <typename T>
    Vector4<T>& operator/=(Vector4<T>& left, T right)
    {
        left.x /= right;
        left.y /= right;
        left.z /= right;
        left.w /= right;
        return left;
    }

    template <typename T>
    bool operator==(const Vector4<T>& left, const Vector4<T>& right)
    {
        return left.x == right.x
            && left.y == right.y
            && left.z == right.z
            && left.w == right.w;
    }

    template <typename T>
    bool operator!=(const Vector4<T>& left, const Vector4<T>& right)
    {
        return !(left == right);
    }

    using Vector4i = Vector4<int32_t>;
    using Vector4u = Vector4<uint32_t>;
    using Vector4f = Vector4<float>;
    using Vector4d = Vector4<double>;
}