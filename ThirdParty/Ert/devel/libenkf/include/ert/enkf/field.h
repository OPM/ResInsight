/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FIELD_H__
#define __FIELD_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/type_macros.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/field_common.h>

/* Typedef field_type moved to field_config.h */

  void         field_scale(field_type * field, double scale_factor);
  int          field_get_global_index(const field_type * , int  , int  , int );
  void         field_ijk_set(field_type * , int  , int  , int  , const void * );
  void         field_indexed_set(field_type * field, ecl_type_enum , int , const int * , const void * );
  void         field_indexed_add(field_type * field, ecl_type_enum , int , const int * , const void * );
  double       field_iget_double(const field_type * , int );
  double       field_ijk_get_double(const field_type * field, int  , int  , int );
  float        field_iget_float(const field_type * , int );
  float        field_ijk_get_float(const field_type * field, int  , int  , int );
  void         field_ijk_get(const field_type * , int , int  , int , void *);
  bool         field_ijk_valid(const field_type * , int , int , int );
  void         field_ijk_get_if_valid(const field_type * , int  , int  , int , void * , bool *);
  void         field_ecl_write1D_fortio(const field_type * , fortio_type *);
  void         field_ecl_write3D_fortio(const field_type * , fortio_type *);
  void         field_ROFF_export(const field_type * , const char * );
  void         field_copy_ecl_kw_data(field_type * , const ecl_kw_type * );
  field_type * field_alloc_shared(const field_config_type * , void * , int );
  void         field_free(field_type *);
  void         field_get_dims(const field_type *, int *, int *, int *);
  void         field_fload_auto(field_type * , const char * );
  void         field_export3D(const field_type * , void *, bool , ecl_type_enum , void *);
  void         field_export(const field_type * , const char * , fortio_type * , field_file_format_type , bool);
  field_type * field_copyc(const field_type *);
  bool         field_cmp(const field_type *  , const field_type * );
  
  double     * field_indexed_get_alloc(const field_type *, int, const int *);
  void         field_inplace_output_transform(field_type * field);
  
  void          field_iscale(field_type * , double );
  void          field_isqrt(field_type *);
  void          field_isqr(field_type *);
  void          field_iaddsqr(field_type * , const field_type *);
  void          field_iadd(field_type * , const field_type *);
  void          field_imul_add(field_type * , double , const field_type *);
  ecl_kw_type * field_alloc_ecl_kw_wrapper(const field_type * );
  void          field_update_sum(field_type * sum , const field_type * field , double lower_limit , double upper_limit);
  void          field_upgrade_103(const char * filename);
  
  UTIL_IS_INSTANCE_HEADER(field);
  UTIL_SAFE_CAST_HEADER_CONST(field);
  VOID_ALLOC_HEADER(field);
  VOID_FREE_HEADER(field);
  VOID_COPY_HEADER      (field);
  VOID_INITIALIZE_HEADER(field);
  VOID_ECL_WRITE_HEADER (field);
  VOID_FORWARD_LOAD_HEADER(field);
  VOID_USER_GET_HEADER(field);
  VOID_READ_FROM_BUFFER_HEADER(field);
  VOID_WRITE_TO_BUFFER_HEADER(field);
  VOID_SERIALIZE_HEADER(field);
  VOID_DESERIALIZE_HEADER(field);
  VOID_CLEAR_HEADER(field);
  VOID_SET_INFLATION_HEADER(field);
  VOID_IMUL_HEADER(field);
  VOID_IADD_HEADER(field);
  VOID_IADDSQR_HEADER(field);
  VOID_SCALE_HEADER(field);
  VOID_ISQRT_HEADER(field);
  VOID_FLOAD_HEADER(field);

#ifdef __cplusplus
}
#endif
#endif

