/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'well_conn.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_WELL_CONN_H
#define ERT_WELL_CONN_H



#include <stdbool.h>

#include <ert/util/type_macros.hpp>

#include <ert/ecl/ecl_rsthead.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum {
    well_conn_dirX  = 1,
    well_conn_dirY  = 2,
    well_conn_dirZ  = 3,
    well_conn_fracX = 4,
    well_conn_fracY = 5
  } well_conn_dir_enum;


  typedef struct well_conn_struct well_conn_type;


  void             well_conn_free( well_conn_type * conn);
  void             well_conn_free__( void * arg );

  well_conn_type * well_conn_alloc( int i , int j , int k , double connection_factor , well_conn_dir_enum dir, bool open);
  well_conn_type * well_conn_alloc_MSW( int i , int j , int k , double connection_factor , well_conn_dir_enum dir, bool open, int segment);
  well_conn_type * well_conn_alloc_fracture( int i , int j , int k , double connection_factor , well_conn_dir_enum dir, bool open);
  well_conn_type * well_conn_alloc_fracture_MSW( int i , int j , int k , double connection_factor , well_conn_dir_enum dir, bool open, int segment);

  bool             well_conn_MSW(const well_conn_type * conn);

  well_conn_type * well_conn_alloc_from_kw( const ecl_kw_type * icon_kw , const ecl_kw_type * scon_kw , const ecl_kw_type* xcon_kw, const ecl_rsthead_type * header , int well_nr , int conn_nr);
  well_conn_type * well_conn_alloc_wellhead( const ecl_kw_type * iwel_kw , const ecl_rsthead_type * header , int well_nr);

  int                well_conn_get_i(const well_conn_type * conn);
  int                well_conn_get_j(const well_conn_type * conn);
  int                well_conn_get_k(const well_conn_type * conn);
  well_conn_dir_enum well_conn_get_dir(const well_conn_type * conn);
  bool               well_conn_open( const well_conn_type * conn );
  int                well_conn_get_segment_id( const well_conn_type * conn );
  bool               well_conn_fracture_connection( const well_conn_type * conn);
  bool               well_conn_matrix_connection( const well_conn_type * conn);
  bool               well_conn_equal( const well_conn_type *conn1  , const well_conn_type * conn2);
  double             well_conn_get_connection_factor( const well_conn_type * conn );

  double             well_conn_get_oil_rate(const well_conn_type *conn);
  double             well_conn_get_gas_rate(const well_conn_type *conn);
  double             well_conn_get_water_rate(const well_conn_type *conn);
  double             well_conn_get_volume_rate(const well_conn_type *conn);

  double             well_conn_get_oil_rate_si(const well_conn_type *conn);
  double             well_conn_get_gas_rate_si(const well_conn_type *conn);
  double             well_conn_get_water_rate_si(const well_conn_type *conn);
  double             well_conn_get_volume_rate_si(const well_conn_type *conn);


UTIL_IS_INSTANCE_HEADER( well_conn );
  UTIL_SAFE_CAST_HEADER( well_conn );


#ifdef __cplusplus
}
#endif
#endif
