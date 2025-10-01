#include "Ailurus/Application/RenderSystem/Uniform/UniformLayoutHelper.h"
#include "Ailurus/Utility/Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Ailurus
{
    uint32_t UniformLayoutHelper::AlignOffset(uint32_t offset, uint32_t alignment)
    {
        return (offset + alignment - 1) & ~(alignment - 1);
    }

    uint32_t UniformLayoutHelper::GetStd140BaseAlignment(UniformValueType type)
    {
        switch (type)
        {
            case UniformValueType::Int:
            case UniformValueType::Float:
                return 4;
            case UniformValueType::Vector2:
                return 8;
            case UniformValueType::Vector3:
            case UniformValueType::Vector4:
                return 16;
            case UniformValueType::Mat4:
                return 16;
            default:
                Logger::LogError("UniformLayoutHelper: Unsupported UniformValueType for alignment calculation.");
                return 0;
        }
    }

    uint32_t UniformLayoutHelper::GetStd140ArrayStride(UniformValueType type)
    {
        // In std140, array elements are always aligned to 16-byte boundaries
        // The array stride is the size of each element rounded up to the next 16-byte boundary
        uint32_t elementSize = GetActualSize(type);
        uint32_t baseAlignment = GetStd140BaseAlignment(type);
        
        // For arrays, elements must be aligned to at least 16 bytes
        uint32_t arrayAlignment = std::max(baseAlignment, 16u);
        return AlignOffset(elementSize, arrayAlignment);
    }

    uint32_t UniformLayoutHelper::CalculateStructureLayout(
        const std::vector<UniformValueType>& memberTypes,
        std::vector<uint32_t>& memberOffsets)
    {
        memberOffsets.clear();
        memberOffsets.reserve(memberTypes.size());

        uint32_t currentOffset = 0;
        uint32_t maxAlignment = 0;

        for (const auto& type : memberTypes)
        {
            uint32_t alignment = GetStd140BaseAlignment(type);
            maxAlignment = std::max(maxAlignment, alignment);

            // Align current offset to member's alignment requirement
            currentOffset = AlignOffset(currentOffset, alignment);
            memberOffsets.push_back(currentOffset);

            // Move to next position
            currentOffset += GetActualSize(type);
        }

        // Align total size to the largest member alignment
        uint32_t totalSize = AlignOffset(currentOffset, maxAlignment);
        return totalSize;
    }

    uint32_t UniformLayoutHelper::CalculateArrayLayout(
        UniformValueType elementType,
        uint32_t elementCount,
        std::vector<uint32_t>& elementOffsets)
    {
        elementOffsets.clear();
        elementOffsets.reserve(elementCount);

        uint32_t arrayStride = GetStd140ArrayStride(elementType);
        
        for (uint32_t i = 0; i < elementCount; ++i)
        {
            elementOffsets.push_back(i * arrayStride);
        }

        return elementCount * arrayStride;
    }

    std::string UniformLayoutHelper::GetLayoutDescription(
        const std::vector<UniformValueType>& memberTypes,
        const std::vector<std::string>& memberNames)
    {
        std::vector<uint32_t> offsets;
        uint32_t totalSize = CalculateStructureLayout(memberTypes, offsets);

        std::ostringstream oss;
        oss << "Std140 Layout Description:\n";
        oss << "Total Size: " << totalSize << " bytes\n";
        oss << "Members:\n";

        for (size_t i = 0; i < memberTypes.size(); ++i)
        {
            std::string name = (i < memberNames.size()) ? memberNames[i] : ("member" + std::to_string(i));
            std::string typeName = GetTypeName(memberTypes[i]);
            uint32_t size = GetActualSize(memberTypes[i]);
            uint32_t alignment = GetStd140BaseAlignment(memberTypes[i]);

            oss << "  [" << std::setw(2) << i << "] " << std::setw(12) << name 
                << " (" << std::setw(8) << typeName << "): "
                << "offset=" << std::setw(3) << offsets[i] 
                << ", size=" << std::setw(2) << size 
                << ", align=" << std::setw(2) << alignment << "\n";
        }

        return oss.str();
    }

    bool UniformLayoutHelper::RequiresPadding(UniformValueType type)
    {
        switch (type)
        {
            case UniformValueType::Vector3:
                return true; // vec3 needs 4 bytes padding to align to 16 bytes
            default:
                return false;
        }
    }

    uint32_t UniformLayoutHelper::GetActualSize(UniformValueType type)
    {
        return UniformValue::GetSize(type);
    }

    std::string UniformLayoutHelper::GetTypeName(UniformValueType type)
    {
        switch (type)
        {
            case UniformValueType::Int:     return "int";
            case UniformValueType::Float:   return "float";
            case UniformValueType::Vector2: return "vec2";
            case UniformValueType::Vector3: return "vec3";
            case UniformValueType::Vector4: return "vec4";
            case UniformValueType::Mat4:    return "mat4";
            default:                        return "unknown";
        }
    }

} // namespace Ailurus