/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'thread_pool.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#ifdef  HAVE_PTHREAD
#define HAVE_THREAD_POOL   
#include "thread_pool_posix.h"
#endif

#endif
