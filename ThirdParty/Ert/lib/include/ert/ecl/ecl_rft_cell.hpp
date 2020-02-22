/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_rft_cell.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_RFT_CELL_H
#define ERT_ECL_RFT_CELL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.hpp>

#define ECL_RFT_CELL_INVALID_VALUE -1

typedef struct ecl_rft_cell_struct ecl_rft_cell_type;




UTIL_IS_INSTANCE_HEADER( ecl_rft_cell );

  ecl_rft_cell_type * ecl_rft_cell_alloc_PLT( int i ,
                                              int j ,
                                              int k ,
                                              double depth ,
                                              double pressure ,
                                              double orat ,
                                              double grat ,
                                              double wrat ,
                                              double connection_start,
                                              double connection_end,
                                              double flowrate,
                                              double oil_flowrate,
                                              double gas_flowrate,
                                              double water_flowrate);

  ecl_rft_cell_type * ecl_rft_cell_alloc_RFT( int i , int j , int k , double depth , double pressure , double swat , double sgas);
  void                ecl_rft_cell_free( ecl_rft_cell_type * cell );
  void                ecl_rft_cell_free__( void * arg);

  bool                ecl_rft_cell_ijk_equal( const ecl_rft_cell_type * cell , int i , int j , int k);
  void                ecl_rft_cell_get_ijk( const ecl_rft_cell_type * cell , int * i , int * j , int * k);
  int                 ecl_rft_cell_get_i( const ecl_rft_cell_type * cell );
  int                 ecl_rft_cell_get_j( const ecl_rft_cell_type * cell );
  int                 ecl_rft_cell_get_k( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_depth( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_pressure( const ecl_rft_cell_type * cell );

  double              ecl_rft_cell_get_swat( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_sgas( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_soil( const ecl_rft_cell_type * cell );

  double              ecl_rft_cell_get_wrat( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_grat( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_orat( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_connection_start( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_connection_end( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_flowrate( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_oil_flowrate( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_gas_flowrate( const ecl_rft_cell_type * cell );
  double              ecl_rft_cell_get_water_flowrate( const ecl_rft_cell_type * cell );

  int                 ecl_rft_cell_cmp__( const void * arg1 , const void * arg2);
  int                 ecl_rft_cell_cmp( const ecl_rft_cell_type * cell1 , const ecl_rft_cell_type * cell2);
  bool                ecl_rft_cell_lt( const ecl_rft_cell_type * cell1 , const ecl_rft_cell_type * cell2);
#ifdef __cplusplus
}
#endif

#endif

