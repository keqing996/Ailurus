#pragma once

#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
    class Render: public NonCopyable
    {
    public:
        Render() = default;
        virtual ~Render() = default;
    };
}