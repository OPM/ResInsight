
#pragma once

#include <cstdlib>
#include <iostream>

#define CAF_ASSERT( expr )                                                                                   \
    do                                                                                                       \
    {                                                                                                        \
        if ( !( expr ) )                                                                                     \
        {                                                                                                    \
            std::cout << __FILE__ << ":" << __LINE__ << ": CAF_ASSERT(" << #expr << ") failed" << std::endl; \
            std::abort();                                                                                    \
        }                                                                                                    \
    } while ( false )

#if 0 // Bits and pieces for reference to improve the assert
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4668 )
#include <windows.h>
#pragma warning( pop )
#endif


void openDebugWindow()
{
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);
#pragma warning( pop )
#endif
}

void assertAbort()
{
#ifdef _MSC_VER
#if ( _MSC_VER >= 1600 )
    //if (::IsDebuggerPresent())
#endif
    {
        __debugbreak();
    }
#endif

    std::terminate();
}

#define ASSERT_TEST( expr )                                                                                  \
    do                                                                                                       \
    {                                                                                                        \
        if ( !( expr ) )                                                                                     \
        {                                                                                                    \
            std::cout << __FILE__ << ":" << __LINE__ << ": CAF_ASSERT(" << #expr << ") failed" << std::endl; \
            assertAbort();                                                                                   \
        }                                                                                                    \
    } while ( false )
#endif