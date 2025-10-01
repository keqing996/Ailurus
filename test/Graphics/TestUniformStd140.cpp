#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Vector2.hpp>
#include <Ailurus/Math/Vector3.hpp>
#include <Ailurus/Math/Vector4.hpp>
#include <Ailurus/Math/Matrix4x4.hpp>
#include "Ailurus/Application/RenderSystem/Uniform/UniformLayoutHelper.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformVariable.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"

using namespace Ailurus;

// Test std140 alignment rules using the project's UniformLayoutHelper
TEST_SUITE("Std140LayoutRules")
{
    TEST_CASE("Basic Type Sizes")
    {
        SUBCASE("Scalar types")
        {
            // In std140, scalars are 4 bytes
            CHECK_EQ(sizeof(int32_t), 4);
            CHECK_EQ(sizeof(float), 4);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Int), 4);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Float), 4);
        }

        SUBCASE("Vector types")
        {
            // Vector2: 8 bytes (2 * 4)
            CHECK_EQ(sizeof(Vector2f), 8);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Vector2), 8);
            
            // Vector3: 12 bytes in memory, but aligned to 16 bytes in std140
            CHECK_EQ(sizeof(Vector3f), 12);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Vector3), 12);
            
            // Vector4: 16 bytes
            CHECK_EQ(sizeof(Vector4f), 16);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Vector4), 16);
        }

        SUBCASE("Matrix types")
        {
            // Matrix4x4: 64 bytes (16 * 4)
            CHECK_EQ(sizeof(Matrix4x4f), 64);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Mat4), 64);
        }
    }

    TEST_CASE("Std140 Base Alignment Rules")
    {
        SUBCASE("Scalar alignment")
        {
            // Test alignment calculation using UniformLayoutHelper
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Int), 4);
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Float), 4);
            
            // Test AlignOffset function
            CHECK_EQ(UniformLayoutHelper::AlignOffset(0, 4), 0);
            CHECK_EQ(UniformLayoutHelper::AlignOffset(1, 4), 4);
            CHECK_EQ(UniformLayoutHelper::AlignOffset(5, 4), 8);
        }

        SUBCASE("Vector alignment")
        {
            // Vector2 has base alignment of 8 bytes
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector2), 8);
            
            // Vector3 and Vector4 have base alignment of 16 bytes
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector3), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector4), 16);
            
            // Test alignment calculations
            CHECK_EQ(UniformLayoutHelper::AlignOffset(0, 16), 0);
            CHECK_EQ(UniformLayoutHelper::AlignOffset(12, 16), 16);
            CHECK_EQ(UniformLayoutHelper::AlignOffset(17, 16), 32);
        }

        SUBCASE("Matrix alignment")
        {
            // Matrix4x4 has base alignment of 16 bytes
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Mat4), 16);
        }
    }

    TEST_CASE("Structure Layout Calculation")
    {
        SUBCASE("Simple structure: float + vec3 + float")
        {
            // struct { float a; vec3 b; float c; }
            std::vector<UniformValueType> memberTypes = {
                UniformValueType::Float,
                UniformValueType::Vector3,
                UniformValueType::Float
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(memberTypes, offsets);
            
            // Expected layout:
            // float a: at offset 0, size 4
            // vec3 b: at offset 16 (aligned to 16), size 12
            // float c: at offset 28, size 4
            // total: 32 (aligned to 16)
            
            CHECK_EQ(offsets.size(), 3);
            CHECK_EQ(offsets[0], 0);   // float a
            CHECK_EQ(offsets[1], 16);  // vec3 b (aligned to 16)
            CHECK_EQ(offsets[2], 28);  // float c
            CHECK_EQ(totalSize, 32);   // total size aligned to 16
        }

        SUBCASE("Complex structure: vec2 + float + vec4")
        {
            // struct { vec2 a; float b; vec4 c; }
            std::vector<UniformValueType> memberTypes = {
                UniformValueType::Vector2,
                UniformValueType::Float,
                UniformValueType::Vector4
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(memberTypes, offsets);
            
            // Expected layout:
            // vec2 a: at offset 0, size 8
            // float b: at offset 8, size 4
            // vec4 c: at offset 16 (aligned to 16), size 16
            // total: 32 (aligned to 16)
            
            CHECK_EQ(offsets.size(), 3);
            CHECK_EQ(offsets[0], 0);   // vec2 a
            CHECK_EQ(offsets[1], 8);   // float b
            CHECK_EQ(offsets[2], 16);  // vec4 c (aligned to 16)
            CHECK_EQ(totalSize, 32);   // total size aligned to 16
        }
    }

    TEST_CASE("Array Layout Calculation")
    {
        SUBCASE("Array of floats")
        {
            // In std140, array elements are aligned to 16-byte boundaries
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Float, 3, offsets);
            
            CHECK_EQ(offsets.size(), 3);
            CHECK_EQ(offsets[0], 0);   // First float at 0
            CHECK_EQ(offsets[1], 16);  // Second float at 16
            CHECK_EQ(offsets[2], 32);  // Third float at 32
            CHECK_EQ(totalSize, 48);   // 3 * 16 = 48
        }

        SUBCASE("Array of vec3")
        {
            // Each vec3 in array is aligned to 16 bytes
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Vector3, 2, offsets);
            
            CHECK_EQ(offsets.size(), 2);
            CHECK_EQ(offsets[0], 0);   // First vec3 at 0
            CHECK_EQ(offsets[1], 16);  // Second vec3 at 16
            CHECK_EQ(totalSize, 32);   // 2 * 16 = 32
        }

        SUBCASE("Array of vec4")
        {
            // Each vec4 in array is naturally aligned to 16 bytes
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Vector4, 3, offsets);
            
            CHECK_EQ(offsets.size(), 3);
            CHECK_EQ(offsets[0], 0);   // First vec4 at 0
            CHECK_EQ(offsets[1], 16);  // Second vec4 at 16
            CHECK_EQ(offsets[2], 32);  // Third vec4 at 32
            CHECK_EQ(totalSize, 48);   // 3 * 16 = 48
        }
    }

    TEST_CASE("Array Stride Calculation")
    {
        SUBCASE("Scalar array stride")
        {
            // float array stride should be 16 bytes in std140
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Float), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Int), 16);
        }

        SUBCASE("Vector array stride")
        {
            // vec2 array stride should be 16 bytes
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector2), 16);
            
            // vec3 array stride should be 16 bytes (12 bytes + 4 padding)
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector3), 16);
            
            // vec4 array stride should be 16 bytes
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector4), 16);
        }

        SUBCASE("Matrix array stride")
        {
            // mat4 array stride should be 64 bytes
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Mat4), 64);
        }
    }

    TEST_CASE("Real World Examples")
    {
        SUBCASE("Camera UBO layout")
        {
            // struct CameraUBO {
            //     mat4 view;       // 64 bytes at offset 0
            //     mat4 projection; // 64 bytes at offset 64
            //     vec3 viewPos;    // 12 bytes at offset 128
            //     float padding;   // 4 bytes at offset 140
            // };
            
            std::vector<UniformValueType> cameraTypes = {
                UniformValueType::Mat4,      // view
                UniformValueType::Mat4,      // projection
                UniformValueType::Vector3,   // viewPos
                UniformValueType::Float      // padding
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(cameraTypes, offsets);
            
            CHECK_EQ(offsets[0], 0);    // view matrix
            CHECK_EQ(offsets[1], 64);   // projection matrix
            CHECK_EQ(offsets[2], 128);  // viewPos
            CHECK_EQ(offsets[3], 140);  // padding
            CHECK_EQ(totalSize, 144);   // total size
        }

        SUBCASE("Light UBO layout")
        {
            // struct LightUBO {
            //     vec3 position;    // 12 bytes at offset 0
            //     float intensity;  // 4 bytes at offset 12
            //     vec3 color;       // 12 bytes at offset 16
            //     float radius;     // 4 bytes at offset 28
            // };
            
            std::vector<UniformValueType> lightTypes = {
                UniformValueType::Vector3,   // position
                UniformValueType::Float,     // intensity
                UniformValueType::Vector3,   // color
                UniformValueType::Float      // radius
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(lightTypes, offsets);
            
            CHECK_EQ(offsets[0], 0);   // position
            CHECK_EQ(offsets[1], 12);  // intensity
            CHECK_EQ(offsets[2], 16);  // color (aligned to 16)
            CHECK_EQ(offsets[3], 28);  // radius
            CHECK_EQ(totalSize, 32);   // total size
        }
    }

    TEST_CASE("Layout Description")
    {
        SUBCASE("Generate layout description")
        {
            std::vector<UniformValueType> types = {
                UniformValueType::Float,
                UniformValueType::Vector3,
                UniformValueType::Float
            };
            
            std::vector<std::string> names = { "floatA", "vec3B", "floatC" };
            
            std::string description = UniformLayoutHelper::GetLayoutDescription(types, names);
            
            // Check that description contains expected information
            CHECK(description.find("floatA") != std::string::npos);
            CHECK(description.find("vec3B") != std::string::npos);
            CHECK(description.find("floatC") != std::string::npos);
            CHECK(description.find("Total Size: 32") != std::string::npos);
        }
    }

    TEST_CASE("Vector3 Padding Issue")
    {
        SUBCASE("Vector3 memory layout verification")
        {
            Vector3f vec3(1.0f, 2.0f, 3.0f);
            
            // Verify the actual size in memory vs std140 requirements
            CHECK_EQ(sizeof(vec3), 12);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Vector3), 12);
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector3), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector3), 16);
            
            // Verify data content
            const float* data = reinterpret_cast<const float*>(&vec3);
            CHECK_EQ(data[0], 1.0f);
            CHECK_EQ(data[1], 2.0f);
            CHECK_EQ(data[2], 3.0f);
            // Note: In actual std140 buffer layout, there would be 4 bytes of padding after data[2]
        }

        SUBCASE("Matrix transpose for GPU")
        {
            // Test that matrices need to be transposed for column-major GPU layout
            Matrix4x4f mat;
            mat(0, 0) = 1.0f;  mat(0, 1) = 2.0f;  mat(0, 2) = 3.0f;  mat(0, 3) = 4.0f;
            mat(1, 0) = 5.0f;  mat(1, 1) = 6.0f;  mat(1, 2) = 7.0f;  mat(1, 3) = 8.0f;
            mat(2, 0) = 9.0f;  mat(2, 1) = 10.0f; mat(2, 2) = 11.0f; mat(2, 3) = 12.0f;
            mat(3, 0) = 13.0f; mat(3, 1) = 14.0f; mat(3, 2) = 15.0f; mat(3, 3) = 16.0f;
            
            // Matrix should be stored properly for GPU
            CHECK_EQ(sizeof(mat), 64);
            CHECK_EQ(UniformValue::GetSize(UniformValueType::Mat4), 64);
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Mat4), 16);
            
            // Check if the matrix needs transposing for GPU upload
            Matrix4x4f transposed = mat.Transpose();
            const float* transposeData = reinterpret_cast<const float*>(&transposed);
            
            // First column of transposed matrix (was first row)
            CHECK_EQ(transposeData[0], 1.0f);   // mat(0,0)
            CHECK_EQ(transposeData[1], 5.0f);   // mat(1,0)
            CHECK_EQ(transposeData[2], 9.0f);   // mat(2,0)
            CHECK_EQ(transposeData[3], 13.0f);  // mat(3,0)
        }
    }

    TEST_CASE("Integration with UniformBindingPoint")
    {
        SUBCASE("Array binding point validation")
        {
            // Create an array of float uniforms
            auto arrayUniform = std::make_unique<UniformVariableArray>();
            
            // Add 3 float elements
            for (int i = 0; i < 3; ++i)
            {
                arrayUniform->AddMember(std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            }
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "testArray", std::move(arrayUniform));
            
            // Verify that array elements are properly spaced according to std140
            auto offset0 = bindingPoint.GetAccessOffset("testArray[0]");
            auto offset1 = bindingPoint.GetAccessOffset("testArray[1]");
            auto offset2 = bindingPoint.GetAccessOffset("testArray[2]");
            
            CHECK(offset0.has_value());
            CHECK(offset1.has_value());
            CHECK(offset2.has_value());
            
            CHECK_EQ(offset0.value(), 0);
            CHECK_EQ(offset1.value(), 16);  // Should be 16 bytes later, not 4
            CHECK_EQ(offset2.value(), 32);  // Should be 32 bytes later, not 8
            
            // Total size should be 48 bytes (3 * 16)
            CHECK_EQ(bindingPoint.GetTotalSize(), 48);
        }

        SUBCASE("Vector3 array binding point validation")
        {
            // Create an array of vec3 uniforms
            auto arrayUniform = std::make_unique<UniformVariableArray>();
            
            // Add 2 vec3 elements
            for (int i = 0; i < 2; ++i)
            {
                arrayUniform->AddMember(std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));
            }
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "vec3Array", std::move(arrayUniform));
            
            // Verify that vec3 array elements are properly spaced
            auto offset0 = bindingPoint.GetAccessOffset("vec3Array[0]");
            auto offset1 = bindingPoint.GetAccessOffset("vec3Array[1]");
            
            CHECK(offset0.has_value());
            CHECK(offset1.has_value());
            
            CHECK_EQ(offset0.value(), 0);
            CHECK_EQ(offset1.value(), 16);  // Should be 16 bytes later (vec3 + padding)
            
            // Total size should be 32 bytes (2 * 16)
            CHECK_EQ(bindingPoint.GetTotalSize(), 32);
        }

        SUBCASE("Structure binding point validation")
        {
            // Create a structure with mixed types to test padding
            auto structUniform = std::make_unique<UniformVariableStructure>();
            
            // Add members: float, vec3, float
            structUniform->AddMember("floatA", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            structUniform->AddMember("vec3B", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));
            structUniform->AddMember("floatC", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "testStruct", std::move(structUniform));
            
            // Verify offsets according to std140 rules
            auto offsetA = bindingPoint.GetAccessOffset("testStruct.floatA");
            auto offsetB = bindingPoint.GetAccessOffset("testStruct.vec3B");
            auto offsetC = bindingPoint.GetAccessOffset("testStruct.floatC");
            
            CHECK(offsetA.has_value());
            CHECK(offsetB.has_value());
            CHECK(offsetC.has_value());
            
            CHECK_EQ(offsetA.value(), 0);
            CHECK_EQ(offsetB.value(), 16);  // vec3 should be aligned to 16-byte boundary
            CHECK_EQ(offsetC.value(), 28);  // float after vec3 (12 bytes + offset 16)
            
            // Total size should be properly aligned
            CHECK_GE(bindingPoint.GetTotalSize(), 32);  // At least 32 bytes for proper alignment
        }
    }
}