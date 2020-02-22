/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ssize_t.h' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#ifndef ERT_SSIZE_T_H
#define ERT_SSIZE_T_H

#ifdef _MSC_VER
/* maximum number of bytes addressable */
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef long ssize_t;
#endif
#else
/* POSIX 2008 states that it should be defined here */
#include <sys/types.h>
#endif

#endif
