/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'well_state.c' is part of ERT - Ensemble based Reservoir Tool.

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

/**
   The well_state_type structure contains state information about one
   well for one particular point in time.
*/


#include <time.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/hash.h>
#include <ert/util/int_vector.h>
#include <ert/util/type_macros.h>

#include <ert/ecl/ecl_rsthead.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_grid.h>

#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_conn.h>
#include <ert/ecl_well/well_state.h>
#include <ert/ecl_well/well_segment_collection.h>
#include <ert/ecl_well/well_branch_collection.h>

/*

Connections, segments and branches
----------------------------------


   +-----+
   |     |  <- Wellhead
   |     |
   +-----+ _________ Segment 2  
      |\  /
      | \/         Segment 1               Segment 0
      |  \-----0---------------0--<----------------------O           <-- Branch: 0
      \        |               |      |                  |
       \    +-----+         +-----++-----+            +-----+
        \   | C3  |         | C2  || C1  |            | C0  |
         \  |     |         |     ||     |            |     |
          \ +-----+         +-----++-----+            +-----+
           \
Segment 5   \
             \
              \        Segment 4                Segment 3    
               \-<--O-------<-------O----------------<------------O   <-- Branch: 1
                    |               |          |                  |
                 +-----+         +-----+    +-----+            +-----+
                 | C7  |         | C6  |    | C5  |            | C4  |
                 |     |         |     |    |     |            |     |
                 +-----+         +-----+    +-----+            +-----+




The boxes show connections C0 - C7; the connections serve as sinks (or
sources in the case of injectors) removing fluids from the
reservoir. As ind             icated by the use of isolated boxes the ECLIPSE model
contains no geomtric concept linking the different connections into a
connected 'well-like' object.

Ordinary wells in the ECLIPSE model are just a collection of
connections like these illustrated boxes, and to draw a connected 1D
object heuristics of some kind must be used to determine how the
various connections should be connected. In particular for wells which
consist of multiple branches this heuristic is non obvious.

More advanced (i.e. newer) wells are modelles as multisegment wells;
the important thing about multisegment wells is that the 1D segments
constituting the flowpipe are modelled explicitly as 'segments', and
the equations of fluid flow are solved by ECLIPSE in these 1D
domains. The figure above shows a multisegment well with six segments
marked as Segment0 ... Segment5. The segments themselves are quite
abstract objects not directly linked to the grid, but indriectly
through the connections. In the figure the segment <-> connection
links are as follows:
    
  Segment0: C0, C1
  Segment1: C2
  Segment2: C3
  Segment3: C4, C5
  Segment4: C6
  Segment5: C7

Each segment has an outlet segment, which will link the segments
together into a geometry. 

The connection can in general be both to the main global grid, and to
an LGR. Hence all questions about connections must be LGR aware. This
is in contrast to the segments and branches which are geometric
objects, not directly coupled to a specific grid; however the segments
have a collection of connections - and these connections are of course
coupledte implementation these objects are modelled as such:

 1. The well_state has hash table which is indexed by LGR name, and
    the values are well_conn_collection instances. The
    well_conn_collection type is a quite simple collection which can
    tell how may connections there are, and index based lookup:


       well_conn_collection_type * connections = well_state_get_grid_connections( well_state , LGR_NAME);
       if (connections) {
          well_conn_type * conn = well_conn_collection_iget( connections , 0 );
          printf("Have %d connections \n",well_conn_collection_get_size( connections );
       }  

    The connections to the global grid are stored with the 'LGR' name
    given by the symbole ECL_GRID_GLOBAL_GRID, or alternatively the
    function well_state_get_global_connections( well_state ) can be
    used.



 2. If - AND ONLY IF - the well is a multisegment well, you can query
    the well_state object for information about segments and branches:

       if (well_state_is_MSW( well_state )) {
          well_segment_collection_type * segments = well_state_get_segments( well_state );  
          well_branch_collection_type * branches = well_state_get_branches( well_state );
          int branch_nr;
          
          for (branch_nr = 0; branch_nr < well_branch_collection_get_size( branches ); branch_nr++) {             
              well_segment_type * segment = well_branch_collection_iget_start_segment( branches , branhc_nr );
              while (segment) {
                  // Inspect the current segment. 
                  segment = well_segment_get_outlet( segment );
              }
          }
       }

     
    

*/


#define WELL_STATE_TYPE_ID 613307832

struct well_state_struct {
  UTIL_TYPE_ID_DECLARATION;
  char           * name;
  time_t           valid_from_time;
  int              valid_from_report;
  int              global_well_nr;
  bool             open;
  well_type_enum   type;

  hash_type      * connections;                                                       // hash<grid_name,well_conn_collection>
  well_segment_collection_type * segments;
  well_branch_collection_type * branches;

