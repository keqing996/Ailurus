#include <queue>
#include <functional>
#include "Ailurus/Application/Application.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Pipeline/VulkanPipelineManager.h"

static std::queue<std::function<void()>> gGraphicsSettingChangeCmdQueue;

namespace Ailurus
{
    bool        Application::GraphicsSetting::_enableRender3D = true;
    bool        Application::GraphicsSetting::_enableRenderImGui = true;
    bool        Application::GraphicsSetting::_enableVSync = true;
    bool        Application::GraphicsSetting::_enableMSAA = true;
    uint32_t    Application::GraphicsSetting::_targetFrameRate = 60;
	double	    Application::GraphicsSetting::_targetFrameTime = 1.0 / 60.0;

    bool Application::GraphicsSetting::IsEnable3D()
    {
        return _enableRender3D;
    }

    void Application::GraphicsSetting::SetEnable3D(bool enable)
    {
        _enableRender3D = enable;
    }

    bool Application::GraphicsSetting::IsEnableImGui()
    {
        return _enableRenderImGui;
    }

    void Application::GraphicsSetting::SetEnableImGui(bool enable)
    {
        _enableRenderImGui = enable;
    }
    
	void Application::GraphicsSetting::SetTargetFrameRate(uint32_t fps)
	{
		_targetFrameRate = fps;
		if (fps > 0)
			_targetFrameTime = 1.0 / static_cast<double>(fps);
		else
			_targetFrameTime = 0.0;
	}

	uint32_t Application::GraphicsSetting::GetTargetFrameRate()
	{
		return _targetFrameRate;
	}

    bool Application::GraphicsSetting::IsVSyncEnabled()
    {
        return _enableVSync;
    }

    void Application::GraphicsSetting::SetVSyncEnabled(bool enable)
    {
        if (IsVSyncEnabled() == enable)
			return;

		_enableVSync = enable;

        auto pRenderSystem = Application::Get<RenderSystem>();
        if (!pRenderSystem)
        {
            _enableVSync = enable;
        }
        else 
        {
            gGraphicsSettingChangeCmdQueue.push([enable, pRenderSystem]() -> void {
                pRenderSystem->GraphicsWaitIdle();
                Application::GraphicsSetting::_enableVSync = enable;
                pRenderSystem->RequestRebuildSwapChain();
            });
        }
    }

    bool Application::GraphicsSetting::IsMSAAEnabled()
    {
        return _enableMSAA;
    }

    void Application::GraphicsSetting::SetMSAAEnabled(bool enable)
    {
        if (IsMSAAEnabled() == enable)
            return;

        auto pRenderSystem = Application::Get<RenderSystem>();
        if (!pRenderSystem)
        {
            _enableMSAA = enable;
        }
        else
        {
            gGraphicsSettingChangeCmdQueue.push([enable, pRenderSystem]() -> void {
                pRenderSystem->GraphicsWaitIdle();

                // Clear existing pipelines to force rebuild with new MSAA setting
                auto pipelineManager = VulkanContext::GetPipelineManager();
                if (pipelineManager)
                    pipelineManager->ClearPipelines();

                Application::GraphicsSetting::_enableMSAA = enable;

                pRenderSystem->RequestRebuildSwapChain();
            });
        }
    }

    void Application::GraphicsSetting::ProcessSettingChange()
    {
        while (!gGraphicsSettingChangeCmdQueue.empty())
        {
            auto& cmd = gGraphicsSettingChangeCmdQueue.front();
            cmd();
            gGraphicsSettingChangeCmdQueue.pop();
        }
    }
}