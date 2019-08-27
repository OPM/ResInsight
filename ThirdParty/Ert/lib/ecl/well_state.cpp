/*
   Copyright (C) 2011  Equinor ASA, Norway.

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
#include <ert/util/vector.hpp>
#include <ert/util/hash.hpp>
#include <ert/util/int_vector.hpp>
#include <ert/util/type_macros.hpp>

#include <string>
#include <vector>
#include <map>

#include <ert/ecl/ecl_rsthead.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_units.hpp>
#include <ert/ecl/ecl_util.hpp>

#include <ert/ecl_well/well_const.hpp>
#include <ert/ecl_well/well_conn.hpp>
#include <ert/ecl_well/well_state.hpp>
#include <ert/ecl_well/well_segment_collection.hpp>
#include <ert/ecl_well/well_branch_collection.hpp>
#include <ert/ecl_well/well_rseg_loader.hpp>

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
  Segment4: C6state_free(
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
  std::string      name;
  time_t           valid_from_time;
  int              valid_from_report;
  int              global_well_nr;
  bool             open;
  well_type_enum   type;
  bool             is_MSW_well;
  double           oil_rate;
  double           gas_rate;
  double           water_rate;
  double           volume_rate;
  ert_ecl_unit_enum unit_system;
     
  std::map<std::string, well_conn_collection_type*> connections;              // hash<grid_name,well_conn_collection>
  well_segment_collection_type * segments;
  well_branch_collection_type * branches;

  /*****************************************************************/
  
  std::vector<well_conn_type*> index_wellhead; // An well_conn_type instance representing the wellhead - indexed by grid_nr.   
  std::map<std::string, well_conn_type*> name_wellhead; // An well_conn_type instance representing the wellhead - indexed by lgr_name.
};



UTIL_IS_INSTANCE_FUNCTION( well_state , WELL_STATE_TYPE_ID)


well_state_type * well_state_alloc(const char * well_name , int global_well_nr , bool open, well_type_enum type , int report_nr, time_t valid_from) {
  well_state_type * well_state = new well_state_type();
  UTIL_TYPE_ID_INIT( well_state , WELL_STATE_TYPE_ID );

  well_state->name = well_name;
  well_state->valid_from_time = valid_from;
  well_state->valid_from_report = report_nr;
  well_state->open = open;
  well_state->type = type;
  well_state->global_well_nr = global_well_nr;
  well_state->segments = well_segment_collection_alloc();
  well_state->branches = well_branch_collection_alloc();
  well_state->is_MSW_well = false;
  well_state->oil_rate = 0;
  well_state->gas_rate = 0;
  well_state->water_rate = 0;
  well_state->volume_rate = 0;
  well_state->unit_system = ECL_METRIC_UNITS;

  /* See documentation of the 'IWEL_UNDOCUMENTED_ZERO' in well_const.h */
  if ((type == ECL_WELL_ZERO) && open)
    util_abort("%s: Invalid type value for open wells.\n",__func__ );
  return well_state;
}




double well_state_get_oil_rate( const well_state_type * well_state ) {
  return well_state->oil_rate;
}


double well_state_get_gas_rate( const well_state_type * well_state ) {
  return well_state->gas_rate;
}


double well_state_get_water_rate( const well_state_type * well_state) {
  return well_state->water_rate;
}

double well_state_get_volume_rate( const well_state_type * well_state) {
  return well_state->volume_rate;
}

double well_state_get_oil_rate_si( const well_state_type * well_state ) {
  double conversion_factor = 1;

  if (well_state->unit_system == ECL_METRIC_UNITS)
    conversion_factor = 1.0 / ECL_UNITS_TIME_DAY;
  else if (well_state->unit_system == ECL_FIELD_UNITS)
    conversion_factor = ECL_UNITS_VOLUME_BARREL / ECL_UNITS_TIME_DAY;
  else if (well_state->unit_system == ECL_LAB_UNITS)
    conversion_factor = ECL_UNITS_VOLUME_MILLI_LITER / ECL_UNITS_TIME_HOUR;

  return well_state->oil_rate * conversion_factor;
}