  /*****************************************************************/

  vector_type    * index_wellhead;   // An well_conn_type instance representing the wellhead - indexed by grid_nr.
  hash_type      * name_wellhead;    // An well_conn_type instance representing the wellhead - indexed by lgr_name.
};



UTIL_IS_INSTANCE_FUNCTION( well_state , WELL_STATE_TYPE_ID)


well_state_type * well_state_alloc(const char * well_name , int global_well_nr , bool open, well_type_enum type , int report_nr, time_t valid_from) {
  well_state_type * well_state = util_malloc( sizeof * well_state );
  UTIL_TYPE_ID_INIT( well_state , WELL_STATE_TYPE_ID );
  well_state->index_wellhead = vector_alloc_new();
  well_state->name_wellhead  = hash_alloc();

  well_state->name = util_alloc_string_copy( well_name );
  well_state->valid_from_time = valid_from;
  well_state->valid_from_report = report_nr;
  well_state->open = open;
  well_state->type = type;
  well_state->global_well_nr = global_well_nr;
  well_state->connections = hash_alloc();
  well_state->segments = well_segment_collection_alloc();
  well_state->branches = well_branch_collection_alloc();

  /* See documentation of the 'IWEL_UNDOCUMENTED_ZERO' in well_const.h */
  if ((type == UNDOCUMENTED_ZERO) && open)
    util_abort("%s: Invalid type value for open wells.\n",__func__ );
  return well_state;
}





void well_state_add_wellhead( well_state_type * well_state , const ecl_rsthead_type * header , const ecl_kw_type * iwel_kw , int well_nr , const char * grid_name , int grid_nr) {
  well_conn_type * wellhead = well_conn_alloc_wellhead( iwel_kw , header , well_nr );

  if (wellhead != NULL) {
    vector_safe_iset_owned_ref( well_state->index_wellhead , grid_nr , wellhead , well_conn_free__ );
    hash_insert_ref( well_state->name_wellhead , grid_name , wellhead );
  }

}




/*
  This function assumes that the ecl_file state has been restricted
  to one LGR block with the ecl_file_subselect_block() function.

  Return value: -1 means that the well is not found in this LGR at
  all.
*/

static int well_state_get_lgr_well_nr( const well_state_type * well_state , const ecl_file_type * ecl_file) {
  int well_nr = -1;

  if (ecl_file_has_kw( ecl_file , ZWEL_KW)) {
    ecl_rsthead_type  * header  = ecl_rsthead_alloc( ecl_file );
    const ecl_kw_type * zwel_kw = ecl_file_iget_named_kw( ecl_file , ZWEL_KW  , 0 );
    int num_wells               = header->nwells;
    well_nr = 0;
    while (true) {
      bool found = false;
      {
        char * lgr_well_name = util_alloc_strip_copy( ecl_kw_iget_ptr( zwel_kw , well_nr * header->nzwelz) );

        if ( strcmp( well_state->name , lgr_well_name) == 0)
          found = true;
        else
          well_nr++;

        free( lgr_well_name );
      }

      if (found)
        break;
      else if (well_nr == num_wells) {
        // The well is not in this LGR at all.
        well_nr = -1;
        break;
      }

    }
    ecl_rsthead_free( header );
  }
  return well_nr;
}



well_type_enum well_state_translate_ecl_type_int(int int_type) {
  well_type_enum type = UNDOCUMENTED_ZERO;

  switch (int_type) {
    /* See documentation of the 'IWEL_UNDOCUMENTED_ZERO' in well_const.h */
  case(IWEL_UNDOCUMENTED_ZERO):
    type = UNDOCUMENTED_ZERO;
    break;
  case(IWEL_PRODUCER):
    type = PRODUCER;
    break;
  case(IWEL_OIL_INJECTOR):
    type = OIL_INJECTOR;
    break;
  case(IWEL_GAS_INJECTOR):
    type = GAS_INJECTOR;
    break;
  case(IWEL_WATER_INJECTOR):
    type = WATER_INJECTOR;
    break;
  default:
    util_abort("%s: Invalid type value %d\n",__func__ , int_type);
  }
  return type;
}



/*
  This function assumes that the ecl_file state has been restricted
  to one LGR block with the ecl_file_subselect_block() function.
*/

