#pragma once

namespace Ailurus::Widgets
{
	enum class ToastType
	{
		Info,
		Success,
		Warning,
		Error,
	};

	void PushToast(const char* message, ToastType type = ToastType::Info,
		float durationSeconds = 3.0f);
	void RenderToasts();
}