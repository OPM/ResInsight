
#pragma once

#include <cstdio>
#include <cstdlib>

// CAF_ASSERT documents an invariant that must hold if the program is correct. It is meant for
// programming errors, not for run-time conditions that can legitimately occur (bad input files,
// missing data, failed network calls) - those need real error handling.
//
// Following the semantics of the standard assert(), CAF_ASSERT is active in debug builds and
// compiled out in optimized builds (NDEBUG). A failing assert prints file/line/expression and
// calls std::abort(), which raises SIGABRT and is picked up by the crash handler installed in
// RiaMain.cpp, so the failure is written to the log file and reported to OpenTelemetry with a
// stack trace.
//
// Developers who want the asserts to stay active in an optimized build (typically RelWithDebInfo)
// can configure with -DRESINSIGHT_ENABLE_ASSERTS_IN_RELEASE=ON, which defines CAF_ENABLE_ASSERTS=1
// for the whole build.
//
// CAF_ENABLE_ASSERTS is given a default here so this header stays self-contained: include order can
// never silently switch the asserts off. See OPM/ResInsight#14397 for the class of bug this avoids.
#ifndef CAF_ENABLE_ASSERTS
#ifdef NDEBUG
#define CAF_ENABLE_ASSERTS 0
#else
#define CAF_ENABLE_ASSERTS 1
#endif
#endif

#if CAF_ENABLE_ASSERTS == 1

#define CAF_ASSERT( expr )                                                                            \
    do                                                                                                \
    {                                                                                                 \
        if ( !( expr ) ) /* NOLINT */                                                                 \
        {                                                                                             \
            std::fprintf( stderr, "%s : %i : CAF_ASSERT( %s ) failed\n", __FILE__, __LINE__, #expr ); \
            std::abort();                                                                             \
        }                                                                                             \
    } while ( false )

#else

// The expression is kept inside an unevaluated sizeof rather than discarded outright. It is not
// evaluated, so there is no run-time cost and no side effects, but it is still type checked (the
// assert cannot bit-rot into something that no longer compiles) and any variable used only by the
// assert still counts as referenced, which avoids a wave of unused-variable warnings.
#define CAF_ASSERT( expr ) ( (void)sizeof( !( expr ) ) ) /* NOLINT */

#endif