static void well_state_add_connections__( well_state_type * well_state ,
                                          const ecl_file_type * rst_file ,
                                          const char * grid_name ,
                                          int grid_nr,
                                          int well_nr ) {

  ecl_rsthead_type  * header   = ecl_rsthead_alloc( rst_file );
  const ecl_kw_type * icon_kw  = ecl_file_iget_named_kw( rst_file , ICON_KW   , 0);
  const ecl_kw_type * iwel_kw  = ecl_file_iget_named_kw( rst_file , IWEL_KW   , 0);

  //const int iwel_offset        = header->niwelz * well_nr;
  //int seg_well_nr              = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_SEGMENTED_WELL_NR_ITEM) - 1; // -1: Ordinary well.

  well_state_add_wellhead( well_state , header , iwel_kw , well_nr , grid_name , grid_nr );

  if (!well_state_has_grid_connections( well_state , grid_name ))
    hash_insert_hash_owned_ref( well_state->connections , grid_name, well_conn_collection_alloc( ) , well_conn_collection_free__ );

  {
    well_conn_collection_type * wellcc = hash_get( well_state->connections , grid_name );
    well_conn_collection_load_from_kw( wellcc , iwel_kw , icon_kw , well_nr , header );
  }
  ecl_rsthead_free( header );
}


static void well_state_add_global_connections( well_state_type * well_state ,
                                               const ecl_file_type * rst_file ,
                                               int well_nr ) {
  well_state_add_connections__( well_state , rst_file , ECL_GRID_GLOBAL_GRID , 0 , well_nr );
}

static void well_state_add_LGR_connections( well_state_type * well_state , const ecl_grid_type * grid , ecl_file_type * ecl_file, int global_well_nr ) {
  // Go through all the LGRs and add connections; both in the bulk
  // grid and as wellhead.
  int num_lgr = ecl_grid_get_num_lgr( grid );
  int lgr_index;
  for (lgr_index = 0; lgr_index < num_lgr; lgr_index++) {
    ecl_file_push_block( ecl_file );                                  // <-------------------------//
    {                                                                                              //
      ecl_file_subselect_block( ecl_file , LGR_KW , lgr_index );                                      //
      {                                                                                            //  Restrict the file view
        const char * grid_name = ecl_grid_iget_lgr_name( grid , lgr_index );                          //
        int well_nr = well_state_get_lgr_well_nr( well_state , ecl_file );                         //  to one LGR block.
        if (well_nr >= 0)                                                                          //
          well_state_add_connections__( well_state , ecl_file , grid_name , lgr_index + 1, well_nr ); //
      }                                                                                            //
    }                                                                                              //
    ecl_file_pop_block( ecl_file );                                   // <-------------------------//
  }
}



void well_state_add_connections( well_state_type * well_state ,
                                 const ecl_grid_type * grid ,
                                 ecl_file_type * rst_file ,  // Either an open .Xnnnn file or UNRST file restricted to one report step
                                 int well_nr) {

  well_state_add_global_connections( well_state , rst_file , well_nr );
  well_state_add_LGR_connections( well_state , grid , rst_file , well_nr );

}


bool well_state_add_MSW( well_state_type * well_state ,
                         const ecl_file_type * rst_file ,
                         int well_nr) {

  if (ecl_file_has_kw( rst_file , ISEG_KW)) {
    ecl_rsthead_type  * rst_head  = ecl_rsthead_alloc( rst_file );
    const ecl_kw_type * iwel_kw = ecl_file_iget_named_kw( rst_file , IWEL_KW , 0);
    const ecl_kw_type * iseg_kw = ecl_file_iget_named_kw( rst_file , ISEG_KW , 0);
    ecl_kw_type * rseg_kw = NULL;
    
    int segments;

    if (ecl_file_has_kw( rst_file , RSEG_KW )) 
      /* 
         Here we check that the file has the RSEG_KW keyword, and pass
         NULL if not. The rseg_kw pointer will later be used in
         well_segment_collection_load_from_kw() where we test if this
         is a MSW well. If this indeed is a MSW well the rseg_kw
         pointer will be used unchecked, if it is then NULL => Crash
         and burn.
      */
      rseg_kw = ecl_file_iget_named_kw( rst_file , RSEG_KW , 0);

    segments = well_segment_collection_load_from_kw( well_state->segments ,
                                                     well_nr ,
                                                     iwel_kw ,
                                                     iseg_kw ,
                                                     rseg_kw ,
                                                     rst_head);

    if (segments) {
      hash_iter_type * grid_iter = hash_iter_alloc( well_state->connections );
      while (!hash_iter_is_complete( grid_iter )) {
        const char * grid_name = hash_iter_get_next_key( grid_iter );
        const well_conn_collection_type * connections = hash_get( well_state->connections , grid_name );
        well_segment_collection_add_connections( well_state->segments , grid_name , connections );
      }
      hash_iter_free( grid_iter );
      well_segment_collection_link( well_state->segments );
      well_segment_collection_add_branches( well_state->segments , well_state->branches );
    }
    ecl_rsthead_free( rst_head );
    return true;
  } else
    return false;
}


