 /*
   Copyright (C) 2012  Equinor ASA, Norway.

   The file 'ecl_sum_tstep.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_SUM_TSTEP_H
#define ERT_ECL_SUM_TSTEP_H

#include <ert/util/int_vector.hpp>

#include <ert/ecl/ecl_smspec.hpp>
#include <ert/ecl/ecl_kw.hpp>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecl_sum_tstep_struct ecl_sum_tstep_type;

  ecl_sum_tstep_type * ecl_sum_tstep_alloc_remap_copy( const ecl_sum_tstep_type * src , const ecl_smspec_type * new_smspec, float default_value , const int * params_map);
  ecl_sum_tstep_type * ecl_sum_tstep_alloc_copy( const ecl_sum_tstep_type * src );
  void ecl_sum_tstep_free( ecl_sum_tstep_type * ministep );
  void ecl_sum_tstep_free__( void * __ministep);
  ecl_sum_tstep_type * ecl_sum_tstep_alloc_from_file(int report_step    ,
                                                     int ministep_nr            ,
                                                     const ecl_kw_type * params_kw ,
                                                     const char * src_file ,
                                                     const ecl_smspec_type * smspec);

  ecl_sum_tstep_type * ecl_sum_tstep_alloc_new( int report_step , int ministep , float sim_seconds , const ecl_smspec_type * smspec );

  void   ecl_sum_tstep_set_from_node( ecl_sum_tstep_type * tstep , const ecl::smspec_node& smspec_node , float value);
  double ecl_sum_tstep_get_from_node( const ecl_sum_tstep_type * tstep , const ecl::smspec_node& smspec_node);

  double ecl_sum_tstep_iget(const ecl_sum_tstep_type * ministep , int index);
  time_t ecl_sum_tstep_get_sim_time(const ecl_sum_tstep_type * ministep);
  double ecl_sum_tstep_get_sim_days(const ecl_sum_tstep_type * ministep);
  double ecl_sum_tstep_get_sim_seconds(const ecl_sum_tstep_type * ministep);

  int  ecl_sum_tstep_get_report(const ecl_sum_tstep_type * ministep);
  int  ecl_sum_tstep_get_ministep(const ecl_sum_tstep_type * ministep);

  void ecl_sum_tstep_fwrite( const ecl_sum_tstep_type * ministep , const int * index_map , int index_map_size, fortio_type * fortio);
  void ecl_sum_tstep_iset( ecl_sum_tstep_type * tstep , int index , float value);

  /// scales with value; equivalent to iset( iget() * scalar)
  void ecl_sum_tstep_iscale(ecl_sum_tstep_type * tstep, int index, float scalar);

  /// adds addend to tstep[index]; equivalent to iset( iget() + addend)
  void ecl_sum_tstep_ishift(ecl_sum_tstep_type * tstep, int index, float addend);


  void ecl_sum_tstep_set_from_key( ecl_sum_tstep_type * tstep , const char * gen_key , float value);
  double ecl_sum_tstep_get_from_key( const ecl_sum_tstep_type * tstep , const char * gen_key);
  bool ecl_sum_tstep_has_key(const ecl_sum_tstep_type * tstep , const char * gen_key);

  bool ecl_sum_tstep_sim_time_equal( const ecl_sum_tstep_type * tstep1 , const ecl_sum_tstep_type * tstep2 );

  UTIL_SAFE_CAST_HEADER( ecl_sum_tstep );
  UTIL_SAFE_CAST_HEADER_CONST( ecl_sum_tstep );



#ifdef __cplusplus
}


#endif
#endif
