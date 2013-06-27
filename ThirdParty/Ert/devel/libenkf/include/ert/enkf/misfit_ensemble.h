/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'misfit_table.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __MISFIT_ENSEMBLE_H__
#define __MISFIT_ENSEMBLE_H__

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/misfit_member.h>

#define MISFIT_DEFAULT_RANKING_KEY "DEFAULT"
#include <ert/enkf/misfit_ensemble_typedef.h>



  void                misfit_ensemble_fread( misfit_ensemble_type * misfit_ensemble , FILE * stream );
  void                misfit_ensemble_clear( misfit_ensemble_type * table);
  misfit_ensemble_type * misfit_ensemble_alloc( );
  void                misfit_ensemble_free( misfit_ensemble_type * table );
  void                misfit_ensemble_fwrite( const misfit_ensemble_type * misfit_ensemble , FILE * stream);
  bool                misfit_ensemble_initialized( const misfit_ensemble_type * misfit_ensemble );
  void                misfit_ensemble_update( misfit_ensemble_type * misfit_ensemble , const ensemble_config_type * ensemble_config , const enkf_obs_type * enkf_obs , enkf_fs_type * fs , int ens_size , int history_length);
  void                misfit_ensemble_set_ens_size( misfit_ensemble_type * misfit_ensemble , int ens_size);
  int                 misfit_ensemble_get_ens_size( const misfit_ensemble_type * misfit_ensemble );

  misfit_member_type * misfit_ensemble_iget_member( const misfit_ensemble_type * table , int iens);

  
  
#ifdef __cplusplus
}
#endif

#endif
