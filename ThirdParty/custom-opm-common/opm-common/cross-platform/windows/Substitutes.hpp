#pragma once

#ifdef _WIN32

// IGNORE is used in InputErrorAction.hpp, as an enum value,
// but is defined as a #def in WinBase.h which is included by Shlwapi.h.
// It is not required here, so we can undefine it.
#undef IGNORE
#undef GROUP_NAME
#undef ERROR
#include <iostream>
#include <stddef.h>
#include <time.h>

int fnmatch(const char* pattern, const char* string, int flags);

struct tm* gmtime_r(const time_t* timer, struct tm* buf);

#endif