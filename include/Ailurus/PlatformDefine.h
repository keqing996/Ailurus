#pragma once

/* Platform define */
#if defined(_MSC_VER) || defined(__MINGW64__) || defined(__MINGW32__)
#   define AILURUS_PLATFORM_WINDOWS 1
#else
#   define AILURUS_PLATFORM_WINDOWS 0
#endif

#if defined(__clang__)
#    define AILURUS_COMPILER_CLANG 1
#    define AILURUS_COMPILER_MSVC 0
#    define AILURUS_COMPILER_GCC 0
#else
#    if defined(_MSC_VER)
#        define AILURUS_COMPILER_CLANG 0
#        define AILURUS_COMPILER_MSVC 1
#        define AILURUS_COMPILER_GCC 0
#    elif defined(__GNUC__)
#        define AILURUS_COMPILER_CLANG 0
#        define AILURUS_COMPILER_MSVC 0
#        define AILURUS_COMPILER_GCC 1
#    else
#        define AILURUS_COMPILER_CLANG 0
#        define AILURUS_COMPILER_MSVC 0
#        define AILURUS_COMPILER_GCC 0
#    endif
#endif

#ifdef __APPLE__
#   include <TargetConditionals.h>
#   ifdef TARGET_OS_IPHONE
#       define AILURUS_PLATFORM_IOS 1
#   else
#       define AILURUS_PLATFORM_IOS 0
#   endif
#   ifdef TARGET_OS_MAC
#       define AILURUS_PLATFORM_MAC 1
#   else
#       define AILURUS_PLATFORM_MAC 0
#   endif
#else
#   define AILURUS_PLATFORM_IOS 0
#   define AILURUS_PLATFORM_MAC 0
#endif

#define AILURUS_PLATFORM_APPLE (AILURUS_PLATFORM_IOS | AILURUS_PLATFORM_MAC)

#ifdef __ANDROID__
#   define AILURUS_PLATFORM_ANDROID 1
#else
#   define AILURUS_PLATFORM_ANDROID 0
#endif

#ifdef __linux__
#   define AILURUS_PLATFORM_LINUX 1
#else
#   define AILURUS_PLATFORM_LINUX 0
#endif

/* Posix support */
#if AILURUS_PLATFORM_LINUX || AILURUS_PLATFORM_ANDROID || AILURUS_PLATFORM_MAC || AILURUS_PLATFORM_IOS
#    define AILURUS_PLATFORM_SUPPORT_POSIX 1
#else
#    define AILURUS_PLATFORM_SUPPORT_POSIX 0
#endif

/* ISA define */
#if defined(__x86_64__) /* clang & gcc x64 */ || defined(_M_X64) /* msvc x64 */
#    define AILURUS_CHIP_X64 1
#    define AILURUS_CHIP_X86 1
#    define AILURUS_CHIP_ARM64 0
#    define AILURUS_CHIP_ARM32 0
#elif defined(__i386__) /* clang & gcc x86 */ || defined(_M_IX86) /* msvc x86 */
#    define AILURUS_CHIP_X64 0
#    define AILURUS_CHIP_X86 1
#    define AILURUS_CHIP_ARM64 0
#    define AILURUS_CHIP_ARM32 0
#elif defined(__aarch64__) /* arm64 */
#    define AILURUS_CHIP_X64 0
#    define AILURUS_CHIP_X86 0
#    define AILURUS_CHIP_ARM64 1
#    define AILURUS_CHIP_ARM32 0
#elif defined(__arm__) /* arm32 */
#    define AILURUS_CHIP_X64 0
#    define AILURUS_CHIP_X86 0
#    define AILURUS_CHIP_ARM64 0
#    define AILURUS_CHIP_ARM32 1
#else
#    error "Not supported chip platform"
#endif

/* Cpp version
 * - std::format need gcc13
 */
#if defined(_MSC_VER)
#    if _MSVC_LANG >= 202002L
#        define AILURUS_CPP_VERSION 20L
#    elif _MSVC_LANG >= 201703L
#        define AILURUS_CPP_VERSION 17L
#    elif _MSVC_LANG >= 201402L
#        define AILURUS_CPP_VERSION 14L
#    else
#        define AILURUS_CPP_VERSION 11L
#    endif
#elif defined(__GNUC__) || defined(__clang__)
#    if __cplusplus >= 202002L
#        define AILURUS_CPP_VERSION 20L
#    elif __cplusplus >= 201703L
#        define AILURUS_CPP_VERSION 17L
#    elif __cplusplus >= 201402L
#        define AILURUS_CPP_VERSION 14L
#    else
#        define AILURUS_CPP_VERSION 11L
#    endif
#endif

/* Force inline */
#if AILURUS_COMPILER_CLANG || AILURUS_COMPILER_GCC
#    define AILURUS_FORCE_INLINE inline __attribute__ ((always_inline))
#elif AILURUS_COMPILER_MSVC
#    define AILURUS_FORCE_INLINE __forceinline
#else
#    define AILURUS_FORCE_INLINE inline
#endif

/* Feature definition */
#define AILURUS_FEAT_SUPPORT_WINDOW AILURUS_PLATFORM_WINDOWS || AILURUS_PLATFORM_MAC
