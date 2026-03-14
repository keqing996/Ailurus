#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Systems/SceneSystem/Component/CompCamera.h>

using namespace Ailurus;

TEST_SUITE("Camera")
{
	TEST_CASE("Perspective Set preserves horizontal fov and aspect ratio")
	{
		CompCamera camera(0.2f, 0.2f, 0.1f, 50.0f);

		CHECK(camera.GetHorizontalFOV() == doctest::Approx(90.0f));

		camera.Set(90.0f, 16.0f / 9.0f, 0.1f, 50.0f);

		CHECK(camera.GetHorizontalFOV() == doctest::Approx(90.0f));
		CHECK(camera.GetAspectRatio() == doctest::Approx(16.0f / 9.0f));
		CHECK(camera.GetWidth() / camera.GetHeight() == doctest::Approx(16.0f / 9.0f));
		CHECK(camera.GetWidth() == doctest::Approx(0.2f));
		CHECK(camera.GetHeight() == doctest::Approx(0.1125f));
	}
}