bool well_state_is_MSW( const well_state_type * well_state) {
  if (well_segment_collection_get_size( well_state->segments ) > 0)
    return true;
  else
    return false;
}


well_state_type * well_state_alloc_from_file( ecl_file_type * ecl_file , const ecl_grid_type * grid , int report_nr ,  int global_well_nr) {
  if (ecl_file_has_kw( ecl_file , IWEL_KW)) {
    well_state_type   * well_state = NULL;
    ecl_rsthead_type  * global_header  = ecl_rsthead_alloc( ecl_file );
    const ecl_kw_type * global_iwel_kw = ecl_file_iget_named_kw( ecl_file , IWEL_KW   , 0);
    const ecl_kw_type * global_zwel_kw = ecl_file_iget_named_kw( ecl_file , ZWEL_KW   , 0);

    const int iwel_offset = global_header->niwelz * global_well_nr;
    {
      char * name;
      bool open;
      well_type_enum type = UNDOCUMENTED_ZERO;
      {
        int int_state = ecl_kw_iget_int( global_iwel_kw , iwel_offset + IWEL_STATUS_ITEM );
        if (int_state > 0)
          open = true;
        else
          open = false;
      }

      {
        int int_type = ecl_kw_iget_int( global_iwel_kw , iwel_offset + IWEL_TYPE_ITEM);
        type = well_state_translate_ecl_type_int( int_type );
      }

      {
        const int zwel_offset         = global_header->nzwelz * global_well_nr;
        name = util_alloc_strip_copy(ecl_kw_iget_ptr( global_zwel_kw , zwel_offset ));  // Hardwired max 8 characters in Well Name
      }

      well_state = well_state_alloc(name , global_well_nr , open , type , report_nr , global_header->sim_time);
      free( name );

      well_state_add_connections( well_state , grid , ecl_file , global_well_nr);
      if (ecl_file_has_kw( ecl_file , ISEG_KW))
        well_state_add_MSW( well_state , ecl_file , global_well_nr );
    }
    ecl_rsthead_free( global_header );
    return well_state;
  } else
    /* This seems a bit weird - have come over E300 restart files without the IWEL keyword. */
    return NULL;
}







void well_state_free( well_state_type * well ) {
  hash_free( well->name_wellhead );
  vector_free( well->index_wellhead );
  hash_free( well->connections );
  well_segment_collection_free( well->segments );
  well_branch_collection_free( well->branches );

  free( well->name );
  free( well );
}

/*****************************************************************/

int well_state_get_report_nr( const well_state_type * well_state ) {
  return well_state->valid_from_report;
}

time_t well_state_get_sim_time( const well_state_type * well_state ) {
  return well_state->valid_from_time;
}

/**
   Will return NULL if no wellhead in this grid.
*/
const well_conn_type * well_state_iget_wellhead( const well_state_type * well_state , int grid_nr) {
  return vector_safe_iget_const( well_state->index_wellhead , grid_nr );
}


const well_conn_type * well_state_get_wellhead( const well_state_type * well_state , const char * grid_name) {
  if (hash_has_key( well_state->name_wellhead , grid_name))
    return hash_get( well_state->name_wellhead , grid_name );
  else
    return NULL;
}


well_type_enum well_state_get_type( const well_state_type * well_state){
  return well_state->type;
}

bool well_state_is_open( const well_state_type * well_state ) {
  return well_state->open;
}

int well_state_get_well_nr( const well_state_type * well_state ) {
  return well_state->global_well_nr;
}


const char * well_state_get_name( const well_state_type * well_state ) {
  return well_state->name;
}


/*****************************************************************/

void well_state_summarize( const well_state_type * well_state , FILE * stream ) {
}


const well_conn_collection_type * well_state_get_grid_connections( const well_state_type * well_state , const char * grid_name) {
  if (hash_has_key( well_state->connections , grid_name))
    return hash_get( well_state->connections , grid_name);
  else
    return NULL;
}


const well_conn_collection_type * well_state_get_global_connections( const well_state_type * well_state ) {
  return well_state_get_grid_connections( well_state , ECL_GRID_GLOBAL_GRID );
}


bool well_state_has_grid_connections( const well_state_type * well_state , const char * grid_name) {
  if (hash_has_key( well_state->connections , grid_name))
    return true;
  else
    return false;
}


bool well_state_has_global_connections( const well_state_type * well_state ) {
  return well_state_has_grid_connections( well_state , ECL_GRID_GLOBAL_GRID );
}


well_segment_collection_type * well_state_get_segments( const well_state_type * well_state ) {
  return well_state->segments;
}


well_branch_collection_type * well_state_get_branches( const well_state_type * well_state ) {
  return well_state->branches;
}

