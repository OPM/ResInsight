/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_rft_node.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/hash.hpp>
#include <ert/util/vector.hpp>
#include <ert/util/int_vector.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_file_view.hpp>
#include <ert/ecl/ecl_rft_node.hpp>
#include <ert/ecl/ecl_rft_cell.hpp>
#include <ert/ecl/ecl_type.hpp>


/**
    The RFT's from several wells, and possibly also several timesteps
    are lumped togeheter in one .RFT file. The ecl_rft_node
    implemented in this file contains the information for one
    well/report step.
*/



/*
  If the type is RFT, PLT or SEGMENT depends on the options used when
  the .RFT file is created. RFT and PLT are quite similar, SEGMENT is
  not really supported.
*/



#define ECL_RFT_NODE_ID 887195
struct ecl_rft_node_struct {
  UTIL_TYPE_ID_DECLARATION;
  char       * well_name;              /* Name of the well. */

  ecl_rft_enum data_type;              /* What type of data: RFT|PLT|SEGMENT */
  time_t       recording_date;         /* When was the RFT recorded - date.*/
  double       days;                   /* When was the RFT recorded - days after simulaton start. */
  bool         MSW;

  bool              sort_perm_in_sync            ;
  int_vector_type * sort_perm;
  vector_type *cells;
};



/*
  The implementation of cell types based on _either_ RFT data or PLT
  data is based on a misunderstanding and is currently WRONG. One
  section in an RFT file can contain RFT data, PLT data and SEGMENT
  data. The @data_type string should therefore not be interpreted as a
  type string, but rather as a "bit mask":


    "R"  => Section contains only RFT data.
    "P"  => Section contains only PLT data.
    "RP" => Section contains *BOTH* RFT data and PLT data.
*/


/**
   Will return NULL if the data_type_string is equal to "SEGMENT" -
   that is not (yet) supported.
*/
static ecl_rft_enum translate_from_sting_to_ecl_rft_enum(const char * data_type_string){
    ecl_rft_enum data_type = SEGMENT;
    /* According to the ECLIPSE documentaton. */
    if (strchr(data_type_string , 'P') != NULL)
        data_type = PLT;
    else if (strchr(data_type_string, 'R') != NULL)
        data_type = RFT;
    else if (strchr(data_type_string , 'S') != NULL)
        data_type = SEGMENT;
    else
        util_abort("%s: Could not determine type of RFT/PLT/SEGMENT data - aborting\n",__func__);

    return data_type;
}

ecl_rft_node_type * ecl_rft_node_alloc_new(const char * well_name, const char * data_type_string, const time_t recording_date, const double days){
    ecl_rft_enum data_type = translate_from_sting_to_ecl_rft_enum(data_type_string);
    ecl_rft_node_type * rft_node = (ecl_rft_node_type*)util_malloc(sizeof * rft_node );
    UTIL_TYPE_ID_INIT( rft_node , ECL_RFT_NODE_ID );
    rft_node->well_name = util_alloc_string_copy(well_name);
    rft_node->cells = vector_alloc_new();
    rft_node->recording_date = recording_date;
    rft_node->days = days;
    rft_node->data_type = data_type;
    rft_node->sort_perm = NULL;
    rft_node->sort_perm_in_sync = false;

    return rft_node;
}


static ecl_rft_node_type * ecl_rft_node_alloc_empty(const char * data_type_string) {
  ecl_rft_enum data_type = translate_from_sting_to_ecl_rft_enum(data_type_string);

  /* Can return NULL */
  if (data_type == SEGMENT) {
    fprintf(stderr,"%s: sorry SEGMENT PLT/RFT is not supported - file a complaint. \n",__func__);
    return NULL;
  }

  {
    ecl_rft_node_type * rft_node = (ecl_rft_node_type*)util_malloc(sizeof * rft_node );
    UTIL_TYPE_ID_INIT( rft_node , ECL_RFT_NODE_ID );

    rft_node->cells = vector_alloc_new();
    rft_node->data_type = data_type;
    rft_node->sort_perm = NULL;
    rft_node->sort_perm_in_sync = false;

    return rft_node;
  }
}


UTIL_SAFE_CAST_FUNCTION( ecl_rft_node   , ECL_RFT_NODE_ID );
UTIL_IS_INSTANCE_FUNCTION( ecl_rft_node , ECL_RFT_NODE_ID );


