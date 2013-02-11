/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'misfit_ranking.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __MISFIT_RANKING_H__
#define __MISFIT_RANKING_H__

#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_fs.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct misfit_ranking_struct misfit_ranking_type;

  UTIL_IS_INSTANCE_HEADER( misfit_ranking );
  UTIL_SAFE_CAST_HEADER( misfit_ranking );
  
  void                  misfit_ranking_fprintf( const misfit_ranking_type * misfit_ranking , const char * filename);
  void                  misfit_ranking_display( const misfit_ranking_type * misfit_ranking , FILE * stream);
  misfit_ranking_type * misfit_ranking_alloc(const misfit_ensemble_type * ensemble , const stringlist_type * sort_keys , int step1 , int step2, const char * ranking_key);
  void                  misfit_ranking_free( misfit_ranking_type * misfit_ranking );
  void                  misfit_ranking_free__( void * arg );
  const int           * misfit_ranking_get_permutation( const misfit_ranking_type * misfit_ranking );
  void                  misfit_ranking_iset_invalid( misfit_ranking_type * misfit_ranking , int iens );
  void                  misfit_ranking_iset( misfit_ranking_type * misfit_ranking , int iens , hash_type * obs_hash , double total_misfit);
  void                  misfit_ranking_init_sort( misfit_ranking_type * misfit_ranking );
  
#ifdef __cplusplus
}
#endif

#endif
