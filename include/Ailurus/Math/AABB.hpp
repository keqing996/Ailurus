#pragma once

#include <cfloat>
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix4x4.hpp"

namespace Ailurus
{
	template <typename T>
	struct AABB
	{
		Vector3<T> min;
		Vector3<T> max;

		AABB()
			: min(T(0), T(0), T(0))
			, max(T(0), T(0), T(0))
		{
		}

		AABB(const Vector3<T>& min, const Vector3<T>& max)
			: min(min)
			, max(max)
		{
		}

		Vector3<T> GetCenter() const { return (min + max) * T(0.5); }
		Vector3<T> GetExtents() const { return (max - min) * T(0.5); }

		AABB Transform(const Matrix4x4<T>& matrix) const
		{
			// Transform all 8 corners and recompute min/max
			Vector3<T> corners[8] = {
				{ min.x, min.y, min.z },
				{ max.x, min.y, min.z },
				{ min.x, max.y, min.z },
				{ max.x, max.y, min.z },
				{ min.x, min.y, max.z },
				{ max.x, min.y, max.z },
				{ min.x, max.y, max.z },
				{ max.x, max.y, max.z },
			};

			Vector3<T> newMin(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
			Vector3<T> newMax(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

			for (const auto& corner : corners)
			{
				Vector4<T> transformed = matrix * Vector4<T>(corner.x, corner.y, corner.z, T(1));
				Vector3<T> pos(transformed.x, transformed.y, transformed.z);
				newMin = Vector3<T>::Min(newMin, pos);
				newMax = Vector3<T>::Max(newMax, pos);
			}

			return { newMin, newMax };
		}

		static AABB Merge(const AABB& a, const AABB& b)
		{
			return {
				Vector3<T>::Min(a.min, b.min),
				Vector3<T>::Max(a.max, b.max)
			};
		}
	};

	using AABBf = AABB<float>;
	using AABBd = AABB<double>;
} // namespace Ailurus
