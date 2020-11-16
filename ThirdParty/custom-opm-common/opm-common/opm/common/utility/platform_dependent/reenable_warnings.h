/*
  Copyright 2014 SINTEF ICT, Applied Mathematics.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

// Note: this file shall not have include guards or #pragma once.

#ifdef SILENCE_EXTERNAL_WARNINGS

// To use this feature, we must have sufficiently new compiler.

// Using gcc is ok if version 4.6 or newer.
#if defined(__GNUC__)
#  define __GNUC_VERSION__ (__GNUC__ * 100 \
                            + __GNUC_MINOR__ * 1)
#  if (__GNUC_VERSION__ >= 406)
#    define GNU_COMPILER_OK 1
#  else
#    define GNU_COMPILER_OK 0
#  endif
#else
#  define GNU_COMPILER_OK 0
#endif

// Uncertain what version of clang to require,
// assume all versions are fine.
#if defined(__clang__)
#  define CLANG_COMPILER_OK 1
#else
#  define CLANG_COMPILER_OK 0
#endif

// More compilers can be added here if necessary.
#define COMPATIBLE_COMPILER (GNU_COMPILER_OK || CLANG_COMPILER_OK)

// If compiler is compatible, pop current warning level.
// Note that both clang and (newer) gcc accept the
// "#pragma GCC diagnostic" syntax.
#if COMPATIBLE_COMPILER
#pragma GCC diagnostic pop
#endif // COMPATIBLE_COMPILER

#endif // SILENCE_EXTERNAL_WARNINGS
