#pragma once

#include <Ailurus/Math/Matrix4x4.hpp>
#include "RenderingInfo.h"

namespace Ailurus
{
	struct RenderIntermediateVariable
	{
		// View and projection matrices
		Matrix4x4f projMatrix;
		Matrix4x4f viewMatrix;
		RenderInfo renderInfo;
	};
}