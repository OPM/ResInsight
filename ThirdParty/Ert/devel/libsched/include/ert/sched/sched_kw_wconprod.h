/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_wconprod.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_KW_WCONPROD_H__
#define __SCHED_KW_WCONPROD_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <ert/sched/sched_macros.h>


  typedef struct sched_kw_wconprod_struct sched_kw_wconprod_type;


  char   ** sched_kw_wconprod_alloc_wells_copy( const sched_kw_wconprod_type * , int * );
  void      sched_kw_wconprod_init_well_list( const sched_kw_wconprod_type * kw , stringlist_type * well_list);



KW_HEADER(wconprod)

#ifdef __cplusplus
}
#endif
#endif
