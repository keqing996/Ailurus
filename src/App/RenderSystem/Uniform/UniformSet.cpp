#include <../../../../include/App/Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{

	UniformSet::UniformSet(uint32_t setId)
		: _setId(setId)
	{
	}

	UniformSet::~UniformSet() = default;

	void UniformSet::AddBindingPoint(std::unique_ptr<UniformBindingPoint>&& pBindingPoint)
	{
		const auto id = pBindingPoint->GetBindingPoint();
		if (_bindingPoints.contains(id))
		{
			Logger::LogError("Duplicate binding point id: {} ", id);
			return;
		}

		_bindingPoints[id] = std::move(pBindingPoint);
	}

	const UniformBindingPoint* UniformSet::GetBindingPoint(uint32_t bindingPoint) const
	{
		auto itr = _bindingPoints.find(bindingPoint);
		if (itr != _bindingPoints.end())
			return itr->second.get();

		return nullptr;
	}

	uint32_t UniformSet::GetSetId() const
	{
		return _setId;
	}
} // namespace Ailurus