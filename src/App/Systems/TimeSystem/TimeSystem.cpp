#include "Ailurus/Application/TimeSystem/TimeSystem.h"

namespace Ailurus
{
    TimeSystem::TimeSystem()
        : _frameCount(0)
        , _frameTimer()
        , _deltaTime(0.0)
        , _elapsedTimer()
    {
        _elapsedTimer.SetNow();
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

	double TimeSystem::GetElapsedTime() const
	{
		// Convert nanoseconds to seconds
		return _elapsedTimer.GetInterval() / 1000000000.0;
	}

    void TimeSystem::Update()
    {
        _frameCount++;
        _deltaTime = _frameTimer.GetIntervalAndSetNow();
    }
} // namespace Ailurus