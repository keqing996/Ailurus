#pragma once

namespace Ailurus
{
	class NonMovable
	{
	public:
		NonMovable() = default;

		NonMovable(NonMovable&&) = delete;
		NonMovable& operator=(NonMovable&&) = delete;
	};
} // namespace Ailurus
