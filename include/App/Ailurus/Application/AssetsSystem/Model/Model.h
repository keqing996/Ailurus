#pragma once

#include <vector>
#include <memory>
#include "Ailurus/Application/AssetsSystem/Asset.h"
#include "Ailurus/Application/AssetsSystem/AssetReference.h"
#include "Ailurus/Application/AssetsSystem/Mesh/Mesh.h"

namespace Ailurus
{
	class Model : public Asset
	{
	public:

	public:
		bool LoadFromFile(const std::string& path);

	private:
		std::vector<std::unique_ptr<Mesh>> _meshes;
	};
}