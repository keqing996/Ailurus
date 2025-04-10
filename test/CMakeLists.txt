function(create_ailurus_core_test test_name source_file)
    add_executable          (${test_name} ${source_file})
    target_link_libraries   (${test_name} PUBLIC ailurus-core doctest)
    add_test                (NAME ${test_name} COMMAND ${test_name})
endfunction()

create_ailurus_core_test (ailurus_test_container_segment_array  Container/TestSegmentArray.cpp)
create_ailurus_core_test (ailurus_test_lock_free_queue          Container/TestLockFreeQueue.cpp)
create_ailurus_core_test (ailurus_test_math_vector              TestVector.cpp)
create_ailurus_core_test (ailurus_test_math_matrix              TestMatrix.cpp)
create_ailurus_core_test (ailurus_test_math_quaternion          TestQuaternion.cpp)
create_ailurus_core_test (ailurus_test_string                   TestString.cpp)
create_ailurus_core_test (ailurus_test_thread_pool              TestThreadPool.cpp)
create_ailurus_core_test (ailurus_test_enum_reflection          TestEnumReflection.cpp)

# Memory allocator
create_ailurus_core_test (ailurus_test_memory_util                  Memory/TestMemoryAllocatorUtil.cpp)
create_ailurus_core_test (ailurus_test_memory_linear_allocator      Memory/TestLinearAllocator.cpp)
create_ailurus_core_test (ailurus_test_memory_free_list_allocator   Memory/TestFreeListAllocator.cpp)
create_ailurus_core_test (ailurus_test_memory_pool_allocator        Memory/TestPoolAllocator.cpp)
create_ailurus_core_test (ailurus_test_memory_stack_allocator       Memory/TestStackAllocator.cpp)