void ecl_rft_node_append_cell( ecl_rft_node_type * rft_node , ecl_rft_cell_type * cell) {
  vector_append_owned_ref( rft_node->cells , cell , ecl_rft_cell_free__ );
  rft_node->sort_perm_in_sync = false;
}


static ecl_kw_type * ecl_rft_node_get_pressure_kw( ecl_rft_node_type * rft_node , const ecl_file_view_type * rft ) {
  if (rft_node->data_type == RFT)
    return ecl_file_view_iget_named_kw( rft , PRESSURE_KW , 0);
  else {
    ecl_kw_type * conpres_kw = ecl_file_view_iget_named_kw( rft , CONPRES_KW , 0);
    if (ecl_kw_element_sum_float( conpres_kw ) > 0.0 )
      return conpres_kw;
    else
      if (ecl_file_view_has_kw(rft, PRESSURE_KW))
        return ecl_file_view_iget_named_kw( rft , PRESSURE_KW , 0);
      else {
        fprintf(stderr, "WARNING: %s returned a CONPRES_KW with all values at zero. PRESSURE_KW not found.\n", __func__);
        return conpres_kw;
      }
  }
}


static void ecl_rft_node_init_RFT_cells( ecl_rft_node_type * rft_node , const ecl_file_view_type * rft_view) {
  const ecl_kw_type * conipos     = ecl_file_view_iget_named_kw( rft_view , CONIPOS_KW , 0);
  const ecl_kw_type * conjpos     = ecl_file_view_iget_named_kw( rft_view , CONJPOS_KW , 0);
  const ecl_kw_type * conkpos     = ecl_file_view_iget_named_kw( rft_view , CONKPOS_KW , 0);
  const ecl_kw_type * depth_kw    = ecl_file_view_iget_named_kw( rft_view , DEPTH_KW , 0);
  const ecl_kw_type * swat_kw     = ecl_file_view_iget_named_kw( rft_view , SWAT_KW , 0);
  const ecl_kw_type * sgas_kw     = ecl_file_view_iget_named_kw( rft_view , SGAS_KW , 0);
  const ecl_kw_type * pressure_kw = ecl_rft_node_get_pressure_kw( rft_node , rft_view );

  const float * SW     = ecl_kw_get_float_ptr( swat_kw );
  const float * SG     = ecl_kw_get_float_ptr( sgas_kw );
  const float * P      = ecl_kw_get_float_ptr( pressure_kw );
  const float * depth  = ecl_kw_get_float_ptr( depth_kw );
  const int   * i      = ecl_kw_get_int_ptr( conipos );
  const int   * j      = ecl_kw_get_int_ptr( conjpos );
  const int   * k      = ecl_kw_get_int_ptr( conkpos );

  {
    int c;
    for (c = 0; c < ecl_kw_get_size( conipos ); c++) {
      /* The connection coordinates are shifted -= 1; i.e. all internal usage is offset 0. */
      ecl_rft_cell_type * cell = ecl_rft_cell_alloc_RFT( i[c] - 1 , j[c] - 1 , k[c] - 1 ,
                                                         depth[c] , P[c] , SW[c] , SG[c]);
      ecl_rft_node_append_cell( rft_node , cell );
    }
  }
}





