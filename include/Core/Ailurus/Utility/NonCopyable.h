#pragma once

namespace Ailurus
{
    class NonCopyable
    {
    public:
        NonCopyable() = default;
        virtual ~NonCopyable() = default;

    public:
        NonCopyable( const NonCopyable& ) = delete;
        const NonCopyable& operator=( const NonCopyable& ) = delete;
    };
}