double well_state_get_gas_rate_si( const well_state_type * well_state ) {
  double conversion_factor = 1;

  if (well_state->unit_system == ECL_METRIC_UNITS)
    conversion_factor = 1.0 / ECL_UNITS_TIME_DAY;
  else if (well_state->unit_system == ECL_FIELD_UNITS)
    conversion_factor = ECL_UNITS_VOLUME_GAS_FIELD / ECL_UNITS_TIME_DAY;
  else if (well_state->unit_system == ECL_LAB_UNITS)
    conversion_factor = ECL_UNITS_VOLUME_MILLI_LITER / ECL_UNITS_TIME_HOUR;

  return well_state->gas_rate * conversion_factor;
}


double well_state_get_water_rate_si( const well_state_type * well_state) {
  double conversion_factor = 1;

  if (well_state->unit_system == ECL_METRIC_UNITS)
    conversion_factor = 1.0 / ECL_UNITS_TIME_DAY;
  else if (well_state->unit_system == ECL_FIELD_UNITS)
    conversion_factor = ECL_UNITS_VOLUME_BARREL / ECL_UNITS_TIME_DAY;
  else if (well_state->unit_system == ECL_LAB_UNITS)
    conversion_factor = ECL_UNITS_VOLUME_MILLI_LITER / ECL_UNITS_TIME_HOUR;

  return well_state->water_rate * conversion_factor;
}

double well_state_get_volume_rate_si( const well_state_type * well_state) {
  return well_state->volume_rate;
}


void well_state_add_wellhead( well_state_type * well_state , const ecl_rsthead_type * header , const ecl_kw_type * iwel_kw , int well_nr , const char * grid_name , int grid_nr) {
  well_conn_type * wellhead = well_conn_alloc_wellhead( iwel_kw , header , well_nr );

  if (wellhead != NULL) {
    if (grid_nr >= static_cast<int>(well_state->index_wellhead.size()))
      well_state->index_wellhead.resize(grid_nr+1, NULL);
    well_state->index_wellhead[grid_nr] = wellhead;
    well_state->name_wellhead[grid_name] = wellhead;
  }

}

static bool well_state_add_rates( well_state_type * well_state ,
                                  ecl_file_view_type * rst_view ,
                                  int well_nr) {

  bool has_xwel_kw = ecl_file_view_has_kw(rst_view, XWEL_KW);
  if (has_xwel_kw) {
    const ecl_kw_type *xwel_kw = ecl_file_view_iget_named_kw(rst_view, XWEL_KW, 0);
    ecl_rsthead_type *header = ecl_rsthead_alloc(rst_view, -1);
    int offset = header->nxwelz * well_nr;

    well_state->unit_system = header->unit_system;
    well_state->oil_rate = ecl_kw_iget_double(xwel_kw, offset + XWEL_RES_ORAT_ITEM);
    well_state->gas_rate = ecl_kw_iget_double(xwel_kw, offset + XWEL_RES_GRAT_ITEM);
    well_state->water_rate = ecl_kw_iget_double(xwel_kw, offset + XWEL_RES_WRAT_ITEM);
    well_state->volume_rate = ecl_kw_iget_double(xwel_kw, offset + XWEL_RESV_ITEM);

    ecl_rsthead_free(header);
  }
  return has_xwel_kw;
}




/*
  This function assumes that the ecl_file state has been restricted
  to one LGR block with the ecl_file_subselect_block() function.

  Return value: -1 means that the well is not found in this LGR at
  all.
*/