static void ecl_rft_node_init_PLT_cells( ecl_rft_node_type * rft_node , const ecl_file_view_type * rft_view) {
  /* For PLT there is quite a lot of extra information which is not yet internalized. */
  const ecl_kw_type * conipos     = ecl_file_view_iget_named_kw( rft_view , CONIPOS_KW  , 0);
  const ecl_kw_type * conjpos     = ecl_file_view_iget_named_kw( rft_view , CONJPOS_KW  , 0);
  const ecl_kw_type * conkpos     = ecl_file_view_iget_named_kw( rft_view , CONKPOS_KW  , 0);

  const int   * i      = ecl_kw_get_int_ptr( conipos );
  const int   * j      = ecl_kw_get_int_ptr( conjpos );
  const int   * k      = ecl_kw_get_int_ptr( conkpos );

  const float * WR               = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONWRAT_KW , 0));
  const float * GR               = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONGRAT_KW , 0));
  const float * OR               = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONORAT_KW , 0));
  const float * P                = ecl_kw_get_float_ptr( ecl_rft_node_get_pressure_kw( rft_node , rft_view ));
  const float * depth            = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONDEPTH_KW , 0));
  const float * flowrate         = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONVTUB_KW , 0));
  const float * oil_flowrate     = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONOTUB_KW , 0));
  const float * gas_flowrate     = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONGTUB_KW , 0));
  const float * water_flowrate   = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONWTUB_KW , 0));
  const float * connection_start = NULL;
  const float * connection_end   = NULL;

  /* The keywords CONLENST_KW and CONLENEN_KW are ONLY present if we are dealing with a MSW well. */
  if (ecl_file_view_has_kw( rft_view , CONLENST_KW))
    connection_start = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONLENST_KW , 0));

  if (ecl_file_view_has_kw( rft_view , CONLENEN_KW))
    connection_end = ecl_kw_get_float_ptr( ecl_file_view_iget_named_kw( rft_view , CONLENEN_KW , 0));

  {
    int c;
    for ( c = 0; c < ecl_kw_get_size( conipos ); c++) {
      ecl_rft_cell_type * cell;
      double cs = 0;
      double ce = 0;

      if (connection_start)
        cs = connection_start[c];

      if (connection_end)
        ce = connection_end[c];

      /* The connection coordinates are shifted -= 1; i.e. all internal usage is offset 0. */
      cell = ecl_rft_cell_alloc_PLT( i[c] -1 , j[c] -1 , k[c] -1 ,
                                     depth[c] , P[c] , OR[c] , GR[c] , WR[c] , cs , ce,  flowrate[c] , oil_flowrate[c] , gas_flowrate[c] , water_flowrate[c]);
      ecl_rft_node_append_cell( rft_node , cell );
    }
  }
}





static void ecl_rft_node_init_cells( ecl_rft_node_type * rft_node , const ecl_file_view_type * rft_view ) {

  if (rft_node->data_type == RFT)
    ecl_rft_node_init_RFT_cells( rft_node , rft_view );
  else if (rft_node->data_type == PLT)
    ecl_rft_node_init_PLT_cells( rft_node , rft_view );

}


ecl_rft_node_type * ecl_rft_node_alloc(const ecl_file_view_type * rft_view) {
  ecl_kw_type       * welletc   = ecl_file_view_iget_named_kw(rft_view , WELLETC_KW , 0);
  ecl_rft_node_type * rft_node  = ecl_rft_node_alloc_empty((const char*)ecl_kw_iget_ptr(welletc , WELLETC_TYPE_INDEX));

  if (rft_node != NULL) {
    ecl_kw_type * date_kw = ecl_file_view_iget_named_kw( rft_view , DATE_KW    , 0);
    rft_node->well_name = (char*)util_alloc_strip_copy( (const char*)ecl_kw_iget_ptr(welletc , WELLETC_NAME_INDEX));

    /* Time information. */
    {
      int * time = ecl_kw_get_int_ptr( date_kw );
      rft_node->recording_date = ecl_util_make_date( time[DATE_DAY_INDEX] , time[DATE_MONTH_INDEX] , time[DATE_YEAR_INDEX] );
    }
    rft_node->days = ecl_kw_iget_float( ecl_file_view_iget_named_kw( rft_view , TIME_KW , 0 ) , 0);
    if (ecl_file_view_has_kw( rft_view , CONLENST_KW))
      rft_node->MSW = true;
    else
      rft_node->MSW = false;

    ecl_rft_node_init_cells( rft_node , rft_view );
  }
  return rft_node;
}


const char * ecl_rft_node_get_well_name(const ecl_rft_node_type * rft_node) {
  return rft_node->well_name;
}


void ecl_rft_node_free(ecl_rft_node_type * rft_node) {

  free(rft_node->well_name);
  vector_free( rft_node->cells );
  if (rft_node->sort_perm)
    int_vector_free( rft_node->sort_perm );

  free(rft_node);
}

void ecl_rft_node_free__(void * void_node) {
  ecl_rft_node_free( ecl_rft_node_safe_cast (void_node) );
}






int          ecl_rft_node_get_size(const ecl_rft_node_type * rft_node) { return vector_get_size( rft_node->cells ); }
time_t       ecl_rft_node_get_date(const ecl_rft_node_type * rft_node) { return rft_node->recording_date; }
ecl_rft_enum ecl_rft_node_get_type(const ecl_rft_node_type * rft_node) { return rft_node->data_type; }


