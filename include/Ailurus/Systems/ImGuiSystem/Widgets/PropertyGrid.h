#pragma once

#include <cstddef>

namespace Ailurus::Widgets
{
	bool BeginPropertyGrid(const char* label);
	void EndPropertyGrid();
	void PropertyLabel(const char* label);
	bool PropertyFloat(const char* label, float* value, float min = 0.0f, float max = 0.0f);
	bool PropertyVec3(const char* label, float value[3]);
	bool PropertyColor(const char* label, float color[4]);
	bool PropertyBool(const char* label, bool* value);
	bool PropertyEnum(const char* label, int* current, const char* const* items, int count);
	bool PropertyText(const char* label, char* buf, size_t bufSize);
}