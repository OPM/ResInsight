/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'cases_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_CASES_CONFIG_H
#define ERT_CASES_CONFIG_H

#ifdef __cplusplus 
extern "C" {
#endif


typedef struct cases_config_struct cases_config_type;

  bool                            cases_config_set_int( cases_config_type * config , const char * var_name, int num_iterations);
  int                             cases_config_get_iteration_number( const cases_config_type * config );
  void                            cases_config_fwrite( cases_config_type * config , const char * filename );
  void                            cases_config_fread( cases_config_type * config , const char * filename);
  cases_config_type * cases_config_alloc();
  void                            cases_config_free( cases_config_type * config );

#ifdef __cplusplus 
}
#endif
#endif