static int well_state_get_lgr_well_nr( const well_state_type * well_state , const ecl_file_view_type * file_view) {
  int well_nr = -1;

  if (ecl_file_view_has_kw( file_view , ZWEL_KW)) {
    ecl_rsthead_type  * header  = ecl_rsthead_alloc( file_view , -1);
    const ecl_kw_type * zwel_kw = ecl_file_view_iget_named_kw( file_view , ZWEL_KW  , 0 );
    int num_wells               = header->nwells;
    well_nr = 0;
    while (true) {
      bool found = false;
      {
        char * lgr_well_name = (char*)util_alloc_strip_copy( (const char*)ecl_kw_iget_ptr( zwel_kw , well_nr * header->nzwelz) );

        if ( well_state->name == lgr_well_name)
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
  well_type_enum type = ECL_WELL_ZERO;

  switch (int_type) {
    /* See documentation of the 'IWEL_UNDOCUMENTED_ZERO' in well_const.h */
  case(IWEL_UNDOCUMENTED_ZERO):
    type = ECL_WELL_ZERO;
    break;
  case(IWEL_PRODUCER):
    type = ECL_WELL_PRODUCER;
    break;
  case(IWEL_OIL_INJECTOR):
    type = ECL_WELL_OIL_INJECTOR;
    break;
  case(IWEL_GAS_INJECTOR):
    type = ECL_WELL_GAS_INJECTOR;
    break;
  case(IWEL_WATER_INJECTOR):
    type = ECL_WELL_WATER_INJECTOR;
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
                                          const ecl_file_view_type * rst_view ,
                                          const char * grid_name ,
                                          int grid_nr,
                                          int well_nr ) {

  ecl_rsthead_type  * header   = ecl_rsthead_alloc( rst_view , -1);
  const ecl_kw_type * iwel_kw  = ecl_file_view_iget_named_kw( rst_view , IWEL_KW   , 0);


  well_state_add_wellhead( well_state , header , iwel_kw , well_nr , grid_name , grid_nr );

  if (ecl_file_view_has_kw(rst_view, ICON_KW)) {
    const ecl_kw_type * icon_kw  = ecl_file_view_iget_named_kw( rst_view , ICON_KW   , 0);
    if (!well_state_has_grid_connections( well_state , grid_name ))
      well_state->connections[grid_name] = well_conn_collection_alloc();

    {
      ecl_kw_type * scon_kw = NULL;
      if (ecl_file_view_has_kw( rst_view , SCON_KW))
        scon_kw = ecl_file_view_iget_named_kw( rst_view , SCON_KW , 0);

      ecl_kw_type * xcon_kw = NULL;
      if (ecl_file_view_has_kw( rst_view , XCON_KW)) {
        xcon_kw = ecl_file_view_iget_named_kw(rst_view, XCON_KW, 0);
      }

      well_conn_collection_type * wellcc = well_state->connections[grid_name];
      well_conn_collection_load_from_kw( wellcc , iwel_kw , icon_kw , scon_kw, xcon_kw , well_nr , header );
    }
  }
  ecl_rsthead_free( header );
}


static void well_state_add_global_connections( well_state_type * well_state ,
                                               const ecl_file_view_type * rst_view ,
                                               int well_nr ) {
  well_state_add_connections__( well_state , rst_view , ECL_GRID_GLOBAL_GRID , 0 , well_nr );
}

static void well_state_add_LGR_connections(well_state_type * well_state,
                                           const ecl_grid_type * grid,
                                           ecl_file_view_type * file_view) {
  // Go through all the LGRs and add connections; both in the bulk
  // grid and as wellhead.

  int num_lgr = ecl_grid_get_num_lgr( grid );
  for (int lgr_index = 0; lgr_index < num_lgr; lgr_index++) {
    ecl_file_view_type * lgr_view = ecl_file_view_add_blockview(file_view , LGR_KW , lgr_index);
    /*
      Even though the grid has LGR information the restart file is not required
      to have corresponding LGR information. This has for a long time been
      unchecked, and there might be bugs lurking based on the incorrect
      assumption that if the grid has LGR information then the corresponding LGR
      information can also be found in the restart file.
    */
    if (lgr_view) {
      const char * grid_name = ecl_grid_iget_lgr_name( grid , lgr_index );
      int well_nr = well_state_get_lgr_well_nr( well_state , lgr_view );
      if (well_nr >= 0)
        well_state_add_connections__( well_state , lgr_view , grid_name , lgr_index + 1, well_nr );
    }
  }
}



void well_state_add_connections( well_state_type * well_state ,
                                 const ecl_grid_type * grid ,
                                 ecl_file_type * rst_file ,  // Either an open .Xnnnn file or UNRST file restricted to one report step
                                 int well_nr) {

  well_state_add_connections2(well_state , grid , ecl_file_get_active_view( rst_file ) , well_nr );

}


void well_state_add_connections2( well_state_type * well_state ,
                                 const ecl_grid_type * grid ,
                                 ecl_file_view_type * rst_view ,
                                 int well_nr) {

  well_state_add_global_connections( well_state , rst_view , well_nr );
  well_state_add_LGR_connections( well_state , grid , rst_view);

}



bool well_state_add_MSW( well_state_type * well_state ,
                         ecl_file_type * rst_file ,
                         int well_nr,
                         bool load_segment_information) {

  return well_state_add_MSW2( well_state , ecl_file_get_active_view(rst_file) , well_nr , load_segment_information );
}


bool well_state_add_MSW2( well_state_type * well_state ,
                          ecl_file_view_type * rst_view ,
                          int well_nr,
                          bool load_segment_information) {

  if (ecl_file_view_has_kw( rst_view , ISEG_KW)) {
    ecl_rsthead_type  * rst_head  = ecl_rsthead_alloc( rst_view , -1);
    const ecl_kw_type * iwel_kw = ecl_file_view_iget_named_kw( rst_view , IWEL_KW , 0);
    const ecl_kw_type * iseg_kw = ecl_file_view_iget_named_kw( rst_view , ISEG_KW , 0);
    well_rseg_loader_type * rseg_loader = NULL;

    int segment_count;

    if (ecl_file_view_has_kw( rst_view , RSEG_KW )) {
      if (load_segment_information)
        rseg_loader = well_rseg_loader_alloc(rst_view);

      segment_count = well_segment_collection_load_from_kw( well_state->segments ,
                                                            well_nr ,
                                                            iwel_kw ,
                                                            iseg_kw ,
                                                            rseg_loader ,
                                                            rst_head,
                                                            load_segment_information ,
                                                            &well_state->is_MSW_well);


      if (segment_count > 0) {

        auto it = well_state->connections.begin();
        while (it != well_state->connections.end()) {
          well_segment_collection_add_connections( well_state->segments , it->first.c_str() , it->second );
          it++;
        }

        well_segment_collection_link( well_state->segments );
        well_segment_collection_add_branches( well_state->segments , well_state->branches );
      }
      ecl_rsthead_free( rst_head );

      if (rseg_loader != NULL) {
        well_rseg_loader_free(rseg_loader);
      }

      return true;
    }
  }
  return false;
}


bool well_state_is_MSW( const well_state_type * well_state) {
  return well_state->is_MSW_well;
}

bool well_state_has_segment_data(const well_state_type * well_state){
    if (well_segment_collection_get_size( well_state->segments ) > 0)
      return true;
    else
      return false;
}

well_state_type * well_state_alloc_from_file( ecl_file_type * ecl_file , const ecl_grid_type * grid , int report_nr ,  int global_well_nr ,bool load_segment_information) {
  return well_state_alloc_from_file2( ecl_file_get_active_view( ecl_file ) , grid , report_nr , global_well_nr , load_segment_information);
}

well_state_type * well_state_alloc_from_file2( ecl_file_view_type * file_view , const ecl_grid_type * grid , int report_nr ,  int global_well_nr ,bool load_segment_information) {
  if (ecl_file_view_has_kw( file_view , IWEL_KW)) {
    well_state_type   * well_state = NULL;
    ecl_rsthead_type  * global_header  = ecl_rsthead_alloc( file_view , -1);
    const ecl_kw_type * global_iwel_kw = ecl_file_view_iget_named_kw( file_view , IWEL_KW   , 0);
    const ecl_kw_type * global_zwel_kw = ecl_file_view_iget_named_kw( file_view , ZWEL_KW   , 0);

    const int iwel_offset = global_header->niwelz * global_well_nr;
    {
      char * name;
      bool open;
      well_type_enum type = ECL_WELL_ZERO;
      {
        int int_state = ecl_kw_iget_int( global_iwel_kw , iwel_offset + IWEL_STATUS_INDEX );
        if (int_state > 0)
          open = true;
        else
          open = false;
      }

      {
        int int_type = ecl_kw_iget_int( global_iwel_kw , iwel_offset + IWEL_TYPE_INDEX);
        type = well_state_translate_ecl_type_int( int_type );
      }

      {
        const int zwel_offset         = global_header->nzwelz * global_well_nr;
        name = (char*)util_alloc_strip_copy((const char*)ecl_kw_iget_ptr( global_zwel_kw , zwel_offset ));  // Hardwired max 8 characters in Well Name
      }

      well_state = well_state_alloc(name , global_well_nr , open , type , report_nr , global_header->sim_time);
      free( name );

      well_state_add_connections2( well_state , grid , file_view , global_well_nr);
      if (ecl_file_view_has_kw( file_view , ISEG_KW))
        well_state_add_MSW2( well_state , file_view , global_well_nr , load_segment_information);

      well_state_add_rates(well_state, file_view, global_well_nr);
    }
    ecl_rsthead_free( global_header );
    return well_state;
  } else
    /* This seems a bit weird - have come over E300 restart files without the IWEL keyword. */
    return NULL;
}







void well_state_free( well_state_type * well ) {

  for (size_t i = 0; i < well->index_wellhead.size(); i++) {
    if (well->index_wellhead[i])
       well_conn_free(well->index_wellhead[i]);
  }

  for (auto& pair : well->connections)
    well_conn_collection_free(pair.second);

  well_segment_collection_free( well->segments );
  well_branch_collection_free( well->branches );

  delete well;
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
  if (grid_nr < static_cast<int>(well_state->index_wellhead.size()))
    return well_state->index_wellhead[grid_nr];
  else
    return NULL;
}


bool well_state_has_named_well_conn( const well_state_type * well_state , const char * grid_name ) {
  const auto it = well_state->name_wellhead.find( grid_name );
  if (it ==  well_state->name_wellhead.end())
    return false;
  return true;
}


const well_conn_type * well_state_get_wellhead( const well_state_type * well_state , const char * grid_name) {
  const auto it = well_state->name_wellhead.find( grid_name );
  if (it !=  well_state->name_wellhead.end())
    return it->second;
  return NULL;
}

const well_conn_type * well_state_get_global_wellhead( const well_state_type * well_state ) {
  return well_state_get_wellhead( well_state, ECL_GRID_GLOBAL_GRID );
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
  return well_state->name.c_str();
}


/*****************************************************************/

const well_conn_collection_type * well_state_get_grid_connections( const well_state_type * well_state , const char * grid_name) {
  if (well_state_has_grid_connections(well_state, grid_name)  )
    return well_state->connections.at(grid_name);
  else
    return NULL;
}


const well_conn_collection_type * well_state_get_global_connections( const well_state_type * well_state ) {
  return well_state_get_grid_connections( well_state , ECL_GRID_GLOBAL_GRID );
}


bool well_state_has_grid_connections( const well_state_type * well_state , const char * grid_name) {

  const auto it = well_state->connections.find(grid_name);
  if (it == well_state->connections.end())
    return false;
  return true;
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
