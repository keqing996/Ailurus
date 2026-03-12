#pragma once

namespace Ailurus
{
	class NonCopyable
	{
	public:
		NonCopyable() = default;

		NonCopyable(const NonCopyable&) = delete;
		const NonCopyable& operator=(const NonCopyable&) = delete;
	};
} // namespace Ailurus
