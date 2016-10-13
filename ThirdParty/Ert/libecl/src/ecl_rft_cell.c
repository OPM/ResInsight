/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_rft_cell.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_rft_cell.h>


#define ECL_RFT_CELL_TYPE_ID 99164012
#define RFT_DATA_TYPE_ID     66787166
#define PLT_DATA_TYPE_ID     87166667



struct ecl_rft_cell_struct {
  UTIL_TYPE_ID_DECLARATION;
  int i,j,k;
  double pressure;
  double depth;
  
  void * data;
};


typedef struct plt_data_struct plt_data_type;
typedef struct rft_data_struct rft_data_type;


struct rft_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  double swat;
  double sgas;
};




struct plt_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  double  orat;
  double  wrat;
  double  grat;
  double  connection_start;
  double  connection_end; 
  double  flowrate;
  double  oil_flowrate;
  double  gas_flowrate;
  double  water_flowrate;
};


/*****************************************************************/

static rft_data_type * rft_data_alloc( double swat , double sgas) {
  rft_data_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , RFT_DATA_TYPE_ID );

  data->swat = swat;
  data->sgas = sgas;
  
  return data;
}


static void rft_data_free( rft_data_type * data ) {
  free( data );
}

static UTIL_TRY_CAST_FUNCTION_CONST( rft_data , RFT_DATA_TYPE_ID)
static UTIL_IS_INSTANCE_FUNCTION( rft_data , RFT_DATA_TYPE_ID)

/*****************************************************************/

     static plt_data_type * plt_data_alloc( double orat , double grat , double wrat,double connection_start, double connection_end, double flowrate , double oil_flowrate , double gas_flowrate , double water_flowrate) {
  plt_data_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , PLT_DATA_TYPE_ID );

  data->orat = orat;
  data->grat = grat;
  data->wrat = wrat;
  data->connection_start = connection_start;
  data->connection_end = connection_end;
  data->flowrate       = flowrate;
  data->oil_flowrate   = oil_flowrate;
  data->gas_flowrate   = gas_flowrate; 
  data->water_flowrate = water_flowrate;
  
  return data;
}


static void plt_data_free( plt_data_type * data ) {
  free( data );
}

static UTIL_TRY_CAST_FUNCTION_CONST( plt_data , PLT_DATA_TYPE_ID)
static UTIL_IS_INSTANCE_FUNCTION( plt_data , PLT_DATA_TYPE_ID)



/*****************************************************************/

static UTIL_SAFE_CAST_FUNCTION( ecl_rft_cell , ECL_RFT_CELL_TYPE_ID)
static UTIL_SAFE_CAST_FUNCTION_CONST( ecl_rft_cell , ECL_RFT_CELL_TYPE_ID)
UTIL_IS_INSTANCE_FUNCTION( ecl_rft_cell , ECL_RFT_CELL_TYPE_ID)



static ecl_rft_cell_type * ecl_rft_cell_alloc_common(int i , int j , int k , double depth , double pressure) {
  ecl_rft_cell_type * cell = util_malloc( sizeof * cell );
  UTIL_TYPE_ID_INIT( cell , ECL_RFT_CELL_TYPE_ID );

  cell->i = i;
  cell->j = j;
  cell->k = k;
  cell->depth = depth;
  cell->pressure = pressure;
  
  return cell;
}



ecl_rft_cell_type * ecl_rft_cell_alloc_RFT( int i , int j , int k , double depth , double pressure , double swat , double sgas) {
  ecl_rft_cell_type * cell = ecl_rft_cell_alloc_common( i , j , k , depth , pressure );

  cell->data = rft_data_alloc( swat , sgas );
  return cell;
}


ecl_rft_cell_type * ecl_rft_cell_alloc_PLT( int i , 
                                            int j , 
                                            int k , 
                                            double depth , 
                                            double pressure , 
                                            double orat , 
                                            double grat , 
                                            double wrat, 
                                            double connection_start, 
                                            double connection_end,
                                            double flowrate ,
                                            double oil_flowrate , 
                                            double gas_flowrate , 
                                            double water_flowrate) {

  ecl_rft_cell_type * cell = ecl_rft_cell_alloc_common( i , j , k , depth , pressure );

  cell->data = plt_data_alloc( orat , grat , wrat  , connection_start , connection_end , flowrate , oil_flowrate , gas_flowrate , water_flowrate);
  return cell;
}



