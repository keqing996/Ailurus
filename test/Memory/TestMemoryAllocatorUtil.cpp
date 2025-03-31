#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Ailurus/Memory/Util/Util.hpp"

using namespace Ailurus;

TEST_SUITE("MemoryAllocatorUtil")
{
    TEST_CASE("TestAlignment")
    {
        CHECK(MemoryAllocatorUtil::UpAlignment(3, 4) == 4);
        CHECK(MemoryAllocatorUtil::UpAlignment(3, 8) == 8);
        CHECK(MemoryAllocatorUtil::UpAlignment(3, 16) == 16);
        CHECK(MemoryAllocatorUtil::UpAlignment(5, 4) == 8);
        CHECK(MemoryAllocatorUtil::UpAlignment(9, 8) == 16);
        CHECK(MemoryAllocatorUtil::UpAlignment(17, 16) == 32);
        CHECK(MemoryAllocatorUtil::UpAlignment(4, 4) == 4);
        CHECK(MemoryAllocatorUtil::UpAlignment(8, 8) == 8);
        CHECK(MemoryAllocatorUtil::UpAlignment(16, 16) == 16);

        CHECK(MemoryAllocatorUtil::UpAlignment<3, 4>() == 4);
        CHECK(MemoryAllocatorUtil::UpAlignment<3, 8>() == 8);
        CHECK(MemoryAllocatorUtil::UpAlignment<3, 16>() == 16);
        CHECK(MemoryAllocatorUtil::UpAlignment<5, 4>() == 8);
        CHECK(MemoryAllocatorUtil::UpAlignment<9, 8>() == 16);
        CHECK(MemoryAllocatorUtil::UpAlignment<17, 16>() == 32);
        CHECK(MemoryAllocatorUtil::UpAlignment<4, 4>() == 4);
        CHECK(MemoryAllocatorUtil::UpAlignment<8, 8>() == 8);
        CHECK(MemoryAllocatorUtil::UpAlignment<16, 16>() == 16);
    }

    TEST_CASE("TestPowOfTwo")
    {
        CHECK(MemoryAllocatorUtil::UpAlignmentPowerOfTwo(2) == 4);
        CHECK(MemoryAllocatorUtil::UpAlignmentPowerOfTwo(5) == 8);
        CHECK(MemoryAllocatorUtil::UpAlignmentPowerOfTwo(9) == 16);
        CHECK(MemoryAllocatorUtil::UpAlignmentPowerOfTwo(16) == 16);
        CHECK(MemoryAllocatorUtil::UpAlignmentPowerOfTwo(55) == 64);
        CHECK(MemoryAllocatorUtil::UpAlignmentPowerOfTwo(129) == 256);
    }
}
