#include <Ailurus/Application/RenderSystem/Uniform/UniformValue.h>

namespace Ailurus
{
	static void AssignUnion(UniformValue::Data& dest, const UniformValue::Data& src, UniformValueType type)
	{
		switch (type)
		{
			case UniformValueType::Int:
				dest.intValue = src.intValue;
				break;
			case UniformValueType::Float:
				dest.floatValue = src.floatValue;
				break;
			case UniformValueType::Vector2:
				dest.vector2Value = src.vector2Value;
				break;
			case UniformValueType::Vector3:
				dest.vector3Value = src.vector3Value;
				break;
			case UniformValueType::Vector4:
				dest.vector4Value = src.vector4Value;
				break;
			case UniformValueType::Mat4:
				dest.matrix4x4Value = src.matrix4x4Value;
				break;
		}
	}

	UniformValue::UniformValue() 
        : type(UniformValueType::Int)
        , data(0)
	{
		data.intValue = 0;
	}

	UniformValue::~UniformValue() {}

	UniformValue::UniformValue(int32_t intValue)
		: type(UniformValueType::Int)
        , data(0)
	{
		data.intValue = intValue;
	}

	UniformValue::UniformValue(float floatValue)
		: type(UniformValueType::Float)
        , data(0)
	{
		data.floatValue = floatValue;
	}

	UniformValue::UniformValue(const Vector2f& vector2Value)
		: type(UniformValueType::Vector2)
        , data(0)
	{
		data.vector2Value = vector2Value;
	}

	UniformValue::UniformValue(const Vector3f& vector3Value)
		: type(UniformValueType::Vector3)
        , data(0)
	{
		data.vector3Value = vector3Value;
	}

	UniformValue::UniformValue(const Vector4f& vector4Value)
		: type(UniformValueType::Vector4)
        , data(0)
	{
		data.vector4Value = vector4Value;
	}

	UniformValue::UniformValue(const Matrix4x4f& matrix4x4Value)
		: type(UniformValueType::Mat4)
        , data(0)
	{
		data.matrix4x4Value = matrix4x4Value;
	}

	UniformValue::UniformValue(const UniformValue& other)
		: data(0)
	{
		type = other.type;
		AssignUnion(data, other.data, type);
	}

	UniformValue& UniformValue::operator=(const UniformValue& other)
	{
		if (this != &other)
		{
			type = other.type;
			AssignUnion(data, other.data, type);
		}
		return *this;
	}

    const void* UniformValue::GetDataPointer() const
    {
        return &data;
	}

	UniformValueType UniformValue::GetType() const
	{
		return type;
	}

	uint32_t UniformValue::GetSize() const
	{
		return GetSize(type);
	}

	uint32_t UniformValue::GetSize(UniformValueType type)
	{
		switch (type)
		{
			case UniformValueType::Int:
				return sizeof(int32_t);
			case UniformValueType::Float:
				return sizeof(float);
			case UniformValueType::Vector2:
				return sizeof(Vector2f);
			case UniformValueType::Vector3:
				return sizeof(Vector3f);
			case UniformValueType::Vector4:
				return sizeof(Vector4f);
			case UniformValueType::Mat4:
				return sizeof(Matrix4x4f);
			default:
				return 0;
		}
	}

	void UniformValue::SetInt(int32_t value)
	{
		type = UniformValueType::Int;
		data.intValue = value;
	}

	void UniformValue::SetFloat(float value)
	{
		type = UniformValueType::Float;
		data.floatValue = value;
	}

	void UniformValue::SetVector2(const Vector2f& value)
	{
		type = UniformValueType::Vector2;
		data.vector2Value = value;
	}

	void UniformValue::SetVector3(const Vector3f& value)
	{
		type = UniformValueType::Vector3;
		data.vector3Value = value;
	}

	void UniformValue::SetVector4(const Vector4f& value)
	{
		type = UniformValueType::Vector4;
		data.vector4Value = value;
	}

	void UniformValue::SetMatrix4x4(const Matrix4x4f& value)
	{
		type = UniformValueType::Mat4;
		data.matrix4x4Value = value;
	}

	const UniformValue::Data& UniformValue::GetData() const
	{
		return data;
	}
} // namespace Ailurus