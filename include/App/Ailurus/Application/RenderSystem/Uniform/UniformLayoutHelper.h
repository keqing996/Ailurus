#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "UniformValue.h"

namespace Ailurus
{
    /**
     * @brief Helper class for calculating std140 uniform buffer layout
     * 
     * This class provides utilities to calculate proper alignment and offsets
     * for uniform buffer objects according to the std140 layout rules as 
     * specified in the OpenGL specification.
     */
    class UniformLayoutHelper
    {
    public:
        /**
         * @brief Calculate the aligned offset according to std140 rules
         * @param offset Current offset
         * @param alignment Required alignment
         * @return Properly aligned offset
         */
        static uint32_t AlignOffset(uint32_t offset, uint32_t alignment);

        /**
         * @brief Get the base alignment requirement for a uniform value type
         * @param type The uniform value type
         * @return Base alignment in bytes
         */
        static uint32_t GetStd140BaseAlignment(UniformValueType type);

        /**
         * @brief Get the array stride for a uniform value type in std140 layout
         * @param type The uniform value type
         * @return Array stride in bytes (always aligned to 16 bytes)
         */
        static uint32_t GetStd140ArrayStride(UniformValueType type);

        /**
         * @brief Calculate the total size of a structure with mixed types
         * @param memberTypes List of member types in order
         * @param memberOffsets Output parameter for calculated offsets
         * @return Total structure size aligned to largest member alignment
         */
        static uint32_t CalculateStructureLayout(
            const std::vector<UniformValueType>& memberTypes,
            std::vector<uint32_t>& memberOffsets);

        /**
         * @brief Calculate the total size of an array
         * @param elementType Type of array elements
         * @param elementCount Number of elements
         * @param elementOffsets Output parameter for calculated element offsets
         * @return Total array size
         */
        static uint32_t CalculateArrayLayout(
            UniformValueType elementType,
            uint32_t elementCount,
            std::vector<uint32_t>& elementOffsets);

        /**
         * @brief Validate if a buffer layout matches std140 requirements
         * @param memberTypes List of member types
         * @param actualOffsets Actual offsets used in the buffer
         * @return True if layout is valid, false otherwise
         */
        static bool ValidateStd140Layout(
            const std::vector<UniformValueType>& memberTypes,
            const std::vector<uint32_t>& actualOffsets);

        /**
         * @brief Get detailed layout information for debugging
         * @param memberTypes List of member types
         * @param memberNames Optional names for members
         * @return String describing the layout
         */
        static std::string GetLayoutDescription(
            const std::vector<UniformValueType>& memberTypes,
            const std::vector<std::string>& memberNames = {});

    private:
        /**
         * @brief Check if a type requires padding in std140 layout
         * @param type The uniform value type
         * @return True if padding is required
         */
        static bool RequiresPadding(UniformValueType type);

        /**
         * @brief Get the actual size of a uniform value type
         * @param type The uniform value type
         * @return Size in bytes (actual memory usage)
         */
        static uint32_t GetActualSize(UniformValueType type);

        /**
         * @brief Get the string name of a uniform value type
         * @param type The uniform value type
         * @return String representation of the type
         */
        static std::string GetTypeName(UniformValueType type);
    };

} // namespace Ailurus