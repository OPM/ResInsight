/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_main_jobs.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ENKF_MAIN_JOBS_H
#define ENKF_MAIN_JOBS_H

#ifdef __cplusplus
extern "C" {
#endif


void * enkf_main_select_case_JOB( void * self , const stringlist_type * args);
void * enkf_main_create_case_JOB( void * self , const stringlist_type * args);


#ifdef __cplusplus
}
#endif

#endif 
