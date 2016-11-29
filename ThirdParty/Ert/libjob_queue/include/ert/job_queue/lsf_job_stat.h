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

#ifdef HAVE_LSF_LIBRARY
#include <lsf/lsbatch.h>
#else
#define JOB_STAT_NULL         0
#define JOB_STAT_PEND         1
#define JOB_STAT_SSUSP        0x08       
#define JOB_STAT_USUSP        0x10       
#define JOB_STAT_PSUSP        0x02                            		
#define JOB_STAT_RUN          0x04
#define JOB_STAT_EXIT         0x20
#define JOB_STAT_DONE         0x40
#define JOB_STAT_PDONE        0x80
#define JOB_STAT_UNKWN        0x10000    
#endif
