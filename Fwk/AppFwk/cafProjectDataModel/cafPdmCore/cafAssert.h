
#pragma once

#include <cstdio>
#include <cstdlib>

// Break into an attached debugger at the assert site. std::abort() alone does not trigger a debug
// break with the release CRT, so asserts in RelWithDebInfo builds would terminate without stopping
// in the debugger. Without a debugger attached, the breakpoint exception terminates the process
// with the assert site at the top of the crash stack.
#if defined( _MSC_VER )
#define CAF_DEBUG_BREAK() __debugbreak()
#else
#define CAF_DEBUG_BREAK() __builtin_trap()
#endif

#define CAF_ASSERT( expr )                                                                   \
    do                                                                                       \
    {                                                                                        \
        if ( !( expr ) ) /* NOLINT */                                                        \
        {                                                                                    \
            std::printf( "%s : %i : CAF_ASSERT( %s ) failed\n", __FILE__, __LINE__, #expr ); \
            CAF_DEBUG_BREAK();                                                               \
            std::abort();                                                                    \
        }                                                                                    \
    } while ( false )
