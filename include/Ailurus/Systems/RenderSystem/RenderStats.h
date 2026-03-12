#pragma once

#include <cstdint>

namespace Ailurus
{
	struct RenderStats
	{
		uint32_t drawCalls = 0;
		uint32_t triangleCount = 0;
		uint32_t entityCount = 0;
		uint32_t culledEntityCount = 0;
		uint32_t meshCount = 0;
		float frameTimeMs = 0.0f;

		void Reset()
		{
			drawCalls = 0;
			triangleCount = 0;
			entityCount = 0;
			culledEntityCount = 0;
			meshCount = 0;
		}
	};
} // namespace Ailurus
