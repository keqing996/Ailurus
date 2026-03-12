#pragma once

#include "Vector3.hpp"
#include "Matrix4x4.hpp"
#include "AABB.hpp"

namespace Ailurus
{
	struct FrustumPlane
	{
		Vector3f normal;
		float distance;

		float SignedDistance(const Vector3f& point) const
		{
			return normal.Dot(point) + distance;
		}
	};

	struct Frustum
	{
		// Near, Far, Left, Right, Top, Bottom
		FrustumPlane planes[6];

		// Extract 6 frustum planes from a View-Projection matrix (Gribb-Hartmann method)
		static Frustum FromViewProjection(const Matrix4x4f& vp)
		{
			Frustum frustum;

			Vector4f r0 = vp.GetRow(0);
			Vector4f r1 = vp.GetRow(1);
			Vector4f r2 = vp.GetRow(2);
			Vector4f r3 = vp.GetRow(3);

			// Left:   row3 + row0
			auto setPlane = [](FrustumPlane& plane, const Vector4f& v)
			{
				float length = Vector3f(v.x, v.y, v.z).Magnitude();
				if (length > 0.0f)
				{
					float invLen = 1.0f / length;
					plane.normal = Vector3f(v.x, v.y, v.z) * invLen;
					plane.distance = v.w * invLen;
				}
			};

			setPlane(frustum.planes[0], r3 + r2); // Near
			setPlane(frustum.planes[1], r3 - r2); // Far
			setPlane(frustum.planes[2], r3 + r0); // Left
			setPlane(frustum.planes[3], r3 - r0); // Right
			setPlane(frustum.planes[4], r3 - r1); // Top
			setPlane(frustum.planes[5], r3 + r1); // Bottom

			return frustum;
		}

		// Test if an AABB intersects the frustum (conservative: no false negatives)
		bool Intersects(const AABBf& aabb) const
		{
			for (const auto& plane : planes)
			{
				// Find the p-vertex: the AABB corner most in the direction of the plane normal
				Vector3f pVertex;
				pVertex.x = (plane.normal.x >= 0.0f) ? aabb.max.x : aabb.min.x;
				pVertex.y = (plane.normal.y >= 0.0f) ? aabb.max.y : aabb.min.y;
				pVertex.z = (plane.normal.z >= 0.0f) ? aabb.max.z : aabb.min.z;

				// If the p-vertex is on the negative side, the AABB is fully outside this plane
				if (plane.SignedDistance(pVertex) < 0.0f)
					return false;
			}
			return true;
		}
	};
} // namespace Ailurus
