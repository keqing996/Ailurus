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
            
            // Total size should be properly aligned to the largest member alignment (16)
            CHECK_EQ(bindingPoint.GetTotalSize(), 32);  // 32 bytes (28 + 4 aligned to 16)
        }
    }

    TEST_CASE("Std140 Edge Cases and Boundary Conditions")
    {
        SUBCASE("Consecutive scalars packing")
        {
            // struct { float a; float b; float c; float d; }
            // Should pack tightly without unnecessary padding
            std::vector<UniformValueType> types = {
                UniformValueType::Float,
                UniformValueType::Float,
                UniformValueType::Float,
                UniformValueType::Float
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);
            CHECK_EQ(offsets[1], 4);
            CHECK_EQ(offsets[2], 8);
            CHECK_EQ(offsets[3], 12);
            CHECK_EQ(totalSize, 16);  // Aligned to 4 bytes
        }

        SUBCASE("Vec2 followed by vec2")
        {
            // struct { vec2 a; vec2 b; }
            // Both should align to 8 bytes
            std::vector<UniformValueType> types = {
                UniformValueType::Vector2,
                UniformValueType::Vector2
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // vec2 a at 0
            CHECK_EQ(offsets[1], 8);   // vec2 b at 8
            CHECK_EQ(totalSize, 16);   // Total aligned to 8
        }

        SUBCASE("Vec3 followed by float - critical padding test")
        {
            // struct { vec3 a; float b; }
            // vec3 at 0 (size 12), float at 12 (no extra alignment needed)
            std::vector<UniformValueType> types = {
                UniformValueType::Vector3,
                UniformValueType::Float
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // vec3 at 0
            CHECK_EQ(offsets[1], 12);  // float at 12 (can fit in vec3's padding)
            CHECK_EQ(totalSize, 16);   // Total aligned to 16
        }

        SUBCASE("Float followed by vec3 - requires alignment")
        {
            // struct { float a; vec3 b; }
            // float at 0 (size 4), vec3 must align to 16
            std::vector<UniformValueType> types = {
                UniformValueType::Float,
                UniformValueType::Vector3
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // float at 0
            CHECK_EQ(offsets[1], 16);  // vec3 must align to 16
            CHECK_EQ(totalSize, 32);   // Total: 16 + 12 aligned to 16 = 32
        }

        SUBCASE("Multiple vec3s")
        {
            // struct { vec3 a; vec3 b; vec3 c; }
            std::vector<UniformValueType> types = {
                UniformValueType::Vector3,
                UniformValueType::Vector3,
                UniformValueType::Vector3
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // vec3 a at 0
            CHECK_EQ(offsets[1], 16);  // vec3 b at 16 (aligned)
            CHECK_EQ(offsets[2], 32);  // vec3 c at 32 (aligned)
            CHECK_EQ(totalSize, 48);   // Total: 32 + 12 aligned to 16 = 48
        }

        SUBCASE("Vec4 alignment is natural")
        {
            // struct { float a; vec4 b; }
            std::vector<UniformValueType> types = {
                UniformValueType::Float,
                UniformValueType::Vector4
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // float at 0
            CHECK_EQ(offsets[1], 16);  // vec4 must align to 16
            CHECK_EQ(totalSize, 32);   // Total: 16 + 16 = 32
        }

        SUBCASE("Matrix followed by scalar")
        {
            // struct { mat4 m; float f; }
            std::vector<UniformValueType> types = {
                UniformValueType::Mat4,
                UniformValueType::Float
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // mat4 at 0
            CHECK_EQ(offsets[1], 64);  // float at 64 (after matrix)
            CHECK_EQ(totalSize, 80);   // Total: 64 + 4 aligned to 16 = 80
        }

        SUBCASE("Complex interleaved structure")
        {
            // struct { float a; vec2 b; float c; vec3 d; vec4 e; }
            std::vector<UniformValueType> types = {
                UniformValueType::Float,   // 0
                UniformValueType::Vector2, // aligned to 8
                UniformValueType::Float,   // after vec2
                UniformValueType::Vector3, // aligned to 16
                UniformValueType::Vector4  // aligned to 16
            };
            
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateStructureLayout(types, offsets);
            
            CHECK_EQ(offsets[0], 0);   // float a at 0
            CHECK_EQ(offsets[1], 8);   // vec2 b at 8 (aligned to 8)
            CHECK_EQ(offsets[2], 16);  // float c at 16 (8 + 8)
            CHECK_EQ(offsets[3], 32);  // vec3 d at 32 (aligned to 16 from 20)
            CHECK_EQ(offsets[4], 48);  // vec4 e at 48 (aligned to 16 from 44)
            CHECK_EQ(totalSize, 64);   // Total: 48 + 16 = 64
        }
    }

    TEST_CASE("Array Edge Cases")
    {
        SUBCASE("Single element arrays")
        {
            // Arrays still follow stride rules even with 1 element
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Float, 1, offsets);
            
            CHECK_EQ(offsets.size(), 1);
            CHECK_EQ(offsets[0], 0);
            CHECK_EQ(totalSize, 16);  // Single float still takes 16 bytes in array
        }

        SUBCASE("Large float arrays")
        {
            // Test array with many elements
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Float, 10, offsets);
            
            CHECK_EQ(offsets.size(), 10);
            for (uint32_t i = 0; i < 10; ++i)
            {
                CHECK_EQ(offsets[i], i * 16);
            }
            CHECK_EQ(totalSize, 160);  // 10 * 16
        }

        SUBCASE("Vec2 array stride")
        {
            // vec2 arrays use 16-byte stride
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Vector2, 4, offsets);
            
            CHECK_EQ(offsets.size(), 4);
            CHECK_EQ(offsets[0], 0);
            CHECK_EQ(offsets[1], 16);
            CHECK_EQ(offsets[2], 32);
            CHECK_EQ(offsets[3], 48);
            CHECK_EQ(totalSize, 64);
        }

        SUBCASE("Matrix array stride")
        {
            // mat4 arrays use 64-byte stride
            std::vector<uint32_t> offsets;
            uint32_t totalSize = UniformLayoutHelper::CalculateArrayLayout(
                UniformValueType::Mat4, 3, offsets);
            
            CHECK_EQ(offsets.size(), 3);
            CHECK_EQ(offsets[0], 0);
            CHECK_EQ(offsets[1], 64);
            CHECK_EQ(offsets[2], 128);
            CHECK_EQ(totalSize, 192);
        }
    }

    TEST_CASE("Integration Tests - Complex Scenarios")
    {
        SUBCASE("Nested structure simulation")
        {
            // Outer struct { float a; Inner inner; float b; }
            // Inner struct { vec3 pos; float intensity; }
            
            // First calculate inner structure
            std::vector<UniformValueType> innerTypes = {
                UniformValueType::Vector3,
                UniformValueType::Float
            };
            std::vector<uint32_t> innerOffsets;
            uint32_t innerSize = UniformLayoutHelper::CalculateStructureLayout(innerTypes, innerOffsets);
            
            // Inner structure: vec3 at 0, float at 12, total 16
            CHECK_EQ(innerOffsets[0], 0);
            CHECK_EQ(innerOffsets[1], 12);
            CHECK_EQ(innerSize, 16);
        }

        SUBCASE("Array of structures with UniformBindingPoint")
        {
            // Test array of vec4 to ensure proper spacing
            auto arrayUniform = std::make_unique<UniformVariableArray>();
            
            for (int i = 0; i < 4; ++i)
            {
                arrayUniform->AddMember(std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4));
            }
            
            std::vector<ShaderStage> stages = { ShaderStage::Fragment };
            UniformBindingPoint bindingPoint(1, stages, "vec4Array", std::move(arrayUniform));
            
            // Check each element
            for (int i = 0; i < 4; ++i)
            {
                auto offset = bindingPoint.GetAccessOffset("vec4Array[" + std::to_string(i) + "]");
                CHECK(offset.has_value());
                CHECK_EQ(offset.value(), i * 16);
            }
            
            CHECK_EQ(bindingPoint.GetTotalSize(), 64);
        }

        SUBCASE("Mixed types in UniformBindingPoint")
        {
            // struct { vec2 a; int b; int c; vec4 d; }
            auto structUniform = std::make_unique<UniformVariableStructure>();
            
            structUniform->AddMember("a", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector2));
            structUniform->AddMember("b", std::make_unique<UniformVariableNumeric>(UniformValueType::Int));
            structUniform->AddMember("c", std::make_unique<UniformVariableNumeric>(UniformValueType::Int));
            structUniform->AddMember("d", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4));
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "mixedStruct", std::move(structUniform));
            
            auto offsetA = bindingPoint.GetAccessOffset("mixedStruct.a");
            auto offsetB = bindingPoint.GetAccessOffset("mixedStruct.b");
            auto offsetC = bindingPoint.GetAccessOffset("mixedStruct.c");
            auto offsetD = bindingPoint.GetAccessOffset("mixedStruct.d");
            
            CHECK(offsetA.has_value());
            CHECK(offsetB.has_value());
            CHECK(offsetC.has_value());
            CHECK(offsetD.has_value());
            
            CHECK_EQ(offsetA.value(), 0);   // vec2 at 0
            CHECK_EQ(offsetB.value(), 8);   // int at 8
            CHECK_EQ(offsetC.value(), 12);  // int at 12
            CHECK_EQ(offsetD.value(), 16);  // vec4 at 16 (aligned)
            CHECK_EQ(bindingPoint.GetTotalSize(), 32);
        }

        SUBCASE("All scalar types together")
        {
            // struct { int i; float f; int j; float g; }
            auto structUniform = std::make_unique<UniformVariableStructure>();
            
            structUniform->AddMember("i", std::make_unique<UniformVariableNumeric>(UniformValueType::Int));
            structUniform->AddMember("f", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            structUniform->AddMember("j", std::make_unique<UniformVariableNumeric>(UniformValueType::Int));
            structUniform->AddMember("g", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "scalarStruct", std::move(structUniform));
            
            // All should pack tightly
            CHECK_EQ(bindingPoint.GetAccessOffset("scalarStruct.i").value(), 0);
            CHECK_EQ(bindingPoint.GetAccessOffset("scalarStruct.f").value(), 4);
            CHECK_EQ(bindingPoint.GetAccessOffset("scalarStruct.j").value(), 8);
            CHECK_EQ(bindingPoint.GetAccessOffset("scalarStruct.g").value(), 12);
            CHECK_EQ(bindingPoint.GetTotalSize(), 16);
        }

        SUBCASE("Worst case padding scenario")
        {
            // struct { float a; vec3 b; float c; vec3 d; float e; }
            // Maximum padding waste
            auto structUniform = std::make_unique<UniformVariableStructure>();
            
            structUniform->AddMember("a", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            structUniform->AddMember("b", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));
            structUniform->AddMember("c", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            structUniform->AddMember("d", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));
            structUniform->AddMember("e", std::make_unique<UniformVariableNumeric>(UniformValueType::Float));
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "paddedStruct", std::move(structUniform));
            
            CHECK_EQ(bindingPoint.GetAccessOffset("paddedStruct.a").value(), 0);
            CHECK_EQ(bindingPoint.GetAccessOffset("paddedStruct.b").value(), 16);  // vec3 aligns to 16
            CHECK_EQ(bindingPoint.GetAccessOffset("paddedStruct.c").value(), 28);  // float after vec3
            CHECK_EQ(bindingPoint.GetAccessOffset("paddedStruct.d").value(), 32);  // vec3 aligns to 16 from 32
            CHECK_EQ(bindingPoint.GetAccessOffset("paddedStruct.e").value(), 44);  // float after vec3
            CHECK_EQ(bindingPoint.GetTotalSize(), 48);  // 44 + 4 aligned to 16
        }

        SUBCASE("Array of vec3 with multiple elements")
        {
            // This is a common issue - vec3 arrays have 16-byte stride
            auto arrayUniform = std::make_unique<UniformVariableArray>();
            
            for (int i = 0; i < 5; ++i)
            {
                arrayUniform->AddMember(std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));
            }
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "vec3Array", std::move(arrayUniform));
            
            // Each vec3 should be at 16-byte intervals
            for (int i = 0; i < 5; ++i)
            {
                auto offset = bindingPoint.GetAccessOffset("vec3Array[" + std::to_string(i) + "]");
                CHECK(offset.has_value());
                CHECK_EQ(offset.value(), i * 16);
            }
            
            CHECK_EQ(bindingPoint.GetTotalSize(), 80);  // 5 * 16
        }

        SUBCASE("Matrix and vectors combination")
        {
            // struct { mat4 transform; vec3 position; vec4 color; }
            auto structUniform = std::make_unique<UniformVariableStructure>();
            
            structUniform->AddMember("transform", std::make_unique<UniformVariableNumeric>(UniformValueType::Mat4));
            structUniform->AddMember("position", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector3));
            structUniform->AddMember("color", std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4));
            
            std::vector<ShaderStage> stages = { ShaderStage::Vertex };
            UniformBindingPoint bindingPoint(0, stages, "transformStruct", std::move(structUniform));
            
            CHECK_EQ(bindingPoint.GetAccessOffset("transformStruct.transform").value(), 0);
            CHECK_EQ(bindingPoint.GetAccessOffset("transformStruct.position").value(), 64);
            CHECK_EQ(bindingPoint.GetAccessOffset("transformStruct.color").value(), 80);
            CHECK_EQ(bindingPoint.GetTotalSize(), 96);  // 64 + 12 + 16 = 92, aligned to 16 = 96
        }
    }

    TEST_CASE("Alignment Rule Verification")
    {
        SUBCASE("Scalar types have 4-byte alignment")
        {
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Int), 4);
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Float), 4);
        }

        SUBCASE("Vector2 has 8-byte alignment")
        {
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector2), 8);
        }

        SUBCASE("Vector3 and Vector4 have 16-byte alignment")
        {
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector3), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Vector4), 16);
        }

        SUBCASE("Matrix4x4 has 16-byte alignment")
        {
            CHECK_EQ(UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType::Mat4), 16);
        }

        SUBCASE("All array strides are at least 16 bytes")
        {
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Int), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Float), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector2), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector3), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Vector4), 16);
            CHECK_EQ(UniformLayoutHelper::GetStd140ArrayStride(UniformValueType::Mat4), 64);
        }
    }
}
