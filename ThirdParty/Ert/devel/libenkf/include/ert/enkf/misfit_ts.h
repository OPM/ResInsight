/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'misfit_ts.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __MISFIT_TS_H__
#define __MISFIT_TS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

  typedef struct misfit_ts_struct        misfit_ts_type;
  

  void                 misfit_ts_fwrite( const misfit_ts_type * misfit_ts , FILE * stream );
  double               misfit_ts_eval( const misfit_ts_type * ts , int step1 , int step2 );
  misfit_ts_type     * misfit_ts_alloc(int history_length);
  misfit_ts_type     * misfit_ts_fread_alloc( FILE * stream );
  void                 misfit_ts_free__( void * vector );
  void                 misfit_ts_iset( misfit_ts_type * vector , int time_index , double value );

#ifdef __cplusplus
}
#endif

#endif
