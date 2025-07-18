#include "Ailurus/Application/TimeSystem/TimeSystem.h"

namespace Ailurus
{
    TimeSystem::TimeSystem()
        : _frameCount(0)
        , _frameTimer()
        , _deltaTime(0.0)
    {
    }

    TimeSystem::~TimeSystem()
    {
    }

	uint64_t TimeSystem::FrameCount() const
	{
		return _frameCount;
	}

	double TimeSystem::DeltaTime() const
	{
		return _deltaTime;
	}

    void TimeSystem::Update()
    {
        _frameCount++;
        _deltaTime = _frameTimer.GetIntervalAndSetNow();
    }
} // namespace Ailurus