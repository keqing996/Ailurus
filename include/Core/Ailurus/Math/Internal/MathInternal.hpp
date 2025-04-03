#pragma once

namespace Ailurus
{
	namespace _internal
	{
		template <size_t Dimension, size_t Value>
		concept DimensionAtLeast = (Dimension >= Value);

		template <typename T, size_t D>
		concept CanDoCross = std::is_floating_point_v<T> && (D == 3);

		template <typename T>
		concept CanNormalize = std::is_floating_point_v<T>;
	} // namespace _internal
} // namespace Ailurus