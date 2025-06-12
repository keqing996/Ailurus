#include <Ailurus/Application/AssetsSystem/Material/MaterialUniformAccess.h>

namespace Ailurus
{
    size_t MaterialUniformAccessHash::operator()(const MaterialUniformAccess& entry) const
    {
        return std::hash<uint32_t>()(static_cast<uint32_t>(entry.pass)) ^ 
               std::hash<uint32_t>()(entry.bindingId) ^ 
               std::hash<std::string>()(entry.access);
    }
    
    bool MaterialUniformAccessEqual::operator()(const MaterialUniformAccess& lhs, const MaterialUniformAccess& rhs) const
    {
        return lhs.pass == rhs.pass && 
               lhs.bindingId == rhs.bindingId && 
               lhs.access == rhs.access;
    }
}