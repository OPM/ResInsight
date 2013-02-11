/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ranking_table.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __RANKING_TABLE_H__
#define __RANKING_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/enkf/misfit_ensemble_typedef.h>

  typedef struct ranking_table_struct ranking_table_type;

  void                 ranking_table_set_ens_size( ranking_table_type * table, int ens_size);
  ranking_table_type * ranking_table_alloc( ) ; 
  void                 ranking_table_free( ranking_table_type * table );

  void                 ranking_table_add_data_ranking( ranking_table_type * ranking_table , 
                                                       bool sort_increasing , 
                                                       const char * ranking_key , 
                                                       const char * user_key , 
                                                       const char * key_index , 
                                                       enkf_fs_type * fs , 
                                                       const enkf_config_node_type * config_node , 
                                                       int step , 
                                                       state_enum state);

  bool                 ranking_table_has_ranking( const ranking_table_type * ranking_table , const char * ranking_key );
  bool                 ranking_table_display_ranking( const ranking_table_type * ranking_table , const char * ranking_key );

  void                 ranking_table_add_misfit_ranking( ranking_table_type * ranking_table , 
                                                         const misfit_ensemble_type * misfit_ensemble , 
                                                         const stringlist_type * obs_keys , 
                                                         int step1 , 
                                                         int step2 , 
                                                         const char * ranking_key);


  int                  ranking_table_get_size( const ranking_table_type * ranking_table );
  const int          * ranking_table_get_permutation( const ranking_table_type * ranking_table , const char * ranking_key);

#ifdef __cplusplus
}
#endif


#endif