/*****************************************************************/
/* various functions to access properties at the cell level      */

const ecl_rft_cell_type * ecl_rft_node_iget_cell( const ecl_rft_node_type * rft_node , int index) {
  return (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index );
}


static void ecl_rft_node_create_sort_perm( ecl_rft_node_type * rft_node ) {
  if (rft_node->sort_perm)
    int_vector_free( rft_node->sort_perm );

  rft_node->sort_perm = vector_alloc_sort_perm( rft_node->cells , ecl_rft_cell_cmp__ );
  rft_node->sort_perm_in_sync = true;
}

void ecl_rft_node_inplace_sort_cells( ecl_rft_node_type * rft_node ) {
  vector_sort( rft_node->cells , ecl_rft_cell_cmp__ );
  rft_node->sort_perm_in_sync = false;  // The permutation is no longer sorted; however the vector itself is sorted ....
}

const ecl_rft_cell_type * ecl_rft_node_iget_cell_sorted( ecl_rft_node_type * rft_node , int index) {
  if (ecl_rft_node_is_RFT( rft_node ))
    return ecl_rft_node_iget_cell( rft_node , index );
  else {
    if (!rft_node->sort_perm_in_sync)
      ecl_rft_node_create_sort_perm( rft_node );

    return (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , int_vector_iget( rft_node->sort_perm , index ));
  }
}



double ecl_rft_node_iget_depth( const ecl_rft_node_type * rft_node , int index) {
  const ecl_rft_cell_type * cell = ecl_rft_node_iget_cell( rft_node , index );
  return ecl_rft_cell_get_depth( cell );
}



double ecl_rft_node_iget_pressure( const ecl_rft_node_type * rft_node , int index) {
  const ecl_rft_cell_type * cell = ecl_rft_node_iget_cell( rft_node , index );
  return ecl_rft_cell_get_pressure( cell );
}


void ecl_rft_node_iget_ijk( const ecl_rft_node_type * rft_node , int index , int *i , int *j , int *k) {
  const ecl_rft_cell_type * cell = ecl_rft_node_iget_cell( rft_node , index );

  ecl_rft_cell_get_ijk( cell , i,j,k);
}


const ecl_rft_cell_type * ecl_rft_node_lookup_ijk( const ecl_rft_node_type * rft_node , int i, int j , int k) {
  int index = 0;
  int size = ecl_rft_node_get_size( rft_node );
  while (true) {
    const ecl_rft_cell_type * cell = ecl_rft_node_iget_cell( rft_node , index );

    if (ecl_rft_cell_ijk_equal( cell , i , j , k ))
      return cell;

    index++;
    if (index == size)                             /* Could not find it. */
      return NULL;
  }
}



static void assert_type_and_index( const ecl_rft_node_type * rft_node , ecl_rft_enum target_type , int index) {
  if (rft_node->data_type != target_type)
    util_abort("%s: wrong type \n",__func__);

  if ((index < 0) || (index >= vector_get_size( rft_node->cells )))
    util_abort("%s: invalid index:%d \n",__func__ , index);
}

double ecl_rft_node_iget_sgas( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , RFT , index );
  {
    const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_sgas( cell );
  }
}


double ecl_rft_node_iget_swat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , RFT , index );
  {
    const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_swat( cell );
  }
}

double ecl_rft_node_get_days(const ecl_rft_node_type * rft_node ){
  return rft_node->days;
}

double ecl_rft_node_iget_soil( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , RFT , index );
  {
    const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_soil( cell );
  }
}

/*****************************************************************/


double ecl_rft_node_iget_orat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , PLT , index );
  {
    const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_orat( cell );
  }
}


double ecl_rft_node_iget_wrat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , PLT , index );
  {
    const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index);
    return ecl_rft_cell_get_wrat( cell );
  }
}


double ecl_rft_node_iget_grat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , PLT , index );
  {
    const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , index);
    return ecl_rft_cell_get_grat( cell );
  }
}


bool ecl_rft_node_is_MSW( const ecl_rft_node_type * rft_node ) {
  return rft_node->MSW;
}


bool ecl_rft_node_is_PLT( const ecl_rft_node_type * rft_node ) {
  if (rft_node->data_type == PLT)
    return true;
  else
    return false;
}

