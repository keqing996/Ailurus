#include "InputAssemble.h"
#include "Vulkan/DataBuffer/IndexBuffer.h"
#include "Vulkan/DataBuffer/VertexBuffer.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    static vk::Format ConvertToVkFormat(AttributeType type)
    {
        switch (type)
        {
            case AttributeType::Vector2:
                return vk::Format::eR32G32Sfloat;
            case AttributeType::Vector3:
                return vk::Format::eR32G32B32Sfloat;
            case AttributeType::Vector4:
                return vk::Format::eR32G32B32A32Sfloat;
        }

        Logger::LogError("Fail to convert attribute type to vk format, attribute type = {}",
            EnumReflection<AttributeType>::ToString(type));
        return vk::Format::eUndefined;
    }

    InputAssemble::InputAssemble(std::unique_ptr<VertexBuffer>&& pVertexBuffer,
        const InputAttribute& inputAttr)
        : _pVertexBuffer(std::move(pVertexBuffer))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(nullptr)
    {
    }

    InputAssemble::InputAssemble(std::unique_ptr<VertexBuffer>&& pVertexBuffer,
        const InputAttribute& inputAttr, std::unique_ptr<IndexBuffer>&& pIndexBuffer)
        : _pVertexBuffer(std::move(pVertexBuffer))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(std::move(pIndexBuffer))
    {
    }

    InputAssemble::~InputAssemble()
    {
    }

    const VertexBuffer* InputAssemble::GetVertexBuffer() const
    {
        return _pVertexBuffer.get();
    }

    const InputAttribute& InputAssemble::GetInputAttribute() const
    {
        return _inputAttr;
    }

    const IndexBuffer* InputAssemble::GetIndexBuffer() const
    {
        return _pIndexBuffer.get();
    }

    size_t InputAssemble::GetVertexCount() const
    {
        auto vertexSizeBytes = _pVertexBuffer->GetSize();
        auto stride = _inputAttr.GetStride();
        return vertexSizeBytes /stride;
    }

    std::vector<vk::VertexInputAttributeDescription> InputAssemble::GetAttributeDescription() const
    {
        std::vector<vk::VertexInputAttributeDescription> result;

        uint32_t offset = 0;
        auto attributes = _inputAttr.GetAttributes();
        for (auto i = 0; i < attributes.size(); i++)
        {
            AttributeType attr = attributes[i];

            vk::VertexInputAttributeDescription attributeDescriptions;
            attributeDescriptions.setBinding(0)
                .setLocation(i)
                .setFormat(ConvertToVkFormat(attr))
                .setOffset(offset);

            result.push_back(attributeDescriptions);

            offset += InputAttribute::SizeOf(attr);
        }

        return result;
    }
}
