/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'test_util_abort.h' is part of ERT - Ensemble based Reservoir Tool.

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

/*
  This header is purely a convenience header - it is not installed.
*/

#ifndef __TEST_UTIL_ABORT__
#define __TEST_UTIL_ABORT__

#ifdef __cplusplus
extern "C" {
#endif

#include <setjmp.h>

  jmp_buf * util_abort_test_jump_buffer();
  void   test_util_addr2line();
  void   test_assert_util_abort(const char * function_name , void call_func (void *) , void * arg);

#ifdef __cplusplus
}
#endif
#endif

