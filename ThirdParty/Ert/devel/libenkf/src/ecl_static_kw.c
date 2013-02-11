/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_static_kw.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>

#include <ert/util/util.h>
#include <ert/util/buffer.h>

#include <ert/ecl/ecl_kw.h>

#include <ert/enkf/ecl_static_kw.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_macros.h>



struct ecl_static_kw_struct {
  UTIL_TYPE_ID_DECLARATION;
  ecl_kw_type * ecl_kw;  /* Mostly NULL */
};




ecl_static_kw_type * ecl_static_kw_alloc( ) {  
  ecl_static_kw_type * static_kw = util_malloc(sizeof *static_kw );
  UTIL_TYPE_ID_INIT( static_kw , STATIC );
  static_kw->ecl_kw        = NULL;
  return static_kw;
}


/*
  ptr is pure dummy to satisfy the api.
*/
void * ecl_static_kw_alloc__(const void *ptr) {  
  return ecl_static_kw_alloc( );
}




ecl_kw_type * ecl_static_kw_ecl_kw_ptr(const ecl_static_kw_type * ecl_static) { return ecl_static->ecl_kw; }


void ecl_static_kw_copy(const ecl_static_kw_type *src , ecl_static_kw_type * target) {
  if (src->ecl_kw != NULL)
    target->ecl_kw = ecl_kw_alloc_copy(src->ecl_kw);
}



void ecl_static_kw_free(ecl_static_kw_type * kw) {
  if (kw->ecl_kw != NULL) ecl_kw_free(kw->ecl_kw);
  free(kw);
}


void ecl_static_kw_init(ecl_static_kw_type * ecl_static_kw, const ecl_kw_type * ecl_kw) {
  if (ecl_static_kw->ecl_kw != NULL)
    util_abort("%s: internal error: trying to assign ecl_kw to ecl_static_kw which is already set.\n",__func__);
  
  ecl_static_kw->ecl_kw = ecl_kw_alloc_copy(ecl_kw);
}








void ecl_static_kw_read_from_buffer(ecl_static_kw_type * ecl_static_kw , buffer_type * buffer, int report_step, state_enum state) {
  enkf_util_assert_buffer_type( buffer , STATIC );
  if (ecl_static_kw->ecl_kw != NULL)
    util_abort("%s: internal error: trying to assign ecl_kw to ecl_static_kw which is already set.\n",__func__);
  ecl_static_kw->ecl_kw = ecl_kw_buffer_alloc( buffer );
}


static void ecl_static_kw_free_data(ecl_static_kw_type * kw) {
  if (kw->ecl_kw != NULL) ecl_kw_free(kw->ecl_kw);
  kw->ecl_kw = NULL;
}


/**
   The ecl_kw instance is discarded immediately after writing to disk·
   For both ecl_write and internal storage.
*/


void ecl_static_kw_ecl_write(const ecl_static_kw_type * ecl_static, const char * run_path /* Not used*/  , const char * path /* Not used */, fortio_type * fortio) {
  ecl_kw_fwrite(ecl_static->ecl_kw , fortio);
}



bool ecl_static_kw_write_to_buffer(const ecl_static_kw_type * ecl_static_kw , buffer_type * buffer, int report_step , state_enum state) {
  buffer_fwrite_int( buffer , STATIC );
  ecl_kw_buffer_store( ecl_static_kw->ecl_kw , buffer);
  return true;
}







/*****************************************************************/
VOID_FREE_DATA(ecl_static_kw);
UTIL_SAFE_CAST_FUNCTION(ecl_static_kw , STATIC)
UTIL_SAFE_CAST_FUNCTION_CONST(ecl_static_kw , STATIC)
VOID_FREE(ecl_static_kw)
VOID_ECL_WRITE (ecl_static_kw)
VOID_COPY(ecl_static_kw)
VOID_READ_FROM_BUFFER(ecl_static_kw)
VOID_WRITE_TO_BUFFER(ecl_static_kw)
