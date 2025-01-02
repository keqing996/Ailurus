#pragma once

#include <functional>
#include "NonCopyable.h"

namespace Utility
{
	class ScopeGuard : NonCopyable
    {
    public:
        ScopeGuard(ScopeGuard&& other) = delete;

        ~ScopeGuard()
        {
            _cleanup();
        }

        template<class Func>
        ScopeGuard(const Func& cleanup) // NOLINT(*-explicit-constructor)
            : _cleanup(cleanup)
        {
        }

    private:
        std::function<void()> _cleanup;
    };
}