bool ecl_rft_node_is_SEGMENT( const ecl_rft_node_type * rft_node ) {
  if (rft_node->data_type == SEGMENT)
    return true;
  else
    return false;
}

bool ecl_rft_node_is_RFT( const ecl_rft_node_type * rft_node ) {
  if (rft_node->data_type == RFT)
    return true;
  else
    return false;
}

static void ecl_rft_node_fill_welletc(ecl_kw_type * welletc, ert_ecl_unit_enum unit_set){
    if(unit_set==ECL_METRIC_UNITS) {
        ecl_kw_iset_string8(welletc, 0, "  DAYS");
        ecl_kw_iset_string8(welletc, 2, "");
        ecl_kw_iset_string8(welletc, 3, " METRES");
        ecl_kw_iset_string8(welletc, 4, "  BARSA");
        ecl_kw_iset_string8(welletc, 6, "STANDARD");
        ecl_kw_iset_string8(welletc, 7, " SM3/DAY");
        ecl_kw_iset_string8(welletc, 8, " SM3/DAY");
        ecl_kw_iset_string8(welletc, 9, " RM3/DAY");
        ecl_kw_iset_string8(welletc, 10, " M/SEC");
        ecl_kw_iset_string8(welletc, 11, "");
        ecl_kw_iset_string8(welletc, 12, "   CP");
        ecl_kw_iset_string8(welletc, 13, " KG/SM3");
        ecl_kw_iset_string8(welletc, 14, " KG/DAY");
        ecl_kw_iset_string8(welletc, 15, "  KG/KG");
    }else if(unit_set==ECL_FIELD_UNITS){
        ecl_kw_iset_string8(welletc, 0, "  DAYS");
        ecl_kw_iset_string8(welletc, 2, "");
        ecl_kw_iset_string8(welletc, 3, "  FEET");
        ecl_kw_iset_string8(welletc, 4, "  PISA");
        ecl_kw_iset_string8(welletc, 6, "STANDARD");
        ecl_kw_iset_string8(welletc, 7, " STB/DAY");
        ecl_kw_iset_string8(welletc, 8, " MSCF/DAY");
        ecl_kw_iset_string8(welletc, 9, " RB/DAY");
        ecl_kw_iset_string8(welletc, 10, " FT/SEC");
        ecl_kw_iset_string8(welletc, 11, "");
        ecl_kw_iset_string8(welletc, 12, "   CP");
        ecl_kw_iset_string8(welletc, 13, " LB/STB");
        ecl_kw_iset_string8(welletc, 14, " LB/DAY");
        ecl_kw_iset_string8(welletc, 15, "  LB/LB");

    }else if(unit_set==ECL_LAB_UNITS){
        ecl_kw_iset_string8(welletc, 0, "   HR");
        ecl_kw_iset_string8(welletc, 2, "");
        ecl_kw_iset_string8(welletc, 3, "   CM");
        ecl_kw_iset_string8(welletc, 4, "  ATMA");
        ecl_kw_iset_string8(welletc, 6, "STANDARD");
        ecl_kw_iset_string8(welletc, 7, " SCC/HR");
        ecl_kw_iset_string8(welletc, 8, " SCC/HR");
        ecl_kw_iset_string8(welletc, 9, " RCC/SCC");
        ecl_kw_iset_string8(welletc, 10, " CM/SEC");
        ecl_kw_iset_string8(welletc, 11, "");
        ecl_kw_iset_string8(welletc, 12, "   CP");
        ecl_kw_iset_string8(welletc, 13, " GM/SCC");
        ecl_kw_iset_string8(welletc, 14, " GH/HR");
        ecl_kw_iset_string8(welletc, 15, "  GM/GM");
    }


}

