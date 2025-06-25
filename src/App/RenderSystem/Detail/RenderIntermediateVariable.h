#pragma once

#include <Ailurus/Math/Matrix4x4.hpp>
#include "RenderingInfo.h"

namespace Ailurus
{
	struct RenderIntermediateVariable
	{
		// View and projection matrices
		Matrix4x4f viewProjectionMatrix;
		RenderInfo renderInfo;
	};
}