/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   
   The file 'well_segment.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>

#include <ert/util/util.hpp>
#include <ert/util/hash.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_rsthead.hpp>
#include <ert/ecl/ecl_grid.hpp>

#include <ert/ecl_well/well_const.hpp>
#include <ert/ecl_well/well_conn.hpp>
#include <ert/ecl_well/well_segment.hpp>
#include <ert/ecl_well/well_conn_collection.hpp>

#define WELL_SEGMENT_TYPE_ID  2209166 

struct well_segment_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                 link_count;
  int                 segment_id;  
  int                 branch_id; 
  int                 outlet_segment_id;  // This is in the global index space given by the ISEG keyword.
  well_segment_type * outlet_segment;
  hash_type         * connections;        // hash_type<grid_name , well_conn_collection>;

  double              depth;              // The depth of the segment node; furthest away from the wellhead.
  double              length;
  double              total_length;       // Total length from wellhead.
  double              diameter;           // The tube diametere available for flow.
};


UTIL_IS_INSTANCE_FUNCTION( well_segment , WELL_SEGMENT_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( well_segment , WELL_SEGMENT_TYPE_ID )


well_segment_type * well_segment_alloc(int segment_id , int outlet_segment_id , int branch_id , const double * rseg_data) {
  well_segment_type * segment = (well_segment_type*)util_malloc( sizeof * segment );
  UTIL_TYPE_ID_INIT( segment , WELL_SEGMENT_TYPE_ID );
  
  segment->link_count = 0;
  segment->segment_id = segment_id;
  segment->outlet_segment_id = outlet_segment_id;
  segment->branch_id = branch_id;
  segment->outlet_segment = NULL;
  segment->connections = hash_alloc();

  segment->depth = 0.0;
  segment->length = 0.0;
  segment->total_length = 0.0;
  segment->diameter = 0.0;

  if(rseg_data != NULL) {
    segment->depth = rseg_data[ RSEG_DEPTH_INDEX ];
    segment->length = rseg_data[ RSEG_LENGTH_INDEX ];
    segment->total_length = rseg_data[ RSEG_TOTAL_LENGTH_INDEX ];
    segment->diameter = rseg_data[ RSEG_DIAMETER_INDEX ];
  }
  
  return segment;
}


well_segment_type * well_segment_alloc_from_kw( const ecl_kw_type * iseg_kw , const well_rseg_loader_type * rseg_loader , const ecl_rsthead_type * header , int well_nr, int segment_index , int segment_id) {
  if (rseg_loader == NULL) {
    util_abort("%s: fatal internal error - tried to create well_segment instance without RSEG keyword.\n",__func__);
    return NULL;
  } else {
    const int iseg_offset = header->nisegz * ( header->nsegmx * well_nr + segment_index);
    const int rseg_offset = header->nrsegz * ( header->nsegmx * well_nr + segment_index);
    int outlet_segment_id = ecl_kw_iget_int( iseg_kw , iseg_offset + ISEG_OUTLET_INDEX ) - ECLIPSE_WELL_SEGMENT_OFFSET + WELL_SEGMENT_OFFSET ;   // -1
    int branch_id         = ecl_kw_iget_int( iseg_kw , iseg_offset + ISEG_BRANCH_INDEX ) - ECLIPSE_WELL_BRANCH_OFFSET  + WELL_BRANCH_OFFSET ;    // -1
    const double * rseg_data = well_rseg_loader_load_values(rseg_loader, rseg_offset);

    well_segment_type * segment = well_segment_alloc( segment_id , outlet_segment_id , branch_id , NULL);
    segment->depth = rseg_data[0];
    segment->length = rseg_data[1];
    segment->total_length = rseg_data[2];
    segment->diameter = rseg_data[3];
    return segment;
  }
}


/*
    if (iseg_kw != NULL) {
      if (conn->segment != WELL_CONN_NORMAL_WELL_SEGMENT_ID) {
  
      } else {
        conn->branch = 0;
        conn->outlet_segment = 0;
      }
    } else {
      conn->branch = 0;
      conn->outlet_segment = 0;
    }
    */


void well_segment_free(well_segment_type * segment ) {
  hash_free( segment->connections );
  free( segment );
}

void well_segment_free__(void * arg) {
  well_segment_type * segment = well_segment_safe_cast( arg );
  well_segment_free( segment );
}


bool well_segment_active( const well_segment_type * segment ) {
  if (segment->branch_id == WELL_SEGMENT_BRANCH_INACTIVE_VALUE)
    return false;
  else
    return true;
}


bool well_segment_main_stem( const well_segment_type * segment ) {
  if (segment->branch_id == WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE)
    return true;
  else
    return false;
}


bool well_segment_nearest_wellhead( const well_segment_type * segment ) {
  if (segment->outlet_segment_id == WELL_SEGMENT_OUTLET_END_VALUE)
    return true;
  else
    return false;
}
  

int well_segment_get_link_count( const well_segment_type * segment ) {
  return segment->link_count;
}

int well_segment_get_branch_id( const well_segment_type * segment ) {
  return segment->branch_id;
}

int well_segment_get_outlet_id( const well_segment_type * segment ) {
  return segment->outlet_segment_id;
}

int well_segment_get_id( const well_segment_type * segment ) {
  return segment->segment_id;
}


well_segment_type * well_segment_get_outlet( const well_segment_type * segment ) {
  return segment->outlet_segment;
}
  

bool well_segment_link( well_segment_type * segment , well_segment_type * outlet_segment ) {
  if (segment->outlet_segment_id == outlet_segment->segment_id) {
    segment->outlet_segment = outlet_segment;
    if (outlet_segment->branch_id == segment->branch_id){
      outlet_segment->link_count++;
    }
    return true;
  } else 
    /* 
       This is a quite fatal topological error - and aborting is probaly the wisest
       thing to do. I.e.  the function well_segment_link_strict() is recommended.
    */
    return false;
}


void well_segment_link_strict( well_segment_type * segment , well_segment_type * outlet_segment ) {
  if (!well_segment_link( segment , outlet_segment))
    util_abort("%s: tried to create invalid link between segments %d and %d \n",segment->segment_id , outlet_segment->segment_id);
}



bool well_segment_has_grid_connections( const well_segment_type * segment , const char * grid_name) {
  return hash_has_key( segment->connections , grid_name );
}


bool well_segment_has_global_grid_connections( const well_segment_type * segment) {
  return well_segment_has_grid_connections( segment , ECL_GRID_GLOBAL_GRID );
}


bool well_segment_add_connection( well_segment_type * segment , const char * grid_name , well_conn_type * conn) {
  int conn_segment_id = well_conn_get_segment_id( conn );
  if (conn_segment_id == segment->segment_id) {
    if (!well_segment_has_grid_connections( segment , grid_name ))
      hash_insert_hash_owned_ref( segment->connections , grid_name , well_conn_collection_alloc() , well_conn_collection_free__ );
    
    {
      well_conn_collection_type * connections = (well_conn_collection_type*)hash_get( segment->connections , grid_name );
      well_conn_collection_add_ref( connections , conn );
    }
    return true;
  } else
    return false;  
}


const well_conn_collection_type * well_segment_get_connections(const well_segment_type * segment , const char * grid_name ) {
  if (well_segment_has_grid_connections( segment , grid_name))
    return (const well_conn_collection_type*)hash_get( segment->connections , grid_name);
  else
    return NULL;
}


const well_conn_collection_type * well_segment_get_global_connections(const well_segment_type * segment ) {
  return well_segment_get_connections( segment , ECL_GRID_GLOBAL_GRID );
}


bool well_segment_well_is_MSW(int well_nr , const ecl_kw_type * iwel_kw , const ecl_rsthead_type * rst_head) {
  int iwel_offset = rst_head->niwelz * well_nr;
  int segment_well_nr = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_SEGMENTED_WELL_NR_INDEX) - 1; 
  
  if (segment_well_nr == IWEL_SEGMENTED_WELL_NR_NORMAL_VALUE) 
    return false;
  else
    return true;
}


double well_segment_get_depth( const well_segment_type * segment ) {
  return segment->depth;
}

double well_segment_get_length( const well_segment_type * segment ) {
  return segment->length;
}

double well_segment_get_total_length( const well_segment_type * segment ) {
  return segment->total_length;
}

double well_segment_get_diameter( const well_segment_type * segment ) {
  return segment->diameter;
}