void ecl_rft_node_fwrite(const ecl_rft_node_type * rft_node, fortio_type * fortio, ert_ecl_unit_enum unit_set){
  ecl_rft_enum type = ecl_rft_node_get_type(rft_node);
  if (type != RFT)
    util_abort("%s: sorry - only writing of simple RFT is currently implemented",__func__);

  {
    ecl_kw_type * time = ecl_kw_alloc(TIME_KW, 1, ECL_FLOAT);
    ecl_kw_iset_float(time, 0, ecl_rft_node_get_days(rft_node));
    ecl_kw_fwrite(time, fortio);
    ecl_kw_free(time);
  }

  {
    ecl_kw_type * datevalue = ecl_kw_alloc(DATE_KW, 3, ECL_INT);
    time_t date = ecl_rft_node_get_date(rft_node);
    int day;
    int month;
    int year;
    ecl_util_set_date_values(date , &day , &month , &year);
    ecl_kw_iset_int(datevalue, 0, day);
    ecl_kw_iset_int(datevalue, 1, month);
    ecl_kw_iset_int(datevalue, 2, year);
    ecl_kw_fwrite(datevalue, fortio);
    ecl_kw_free(datevalue);
  }

  {
    ecl_kw_type * welletc = ecl_kw_alloc(WELLETC_KW, 16, ECL_CHAR);
    ecl_rft_enum type = ecl_rft_node_get_type(rft_node);

    ecl_kw_iset_string8(welletc, 1, ecl_rft_node_get_well_name(rft_node));

    if(type == PLT) {
      ecl_kw_iset_string8(welletc, 5, "P");
    }else if(type == RFT){
      ecl_kw_iset_string8(welletc, 5, "R");
    }else if(type == SEGMENT){
      ecl_kw_iset_string8(welletc, 5, "S");
    }
    ecl_rft_node_fill_welletc(welletc, unit_set);
    ecl_kw_fwrite(welletc, fortio);
    ecl_kw_free(welletc);
  }

  {
    int size_cells = ecl_rft_node_get_size(rft_node);
    ecl_kw_type * conipos = ecl_kw_alloc(CONIPOS_KW, size_cells, ECL_INT);
    ecl_kw_type * conjpos = ecl_kw_alloc(CONJPOS_KW, size_cells, ECL_INT);
    ecl_kw_type * conkpos = ecl_kw_alloc(CONKPOS_KW, size_cells, ECL_INT);
    ecl_kw_type * hostgrid = ecl_kw_alloc(HOSTGRID_KW, size_cells, ECL_CHAR);
    ecl_kw_type * depth = ecl_kw_alloc(DEPTH_KW, size_cells, ECL_FLOAT);
    ecl_kw_type * pressure = ecl_kw_alloc(PRESSURE_KW, size_cells, ECL_FLOAT);
    ecl_kw_type * swat = ecl_kw_alloc(SWAT_KW, size_cells, ECL_FLOAT);
    ecl_kw_type * sgas = ecl_kw_alloc(SGAS_KW, size_cells, ECL_FLOAT);
    int i;

    for(i =0;i<size_cells;i++){
      const ecl_rft_cell_type * cell = (const ecl_rft_cell_type*)vector_iget_const( rft_node->cells , i);
      ecl_kw_iset_int(conipos, i, ecl_rft_cell_get_i(cell)+1);
      ecl_kw_iset_int(conjpos, i, ecl_rft_cell_get_j(cell)+1);
      ecl_kw_iset_int(conkpos, i, ecl_rft_cell_get_k(cell)+1);
      ecl_kw_iset_float(depth, i, ecl_rft_cell_get_depth(cell));
      ecl_kw_iset_float(pressure, i, ecl_rft_cell_get_pressure(cell));
      ecl_kw_iset_float(swat, i, ecl_rft_cell_get_swat(cell));
      ecl_kw_iset_float(sgas, i, ecl_rft_cell_get_sgas(cell));
    }
    ecl_kw_fwrite(conipos, fortio);
    ecl_kw_fwrite(conjpos, fortio);
    ecl_kw_fwrite(conkpos, fortio);
    ecl_kw_fwrite(hostgrid, fortio);
    ecl_kw_fwrite(depth, fortio);
    ecl_kw_fwrite(pressure, fortio);
    ecl_kw_fwrite(swat, fortio);
    ecl_kw_fwrite(sgas, fortio);

    ecl_kw_free(conipos);
    ecl_kw_free(conjpos);
    ecl_kw_free(conkpos);
    ecl_kw_free(hostgrid);
    ecl_kw_free(depth);
    ecl_kw_free(pressure);
    ecl_kw_free(swat);
    ecl_kw_free(sgas);
  }
}

int ecl_rft_node_cmp( const ecl_rft_node_type * n1 , const ecl_rft_node_type * n2) {
    time_t val1 = ecl_rft_node_get_date(n1);
    time_t val2 = ecl_rft_node_get_date(n2);

    if (val1 < val2)
        return -1;
    else if (val1 == val2)
        return 0;
    else
        return 1;

}


