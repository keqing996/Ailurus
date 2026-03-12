#pragma once

#include <cstdint>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Utility/Timer.h"

namespace Ailurus
{
	class TimeSystem : public NonCopyable, public NonMovable
	{
		friend class Application;
		TimeSystem();

	public:
		~TimeSystem();

	public:
		uint64_t FrameCount() const;
		double DeltaTime() const;
		double GetElapsedTime() const;

	private:
		void Update();

	private:
		uint64_t _frameCount;
		Timer<> _frameTimer;
		double _deltaTime;
		Timer<TimePrecision::Nanoseconds> _elapsedTimer;
	};
} // namespace Ailurus