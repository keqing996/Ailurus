#pragma once

#include <cstdint>
#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Math/Vector2.hpp"
#include "Ailurus/Math/Vector3.hpp"
#include "Ailurus/Math/Vector4.hpp"
#include "Ailurus/Math/Matrix4x4.hpp"

namespace Ailurus
{
    REFLECTION_ENUM(UniformValueType,
		Int,
		Float,
		Vector2,
		Vector3,
		Vector4,
		Mat4);

	class UniformValue
	{
	public:
		union Data
		{
			int32_t intValue;
			float floatValue;
			Vector2f vector2Value;
			Vector3f vector3Value;
			Vector4f vector4Value;
			Matrix4x4f matrix4x4Value;
		};

	public:
		UniformValue();
		~UniformValue();

		UniformValue(int32_t intValue);
		UniformValue(float floatValue);
		UniformValue(const Vector2f& vector2Value);
		UniformValue(const Vector3f& vector3Value);
		UniformValue(const Vector4f& vector4Value);
		UniformValue(const Matrix4x4f& matrix4x4Value);

		UniformValue(const UniformValue& other);
		UniformValue& operator=(const UniformValue& other);

	public:
		auto GetDataPointer() const -> const void*;
		auto GetType() const -> UniformValueType;
		auto GetSize() const -> uint32_t;
		auto GetData() const -> const Data&;

		void SetInt(int32_t value);
		void SetFloat(float value);
		void SetVector2(const Vector2f& value);
		void SetVector3(const Vector3f& value);
		void SetVector4(const Vector4f& value);
		void SetMatrix4x4(const Matrix4x4f& value);

	public:
		static uint32_t GetSize(UniformValueType type);

	private:
		UniformValueType type;
		Data data;
	};
}