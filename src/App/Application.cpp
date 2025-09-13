#include "Ailurus/Application/Application.h"
#include "VulkanContext/VulkanContext.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <memory>

namespace Ailurus
{
	static std::vector<const char*> VulkanContextGetInstanceExtensions()
	{
		std::vector<const char*> extensions;
		uint32_t size;
		char const* const* pExt = SDL_Vulkan_GetInstanceExtensions(&size);
		for (uint32_t n = 0; n < size; n++)
			extensions.push_back(pExt[n]);
		return extensions;
	}

	static vk::SurfaceKHR VulkanContextCreateSurface(const vk::Instance& instance)
	{
		VkSurfaceKHR surface;
		SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(Application::GetSDLWindowPtr()), instance, nullptr, &surface);
		return surface;
	}

	static void VulkanContextDestroySurface(const vk::Instance& instance, const vk::SurfaceKHR& surface)
	{
		SDL_Vulkan_DestroySurface(instance, surface, nullptr);
	}

	void* 							Application::_pWindow = nullptr;
	bool 							Application::_ignoreNextQuit = false;

	std::function<void()> 			Application::_onWindowCreated = nullptr;
	std::function<void(Vector2i)> 	Application::_onWindowMoved = nullptr;
	std::function<bool()> 			Application::_onWindowTryToClose = nullptr;
	std::function<void()> 			Application::_onWindowClosed = nullptr;
	std::function<void()> 			Application::_onWindowPreDestroyed = nullptr;
	std::function<void()> 			Application::_onWindowPostDestroyed = nullptr;
	std::function<void(Vector2i)> 	Application::_onWindowResize = nullptr;
	std::function<void(bool)> 		Application::_onWindowFocusChanged = nullptr;
	std::function<void(bool)> 		Application::_onWindowCursorEnteredOrLeaved = nullptr;
	std::function<void(bool)> 		Application::_onWindowCursorVisibleChanged = nullptr;

	std::unique_ptr<TimeSystem> 	Application::_pTimeSystem = nullptr;
	std::unique_ptr<InputSystem> 	Application::_pInputManager = nullptr;
	std::unique_ptr<RenderSystem> 	Application::_pRenderSystem = nullptr;
	std::unique_ptr<AssetsSystem> 	Application::_pAssetsSystem = nullptr;
	std::unique_ptr<SceneSystem> 	Application::_pSceneManager = nullptr;
	std::unique_ptr<ImGuiSystem> 	Application::_pImGuiSystem = nullptr;

	bool Application::Create(int width, int height, const std::string& title, Style style)
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
			return false;

		SDL_WindowFlags flags = 0;
		if (!style.haveBorder)
			flags |= SDL_WINDOW_BORDERLESS;
		if (style.canResize)
			flags |= SDL_WINDOW_RESIZABLE;

		flags |= SDL_WINDOW_VULKAN;
		flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
		flags |= SDL_WINDOW_HIDDEN;

		_pWindow = SDL_CreateWindow(title.c_str(), width, height, flags);
		if (_pWindow == nullptr)
			return false;

		SetWindowVisible(true);

		VulkanContext::Initialize(VulkanContextGetInstanceExtensions, VulkanContextCreateSurface, true);
		if (!VulkanContext::Initialized())
		{
			Destroy();
			return false;
		}

		_pTimeSystem.reset(new TimeSystem());
		_pInputManager.reset(new InputSystem());
		_pRenderSystem.reset(new RenderSystem());
		_pAssetsSystem.reset(new AssetsSystem());
		_pSceneManager.reset(new SceneSystem());
		_pImGuiSystem.reset(new ImGuiSystem());

		if (_onWindowCreated != nullptr)
			_onWindowCreated();

		return true;
	}

	void Application::Destroy()
	{
		if (_pWindow)
		{
			if (_onWindowPreDestroyed)
				_onWindowPreDestroyed();

			_pImGuiSystem = nullptr;
			_pSceneManager = nullptr;
			_pAssetsSystem = nullptr;
			_pRenderSystem = nullptr;
			_pInputManager = nullptr;

			VulkanContext::Destroy(VulkanContextDestroySurface);

			SDL_DestroyWindow(static_cast<SDL_Window*>(_pWindow));
			_pWindow = nullptr;

			if (_onWindowPostDestroyed)
				_onWindowPostDestroyed();
		}
	}

	bool Application::IsWindowValid()
	{
		return _pWindow != nullptr;
	}

	void Application::Loop(const std::function<void()>& loopFunction)
	{
		while (true)
		{
			_pTimeSystem->Update();
			_pImGuiSystem->NewFrame();
			
			bool shouldBreakLoop = false;
			EventLoop(&shouldBreakLoop);

			if (shouldBreakLoop)
				break;

			if (loopFunction != nullptr)
				loopFunction();

			_pRenderSystem->RenderScene();
		}

		VulkanContext::WaitDeviceIdle();

		Destroy();
	}

	Vector2i Application::GetSize()
	{
		if (_pWindow != nullptr)
		{
			int w, h;
			SDL_GetWindowSize(static_cast<SDL_Window*>(_pWindow), &w, &h);
			return {w, h};
		}

		return {0, 0};
	}

	void Application::SetSize(int width, int height)
	{
		if (_pWindow != nullptr)
			SDL_SetWindowSize(static_cast<SDL_Window*>(_pWindow), width, height);
	}

	void Application::SetSize(const Vector2i& size)
	{
		SetSize(size.x, size.y);
	}

	Vector2i Application::GetPosition()
	{
		if (_pWindow == nullptr)
			return {0, 0};

		int x, y;
		SDL_GetWindowPosition(static_cast<SDL_Window*>(_pWindow), &x, &y);
		return {x, y};
	}

	void Application::SetPosition(int x, int y)
	{
		if (_pWindow == nullptr)
			return;

		SDL_SetWindowPosition(static_cast<SDL_Window*>(_pWindow), x, y);
	}

	void Application::SetPosition(const Vector2i& pos)
	{
		SetPosition(pos.x, pos.y);
	}

	void Application::SetIcon(unsigned int width, unsigned int height, const std::byte* pixels)
	{
		if (_pWindow == nullptr)
			return;

		auto surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA8888,
			const_cast<std::byte*>(pixels), width * 4);
		SDL_SetWindowIcon(static_cast<SDL_Window*>(_pWindow), surface);
	}

	void Application::SetTitle(const std::string& title)
	{
		if (_pWindow != nullptr)
			SDL_SetWindowTitle(static_cast<SDL_Window*>(_pWindow), title.c_str());
	}

	void Application::SetWindowVisible(bool show)
	{
		if (_pWindow == nullptr)
			return;

		if (show)
			SDL_ShowWindow(static_cast<SDL_Window*>(_pWindow));
		else
			SDL_HideWindow(static_cast<SDL_Window*>(_pWindow));
	}

	bool Application::IsCursorVisible()
	{
		return SDL_CursorVisible();
	}

	void Application::SetCursorVisible(bool show)
	{
		if (show)
			SDL_ShowCursor();
		else
			SDL_HideCursor();
	}

	bool Application::IsCursorLimitedInWindow()
	{
		if (_pWindow == nullptr)
			return false;

		return SDL_GetWindowMouseGrab(static_cast<SDL_Window*>(_pWindow));
	}

	void Application::SetCursorLimitedInWindow(bool capture)
	{
		if (_pWindow == nullptr)
			return;

		SDL_SetWindowMouseGrab(static_cast<SDL_Window*>(_pWindow), capture);
	}

	bool Application::IsCursorInsideWindow()
	{
		if (_pWindow == nullptr)
			return false;

		int windowX, windowY;
		SDL_GetWindowPosition(static_cast<SDL_Window*>(_pWindow), &windowX, &windowY);

		int windowWidth, windowHeight;
		SDL_GetWindowSize(static_cast<SDL_Window*>(_pWindow), &windowWidth, &windowHeight);

		float mouseX, mouseY;
		SDL_GetGlobalMouseState(&mouseX, &mouseY);

		return (mouseX >= windowX && mouseX <= windowX + windowWidth && mouseY >= windowY && mouseY <= windowY + windowHeight);
	}

	void Application::SetCallbackOnWindowCreated(const std::function<void()>& callback)
	{
		_onWindowCreated = callback;
	}

	void Application::SetCallbackOnWindowTryToClose(const std::function<bool()>& callback)
	{
		_onWindowTryToClose = callback;
	}

	void Application::SetCallbackOnWindowClosed(const std::function<void()>& callback)
	{
		_onWindowClosed = callback;
	}

	void Application::SetCallbackOnWindowPreDestroyed(const std::function<void()>& callback)
	{
		_onWindowPreDestroyed = callback;
	}

	void Application::SetCallbackOnWindowPostDestroyed(const std::function<void()>& callback)
	{
		_onWindowPostDestroyed = callback;
	}

	void Application::SetCallbackOnWindowMoved(const std::function<void(Vector2i)>& callback)
	{
		_onWindowMoved = callback;
	}

	void Application::SetCallbackOnWindowResize(const std::function<void(Vector2i)>& callback)
	{
		_onWindowResize = callback;
	}

	void Application::SetCallbackOnWindowFocusChanged(const std::function<void(bool)>& callback)
	{
		_onWindowFocusChanged = callback;
	}

	void Application::SetCallbackOnWindowCursorEnteredOrLeaved(const std::function<void(bool)>& callback)
	{
		_onWindowCursorEnteredOrLeaved = callback;
	}

	void Application::SetCallbackOnWindowCursorVisibleChanged(const std::function<void(bool)>& callback)
	{
		_onWindowCursorVisibleChanged = callback;
	}

	void* Application::GetSDLWindowPtr()
	{
		return _pWindow;
	}

	template <>
    InputSystem* Application::Get<InputSystem>()
	{
		return _pInputManager.get();
	}

	template <>
	RenderSystem* Application::Get<RenderSystem>()
	{
		return _pRenderSystem.get();
	}

	template <>
    SceneSystem* Application::Get<SceneSystem>()
	{
		return _pSceneManager.get();
	}

	template <>
	TimeSystem* Application::Get<TimeSystem>()
	{
		return _pTimeSystem.get();
	}

	template <>
	AssetsSystem* Application::Get<AssetsSystem>()
	{
		return _pAssetsSystem.get();
	}

	template<>
	ImGuiSystem* Application::Get<ImGuiSystem>()
	{
		return _pImGuiSystem.get();
	}

	void Application::EventLoop(bool* quitLoop)
	{
		if (_pInputManager != nullptr)
			_pInputManager->BeforeEventLoop();

		while (true)
		{
			SDL_Event event;
			if (!SDL_PollEvent(&event))
				break;

			bool closeWindow = false;
			HandleEvent(&event, &closeWindow);

			if (_pImGuiSystem != nullptr)
				_pImGuiSystem->HandleEvent(&event);

			if (_pInputManager != nullptr)
				_pInputManager->HandleEvent(_pWindow, &event);

			if (closeWindow)
				*quitLoop = true;
		}

		if (_pInputManager != nullptr)
			_pInputManager->AfterEventLoop();
	}

	void Application::HandleEvent(const void* pEvent, bool* quitLoop)
	{
		const SDL_Event* pSDLEvent = static_cast<const SDL_Event*>(pEvent);
		const SDL_WindowID windowId = SDL_GetWindowID(static_cast<SDL_Window*>(_pWindow));

		switch (pSDLEvent->type)
		{
			case SDL_EVENT_QUIT:
			{
				if (_ignoreNextQuit)
					_ignoreNextQuit = false;
				else
					*quitLoop = true;
				break;
			}
			case SDL_EVENT_WINDOW_DESTROYED:
			{
				if (windowId == pSDLEvent->window.windowID)
					*quitLoop = true;

				break;
			}
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				if (windowId == pSDLEvent->window.windowID
					&& _onWindowTryToClose != nullptr && !_onWindowTryToClose())
				{
					_ignoreNextQuit = true;
				}

				break;
			}
			case SDL_EVENT_WINDOW_MOVED:
			{
				if (windowId == pSDLEvent->window.windowID && _onWindowMoved != nullptr)
					_onWindowMoved(Vector2i(pSDLEvent->window.data1, pSDLEvent->window.data2));

				break;
			}
			case SDL_EVENT_WINDOW_RESIZED:
			{
				if (_pRenderSystem)
					_pRenderSystem->RequestRebuildSwapChain();

				if (windowId == pSDLEvent->window.windowID && _onWindowResize != nullptr)
					_onWindowResize(Vector2i(pSDLEvent->window.data1, pSDLEvent->window.data2));

				break;
			}
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
			{
				if (windowId == pSDLEvent->window.windowID && _onWindowFocusChanged != nullptr)
					_onWindowFocusChanged(true);

				break;
			}
			case SDL_EVENT_WINDOW_FOCUS_LOST:
			{
				if (windowId == pSDLEvent->window.windowID && _onWindowFocusChanged != nullptr)
					_onWindowFocusChanged(false);

				break;
			}
			case SDL_EVENT_WINDOW_MOUSE_ENTER:
			{
				if (windowId == pSDLEvent->window.windowID && _onWindowCursorEnteredOrLeaved != nullptr)
					_onWindowCursorEnteredOrLeaved(true);

				break;
			}
			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			{
				if (windowId == pSDLEvent->window.windowID && _onWindowCursorEnteredOrLeaved != nullptr)
					_onWindowCursorEnteredOrLeaved(false);

				break;
			}
			default:
				break;
		}
	}
} // namespace Ailurus
