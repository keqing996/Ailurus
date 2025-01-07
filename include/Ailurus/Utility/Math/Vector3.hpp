#pragma once

#include <cmath>
#include <cstdint>

namespace Ailurus
{
    template<typename T>
    struct Vector3
    {
        union {
            struct {
                T x, y, z;
            };
            T data[3];
        };

        Vector3(): x(0), y(0), z(0) { }
        Vector3(T x, T y, T z): x(x), y(y), z(z) { }

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
        explicit operator Vector3<F>() const
        {
            return Vector3<F>(static_cast<F>(x), static_cast<F>(y), static_cast<F>(z));
        }

        T Magnitude() const
        {
            return std::sqrt(x * x + y * y + z * z);
        }

        Vector3 Normalize() const
        {
            float mag = Magnitude();
            if (mag > 0)
                return *this * (1.0f / mag);

            return *this;
        }

        Vector3 Cross(const Vector3& other) const
        {
            return Vector3(y * other.z - z * other.y,
                           z * other.x - x * other.z,
                           x * other.y - y * other.x);
        }
    };

    template <typename T>
    Vector3<T> operator+(const Vector3<T>& left, const Vector3<T>& right)
    {
        return Vector3<T>(left.x + right.x, left.y + right.y, left.z + right.z);
    }

    template <typename T>
    Vector3<T>& operator+=(Vector3<T>& left, const Vector3<T>& right)
    {
        left.x += right.x;
        left.y += right.y;
        left.z += right.z;
        return left;
    }

    template <typename T>
    Vector3<T> operator-(const Vector3<T>& left, const Vector3<T>& right)
    {
        return Vector3<T>(left.x - right.x, left.y - right.y, left.z - right.z);
    }

    template <typename T>
    Vector3<T>& operator-=(Vector3<T>& left, const Vector3<T>& right)
    {
        left.x -= right.x;
        left.y -= right.y;
        left.z -= right.z;
        return left;
    }

    template <typename T>
    T operator*(const Vector3<T>& left, const Vector3<T>& right)
    {
        return left.x * right.x + left.y * right.y + left.z * right.z;
    }

    template <typename T>
    Vector3<T> operator*(const Vector3<T>& left, T right)
    {
        return Vector3<T>(left.x * right, left.y * right, left.z * right.z);
    }

    template <typename T>
    Vector3<T> operator*(T left, const Vector3<T>& right)
    {
        return Vector3<T>(right.x * left, right.y * left, right.z * left);
    }

    template <typename T>
    Vector3<T>& operator*=(Vector3<T>& left, T right)
    {
        left.x *= right;
        left.y *= right;
        left.z *= right;
        return left;
    }

    template <typename T>
    Vector3<T> operator/(const Vector3<T>& left, T right)
    {
        return Vector3<T>(left.x / right, left.y / right, left.z / right);
    }

    template <typename T>
    Vector3<T>& operator/=(Vector3<T>& left, T right)
    {
        left.x /= right;
        left.y /= right;
        left.z /= right;
        return left;
    }

    template <typename T>
    bool operator==(const Vector3<T>& left, const Vector3<T>& right)
    {
        return left.x == right.x && left.y == right.y && left.z == right.z;
    }

    template <typename T>
    bool operator!=(const Vector3<T>& left, const Vector3<T>& right)
    {
        return !(left == right);
    }

    using Vector3i = Vector3<int32_t>;
    using Vector3u = Vector3<uint32_t>;
    using Vector3f = Vector3<float>;
    using Vector3d = Vector3<double>;
}