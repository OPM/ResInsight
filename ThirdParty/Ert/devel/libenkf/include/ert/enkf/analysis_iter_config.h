/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'analysis_iter_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ANALYSIS_ITER_CONFIG_H__
#define __ANALYSIS_ITER_CONFIG_H__

#ifdef __cplusplus 
extern "C" {
#endif


typedef struct analysis_iter_config_struct analysis_iter_config_type;

  void                            analysis_iter_config_set_num_iterations( analysis_iter_config_type * config , int num_iterations);
  int                             analysis_iter_config_get_num_iterations( const analysis_iter_config_type * config );
  void                            analysis_iter_config_set_case_fmt( analysis_iter_config_type * config, const char * case_fmt);
  char *                          analysis_iter_config_get_case_fmt( analysis_iter_config_type * config);
  analysis_iter_config_type * analysis_iter_config_alloc();
  void                            analysis_iter_config_free( analysis_iter_config_type * config );
  const char *                    analysis_iter_config_iget_case( analysis_iter_config_type * config , int iter);
  const char *                    analysis_iter_config_iget_runpath_fmt( analysis_iter_config_type * config , int iter);
  void                            analysis_iter_config_add_config_items( config_type * config );
  void                            analysis_iter_config_init(analysis_iter_config_type * iter_config , const config_type * config);

#ifdef __cplusplus 
}
#endif
#endif
