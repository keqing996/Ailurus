#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
	class Mesh;

	class AssetsManager: public NonCopyable
	{
	public:
		

	private:
		std::unordered_map<std::string, std::shared_ptr<Mesh>> _meshMap;
	};
} // namespace Ailurus