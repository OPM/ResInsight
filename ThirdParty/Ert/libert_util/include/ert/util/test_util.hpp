/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'test_util.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef __TEST_UTIL_HPP__
#define __TEST_UTIL_HPP__

#include <ert/util/test_util.h>

#define test_assert_throw(expr , exception_type ) \
{                          		     \
   bool throw_ok = false;  		     \
   try {                   		     \
        expr;                                \
   }                                         \
   catch (std::exception &e) {	             \
   if (dynamic_cast<exception_type *>(&e))   \
       throw_ok = true;                      \
   }                                         \
   if (!throw_ok)                            \
      test_error_exit("Correct exception not thrown at %s:%d\n",__FILE__ , __LINE__); \
}



#endif
