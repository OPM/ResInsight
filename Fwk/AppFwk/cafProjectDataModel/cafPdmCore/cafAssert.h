
#pragma once

#include <cstdio>
#include <cstdlib>

#define CAF_ASSERT( expr )                                                                   \
    do                                                                                       \
    {                                                                                        \
        if ( !( expr ) ) /* NOLINT */                                                        \
        {                                                                                    \
            std::printf( "%s : %i : CAF_ASSERT( %s ) failed\n", __FILE__, __LINE__, #expr ); \
            std::abort();                                                                    \
        }                                                                                    \
    } while ( false )
