#include <Ailurus/Application/RenderSystem/Uniform/UniformAccess.h>

namespace Ailurus
{
    size_t UniformAccessHash::operator()(const UniformAccess& entry) const
    {
        return std::hash<uint32_t>()(entry.bindingId) ^ 
               std::hash<std::string>()(entry.access);
    }
    
    bool UniformAccessEqual::operator()(const UniformAccess& lhs, const UniformAccess& rhs) const
    {
        return lhs.bindingId == rhs.bindingId && 
               lhs.access == rhs.access;
    }
}