/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_kw.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_GEN_KW_H
#define ERT_GEN_KW_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/subst_list.h>
#include <ert/util/double_vector.h>

#include <ert/enkf/gen_kw_config.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/gen_kw_common.h>



void             gen_kw_output_transform(gen_kw_type * );
void             gen_kw_get_output_data(const gen_kw_type * , double * );
const double   * gen_kw_get_output_ref(const gen_kw_type * );
const double   * gen_kw_get_data_ref(const gen_kw_type * );
  //void             gen_kw_get_data(const gen_kw_type * , double * );

void             gen_kw_free(gen_kw_type *);
int              gen_kw_data_size( gen_kw_type * );
double           gen_kw_data_iget( gen_kw_type * , int , bool );
void             gen_kw_data_iset( gen_kw_type * , int , double );
void             gen_kw_data_set_vector( gen_kw_type * gen_kw, const double_vector_type * values );
double           gen_kw_data_get( gen_kw_type * , const char * , bool );
void             gen_kw_data_set( gen_kw_type *, const char *, double );
bool             gen_kw_data_has_key( gen_kw_type *, const char *);
void             gen_kw_ens_write(const gen_kw_type * , const char *);
void             gen_kw_ens_read(gen_kw_type * , const char *);
void             gen_kw_truncate(gen_kw_type * );
gen_kw_type   *  gen_kw_alloc_mean(int , const gen_kw_type **);
const char     * gen_kw_get_name(const gen_kw_type * , int );
void             gen_kw_filter_file(const gen_kw_type * , const char * );
void             gen_kw_export(const gen_kw_type * , int * , char ***, double **);
void             gen_kw_upgrade_103( const char * filename );
char           * gen_kw_alloc_user_key(const gen_kw_config_type *  , const char * , int );
void             gen_kw_set_subst_parent(gen_kw_type * gen_kw , const subst_list_type * parent_subst);
void             gen_kw_ecl_write_template(const gen_kw_type * gen_kw , const char * file_name);


UTIL_SAFE_CAST_HEADER(gen_kw);
UTIL_SAFE_CAST_HEADER_CONST(gen_kw);
ALLOC_STATS_HEADER(gen_kw)
VOID_ECL_WRITE_HEADER  (gen_kw)
VOID_COPY_HEADER (gen_kw);
VOID_INITIALIZE_HEADER(gen_kw);
VOID_FREE_HEADER       (gen_kw);
  //MATH_OPS_VOID_HEADER(gen_kw);
VOID_ALLOC_HEADER(gen_kw);
VOID_ECL_WRITE_HEADER(gen_kw);
VOID_USER_GET_HEADER(gen_kw);
VOID_WRITE_TO_BUFFER_HEADER(gen_kw);
VOID_READ_FROM_BUFFER_HEADER(gen_kw);
VOID_FLOAD_HEADER(gen_kw);
VOID_CLEAR_HEADER(gen_kw);
VOID_SERIALIZE_HEADER(gen_kw)
VOID_DESERIALIZE_HEADER(gen_kw)
VOID_IADD_HEADER(gen_kw);
VOID_IMUL_HEADER(gen_kw);
VOID_SCALE_HEADER(gen_kw);
VOID_IADDSQR_HEADER(gen_kw);
VOID_ISQRT_HEADER(gen_kw);
VOID_SET_INFLATION_HEADER(gen_kw);
#ifdef __cplusplus
}
#endif
#endif
