#pragma once

#include <random>
#include <chrono>
#include <limits>
#include <type_traits>

namespace Ailurus
{
	class Random
	{
	public:
		Random()
			: _generator(static_cast<unsigned int>(
				  std::chrono::high_resolution_clock::now().time_since_epoch().count())) {}

		explicit Random(unsigned int seed)
			: _generator(seed) {}

		template <typename T> requires std::is_integral_v<T>
		T Next(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(_generator);
		}

		template <typename T> requires std::is_floating_point_v<T>
		T Next(T min = static_cast<T>(0), T max = static_cast<T>(1))
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(_generator);
		}

		bool NextBool(double trueProbability = 0.5)
		{
			std::bernoulli_distribution distribution(trueProbability);
			return distribution(_generator);
		}

		void SetSeed(unsigned int seed)
		{
			_generator.seed(seed);
		}

		std::mt19937& GetGenerator()
		{
			return _generator;
		}

	private:
		std::mt19937 _generator;
	};
} // namespace Ailurus