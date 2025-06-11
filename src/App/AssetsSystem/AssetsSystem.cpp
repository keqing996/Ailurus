
#include "Ailurus/Application/AssetsSystem/AssetsSystem.h"

namespace Ailurus
{

	AssetsSystem::~AssetsSystem()
	{
	}

	AssetsSystem::AssetsSystem()
	{
	}

	uint64_t AssetsSystem::NextAssetId()
	{
		return _globalAssetIdCounter.fetch_add(1, std::memory_order_seq_cst);
	}
} // namespace Ailurus