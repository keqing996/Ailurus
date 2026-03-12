#include "Ailurus/Systems/SceneSystem/Component/Base/Component.h"
#include <nlohmann/json.hpp>

namespace Ailurus
{
	nlohmann::json Component::Serialize() const
	{
		return nlohmann::json::object();
	}
} // namespace Ailurus
