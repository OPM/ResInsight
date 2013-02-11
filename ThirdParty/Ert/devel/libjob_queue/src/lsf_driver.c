/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lsf_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

/**
   If the symbol INCLUDE_LSF is not defined a dummy LSF driver will be
   compiled. This driver has stubs for all the driver related
   functions, and a dummy driver can be created with the
   queue_driver_alloc_LSF() function.

   If one of the queue_driver function pointers pointing down to one
   of the lsf_driver_xxx() functions is actually invoked (e.g. through
   the queue layer) the program will exit with an error message. This
   is only a utility to avoid changing the source when the library is
   built and used on a platform without LSF inst1alled.
   
   When compiling with proper LSF support the preprocessor symbol
   INCLUDE_LSF must be set (to an arbitrary value), in addition the
   libraries liblsf, libbat and libnsl must be linked in when creating
   the final executable.  
*/

#ifdef HAVE_LSF
#include "lsf_driver_impl.c"
#else
#include "lsf_driver_dummy.c"
#endif
