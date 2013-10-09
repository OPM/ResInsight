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
#include <ert/util/hash.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_rft_node.h>
#include <ert/ecl/ecl_rft_cell.h>

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



/**
   Will return NULL if the data_type_string is equal to "SEGMENT" -
   that is not (yet) supported.
*/

static ecl_rft_node_type * ecl_rft_node_alloc_empty(const char * data_type_string) {
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
  

  /* Can return NULL */
  if (data_type == SEGMENT) {
    fprintf(stderr,"%s: sorry SEGMENT PLT/RFT is not supported - file a complaint. \n",__func__);
    return NULL;
  }

  {
    ecl_rft_node_type * rft_node = util_malloc(sizeof * rft_node );
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


static void ecl_rft_node_append_cell( ecl_rft_node_type * rft_node , ecl_rft_cell_type * cell) {
  vector_append_owned_ref( rft_node->cells , cell , ecl_rft_cell_free__ );
  rft_node->sort_perm_in_sync = false;
}



static void ecl_rft_node_init_RFT_cells( ecl_rft_node_type * rft_node , const ecl_file_type * rft) {
  const ecl_kw_type * conipos     = ecl_file_iget_named_kw( rft , CONIPOS_KW , 0);
  const ecl_kw_type * conjpos     = ecl_file_iget_named_kw( rft , CONJPOS_KW , 0);
  const ecl_kw_type * conkpos     = ecl_file_iget_named_kw( rft , CONKPOS_KW , 0);
  const ecl_kw_type * depth_kw    = ecl_file_iget_named_kw( rft , DEPTH_KW , 0);
  const ecl_kw_type * swat_kw     = ecl_file_iget_named_kw( rft , SWAT_KW , 0);
  const ecl_kw_type * sgas_kw     = ecl_file_iget_named_kw( rft , SGAS_KW , 0);
  const ecl_kw_type * pressure_kw = ecl_file_iget_named_kw( rft , PRESSURE_KW , 0);

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
  



                                            
static void ecl_rft_node_init_PLT_cells( ecl_rft_node_type * rft_node , const ecl_file_type * rft) {
  /* For PLT there is quite a lot of extra information which is not yet internalized. */
  const ecl_kw_type * conipos     = ecl_file_iget_named_kw( rft , CONIPOS_KW  , 0);
  const ecl_kw_type * conjpos     = ecl_file_iget_named_kw( rft , CONJPOS_KW  , 0);
  const ecl_kw_type * conkpos     = ecl_file_iget_named_kw( rft , CONKPOS_KW  , 0);  

  const int   * i      = ecl_kw_get_int_ptr( conipos );
  const int   * j      = ecl_kw_get_int_ptr( conjpos );
  const int   * k      = ecl_kw_get_int_ptr( conkpos );
  
  const float * WR               = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONWRAT_KW , 0));
  const float * GR               = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONGRAT_KW , 0)); 
  const float * OR               = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONORAT_KW , 0)); 
  const float * P                = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONPRES_KW , 0));
  const float * depth            = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONDEPTH_KW , 0));
  const float * flowrate         = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONVTUB_KW , 0));
  const float * oil_flowrate     = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONOTUB_KW , 0));
  const float * gas_flowrate     = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONGTUB_KW , 0));
  const float * water_flowrate   = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONWTUB_KW , 0));
  const float * connection_start = NULL;
  const float * connection_end   = NULL; 

  /* The keywords CONLENST_KW and CONLENEN_KW are ONLY present if we are dealing with a MSW well. */
  if (ecl_file_has_kw( rft , CONLENST_KW))
    connection_start = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONLENST_KW , 0));
  
  if (ecl_file_has_kw( rft , CONLENEN_KW))
    connection_end = ecl_kw_get_float_ptr( ecl_file_iget_named_kw( rft , CONLENEN_KW , 0));

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





static void ecl_rft_node_init_cells( ecl_rft_node_type * rft_node , const ecl_file_type * rft ) {

  if (rft_node->data_type == RFT)
    ecl_rft_node_init_RFT_cells( rft_node , rft );
  else if (rft_node->data_type == PLT)
    ecl_rft_node_init_PLT_cells( rft_node , rft );
  
}


ecl_rft_node_type * ecl_rft_node_alloc(const ecl_file_type * rft) {
  ecl_kw_type       * welletc   = ecl_file_iget_named_kw(rft , WELLETC_KW , 0);
  ecl_rft_node_type * rft_node  = ecl_rft_node_alloc_empty(ecl_kw_iget_ptr(welletc , WELLETC_TYPE_INDEX));
  
  if (rft_node != NULL) {
    ecl_kw_type * date_kw = ecl_file_iget_named_kw( rft , DATE_KW    , 0);
    rft_node->well_name = util_alloc_strip_copy( ecl_kw_iget_ptr(welletc , WELLETC_NAME_INDEX));
    
    /* Time information. */
    {
      int * time = ecl_kw_get_int_ptr( date_kw );
      rft_node->recording_date = ecl_util_make_date( time[DATE_DAY_INDEX] , time[DATE_MONTH_INDEX] , time[DATE_YEAR_INDEX] );
    }
    rft_node->days = ecl_kw_iget_float( ecl_file_iget_named_kw( rft , TIME_KW , 0 ) , 0);
    if (ecl_file_has_kw( rft , CONLENST_KW))
      rft_node->MSW = true;
    else
      rft_node->MSW = false;

    ecl_rft_node_init_cells( rft_node , rft );
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
  return vector_iget_const( rft_node->cells , index );
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
    
    return vector_iget_const( rft_node->cells , int_vector_iget( rft_node->sort_perm , index ));
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
    const ecl_rft_cell_type * cell = vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_sgas( cell );
  }
}


double ecl_rft_node_iget_swat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , RFT , index );
  {
    const ecl_rft_cell_type * cell = vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_swat( cell );
  }
}


double ecl_rft_node_iget_soil( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , RFT , index );
  {
    const ecl_rft_cell_type * cell = vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_soil( cell );
  }
}

/*****************************************************************/


double ecl_rft_node_iget_orat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , PLT , index );
  {
    const ecl_rft_cell_type * cell = vector_iget_const( rft_node->cells , index );
    return ecl_rft_cell_get_orat( cell );
  }
}


double ecl_rft_node_iget_wrat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , PLT , index );
  {
    const ecl_rft_cell_type * cell = vector_iget_const( rft_node->cells , index);
    return ecl_rft_cell_get_wrat( cell );
  }
}


double ecl_rft_node_iget_grat( const ecl_rft_node_type * rft_node , int index) {
  assert_type_and_index( rft_node , PLT , index );
  {
    const ecl_rft_cell_type * cell = vector_iget_const( rft_node->cells , index);
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


