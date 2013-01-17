/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_plot_arg.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef __ENKF_PLOT_ARG_H__
#define __ENKF_PLOT_ARG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdbool.h>


  typedef struct enkf_plot_arg_struct enkf_plot_arg_type;
  
  enkf_plot_arg_type * enkf_plot_arg_alloc(bool time_mode, time_t start_time);
  enkf_plot_arg_type * enkf_plot_arg_alloc( bool time_mode , time_t start_time);
  void                 enkf_plot_arg_free( enkf_plot_arg_type * plot_arg );
  void                 enkf_plot_arg_free__( void * arg );
  void                 enkf_plot_arg_reset( enkf_plot_arg_type * plot_arg , bool time_mode , time_t start_time);

#ifdef __cplusplus
}
#endif
#endif
