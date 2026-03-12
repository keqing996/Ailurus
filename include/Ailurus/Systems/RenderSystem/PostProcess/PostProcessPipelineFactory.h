#pragma once

#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
    class VulkanPipeline;
    class VulkanDescriptorSetLayout;
    class Shader;
    class ShaderLibrary;

    struct PostProcessPipelineDesc
    {
        vk::Format outputFormat = vk::Format::eUndefined;
        std::string fragShaderPath;
        const VulkanDescriptorSetLayout* pDescriptorSetLayout = nullptr;
        uint32_t pushConstantSize = 0;
        bool blendEnabled = false;
    };

    /// @brief Factory for creating post-process pipelines.
    /// Encapsulates common post-process pipeline configuration:
    /// full-screen triangle (no vertex input), no depth test, single sample.
    class PostProcessPipelineFactory : public NonCopyable, public NonMovable
    {
    public:
        /// @brief Initialize the factory with a shader library for loading shaders.
        void Init(ShaderLibrary* pShaderLibrary);

        /// @brief Create a post-process pipeline. Ownership of the pipeline transfers to the caller.
        std::unique_ptr<VulkanPipeline> CreatePipeline(const PostProcessPipelineDesc& desc);

        /// @brief Get the shared fullscreen vertex shader.
        Shader* GetFullscreenVertexShader() const;

    private:
        static const char* FULLSCREEN_VERT_PATH;
        Shader* _pFullscreenVertShader = nullptr;
        ShaderLibrary* _pShaderLibrary = nullptr;
    };
} // namespace Ailurus
