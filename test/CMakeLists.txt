
add_executable          (ailurus_test_container_segment_array Container/TestSegmentArray.cpp)
target_link_libraries   (ailurus_test_container_segment_array PUBLIC ${AILURUS_TARGET_NAME})
add_test                (NAME ailurus_test_container_segment_array COMMAND ailurus_test_container_segment_array)

add_executable          (ailurus_test_math TestVector.cpp)
target_link_libraries   (ailurus_test_math PUBLIC ${AILURUS_TARGET_NAME})
add_test                (NAME ailurus_test_math COMMAND ailurus_test_math)

add_executable          (ailurus_test_string TestString.cpp)
target_link_libraries   (ailurus_test_string PUBLIC ${AILURUS_TARGET_NAME})
add_test                (NAME ailurus_test_string COMMAND ailurus_test_string)

add_executable          (ailurus_test_lock_free_queue Container/TestLockFreeQueue.cpp)
target_link_libraries   (ailurus_test_lock_free_queue PUBLIC ${AILURUS_TARGET_NAME})
add_test                (NAME ailurus_test_lock_free_queue COMMAND ailurus_test_lock_free_queue)

add_executable          (ailurus_test_thread_pool TestThreadPool.cpp)
target_link_libraries   (ailurus_test_thread_pool PUBLIC ${AILURUS_TARGET_NAME})
add_test                (NAME ailurus_test_thread_pool COMMAND ailurus_test_thread_pool)

add_executable          (ailurus_test_enum_reflection TestEnumReflection.cpp)
target_link_libraries   (ailurus_test_enum_reflection PUBLIC ${AILURUS_TARGET_NAME})
add_test                (NAME ailurus_test_enum_reflection COMMAND ailurus_test_enum_reflection)