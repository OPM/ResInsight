/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'misfit_member.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __MISFIT_MEMBER_H__
#define __MISFIT_MEMBER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <ert/enkf/misfit_ts.h>

  typedef struct misfit_member_struct    misfit_member_type;
  misfit_ts_type     * misfit_member_get_ts( const misfit_member_type * member , const char * obs_key );
  bool                 misfit_member_has_ts( const misfit_member_type * member , const char * obs_key );
  misfit_member_type * misfit_member_fread_alloc( FILE * stream );
  void                 misfit_member_fwrite( const misfit_member_type * node , FILE * stream );
  void                 misfit_member_update( misfit_member_type * node , const char * obs_key , int history_length , int iens , const double ** work_chi2);
  void                 misfit_member_free__( void * node );
  misfit_member_type * misfit_member_alloc(int iens);


#ifdef __cplusplus
}
#endif

#endif