void ecl_rft_cell_free( ecl_rft_cell_type * cell ) {
  if (rft_data_is_instance( cell->data ))
    rft_data_free( cell->data );
  else if (plt_data_is_instance( cell->data ))
    plt_data_free( cell->data );

  free( cell );
}

void ecl_rft_cell_free__( void * arg) {
   ecl_rft_cell_type * cell = ecl_rft_cell_safe_cast( arg );
   ecl_rft_cell_free( cell );
}


/*****************************************************************/

int ecl_rft_cell_get_i( const ecl_rft_cell_type * cell ) {
   return cell->i;
}

int ecl_rft_cell_get_j( const ecl_rft_cell_type * cell ) {
   return cell->j;
}

int ecl_rft_cell_get_k( const ecl_rft_cell_type * cell ) {
   return cell->k;
}

void ecl_rft_cell_get_ijk( const ecl_rft_cell_type * cell , int * i , int * j , int * k) {
  *i = cell->i;
  *j = cell->j;
  *k = cell->k;
}

double ecl_rft_cell_get_depth( const ecl_rft_cell_type * cell ) {
   return cell->depth;
}

double ecl_rft_cell_get_pressure( const ecl_rft_cell_type * cell ) {
   return cell->pressure;
}


/*****************************************************************/

double ecl_rft_cell_get_swat( const ecl_rft_cell_type * cell ) {
   const rft_data_type * data = rft_data_try_cast_const( cell->data );
   if (data) 
     return data->swat;
   else
     return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_sgas( const ecl_rft_cell_type * cell ) {
   const rft_data_type * data = rft_data_try_cast_const( cell->data );
   if (data) 
     return data->sgas;
   else
     return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_soil( const ecl_rft_cell_type * cell ) {
   const rft_data_type * data = rft_data_try_cast_const( cell->data );
   if (data) 
     return 1 - (data->swat + data->sgas);
   else
     return ECL_RFT_CELL_INVALID_VALUE; 
}

/*****************************************************************/

double ecl_rft_cell_get_orat( const ecl_rft_cell_type * cell ) {
   const plt_data_type * data = plt_data_try_cast_const( cell->data );
   if (data) 
     return data->orat;
   else
     return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_grat( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->grat;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_wrat( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->wrat;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}

double ecl_rft_cell_get_connection_start( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->connection_start;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}

double ecl_rft_cell_get_connection_end( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->connection_end;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}

double ecl_rft_cell_get_flowrate( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->flowrate;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_oil_flowrate( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->oil_flowrate;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_gas_flowrate( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->gas_flowrate;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}


double ecl_rft_cell_get_water_flowrate( const ecl_rft_cell_type * cell ) {
  const plt_data_type * data = plt_data_try_cast_const( cell->data );
  if (data) 
    return data->water_flowrate;
  else
    return ECL_RFT_CELL_INVALID_VALUE; 
}





/*****************************************************************/



bool ecl_rft_cell_ijk_equal( const ecl_rft_cell_type * cell , int i , int j , int k) {
    return ( (i == cell->i) && 
             (j == cell->j) && 
             (k == cell->k) );
}


/*
  Currently only comparison based on connection length along PLT is supported. 
*/
int ecl_rft_cell_cmp( const ecl_rft_cell_type * cell1 , const ecl_rft_cell_type * cell2) {
  double val1 = ecl_rft_cell_get_connection_start( cell1 );
  double val2 = ecl_rft_cell_get_connection_start( cell2 );

  if (val1 < val2)
    return -1;
  else if (val1 == val2)
    return 0;
  else
    return 1;
    
}


int ecl_rft_cell_cmp__( const void * arg1 , const void * arg2) {
  const ecl_rft_cell_type * cell1 = ecl_rft_cell_safe_cast_const( arg1 );
  const ecl_rft_cell_type * cell2 = ecl_rft_cell_safe_cast_const( arg2 );
  return ecl_rft_cell_cmp( cell1 , cell2 );
}
