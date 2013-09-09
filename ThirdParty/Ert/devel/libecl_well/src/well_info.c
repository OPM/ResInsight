/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'well_info.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <time.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/int_vector.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_rsthead.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_util.h>

#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_conn.h>
#include <ert/ecl_well/well_state.h>
#include <ert/ecl_well/well_info.h>
#include <ert/ecl_well/well_ts.h>


/*
  The library libwell contains functionality to read and interpret
  some of the well related keywords from an ECLIPSE restart
  file. Roughly speaking the implementation is spread between three
  six datatypes; users of the libwell library are mainly conserned
  with three first.

    well_info_type: This is the container type which holds information
       about all the wells; at all times.

    well_ts_type: The status and properties of a well can typically
       change throughout the simulation; the datatype well_ts_type
       contains a time series for one well.

    well_state_type: The well_state_type datatype contains the
       state/properties of one well at one particular instant of
       time. The well_state.c file contains further documentation of
       the concepts connections, branches and segments.

              
               WELL1

                | |
                | |
                | |
                | |
           +----| |--------+---------------+---------------+
    LGR1   |   :| |:   :   |               |               |
           +���+| |+���+���+               |    (2,2)      |
           |   :| |:   :   |               |               |
           +���+| |+���+���+               |               |
           |   :| |:   :   |               |               |
           +----| |--------+---------------+---------------+
           |   :| |:   :   |               |   :   :   :   |
           +���+| \__________________________________ -+---+   LGR2
           |   :\______________   ___________________| :   |
           +���+���+���+���+   | |         +---+---+---+---+
           |   :   :   :   |   | |         |   :   :   :   |
           +---------------+---| |---------+---------------+
           |               |   | |         |               |
           |               |   | |         |               |
           |   (0,0)       |   | |         |    (2,0)      |
           |               |   |_|         |               |
           |               |               |               |
           +---------------+---------------+---------------+


     This figure shows the following:

       1. A global grid of 3 x 3 times cell; we assume numbering
          starts with (i,j) = (0,0) in the lower left corner.

       2. The grid contains two LGRs named LGR1 and LGR2
          respectively. The LGR LGR1 has size (4,8) and the LGR2 has
          size (4,4).

       3. The grid contains one well called 'WELL1'; the well has the
          following characteristics[*]:

            a) It is perforated both in LGR1 and LGR2 in addition to
               the global grid.
           
            b) It has two branches.

          In the well_state instance this will be represented as:

            i) There are three well_path instances corresponding to
               the global grid and the two LGRs respectively.

            ii) The well_path instances corresponding to the two LGRs
                will have one branch only, whereas the well_path
                corrseponding to the global grid will have two branches. 
          
           In pseudo json:

 well_state =  {
                  {well_path : GLOBAL
                            {branch : 0 [ (0,2) , (0,1) , (1,1) , (2,1) ]},
                            {branch : 1 [ (1,0) ] }
                  },
                  {well_path : LGR1
                            {branch : 0 [(1,5),(1,4),(1,3),(1,2),(1,1),(2,1),(3,1)] }
                  }
                  {well_path : LGR2 : 
                            {branch : 0 [(0,1),(1,1),(2,1)]}
                  }
               }
                       
                     
 [*] Observe that wells in LGR is quite constrained in ECLIPSE; the
     current libwell implementation handles the illustrated case - but
     it might be too complex for ECLIPSE.


  Limitations
  -----------

     Read-only: The well properties for ECLIPSE is specified through
       the SCHEDULE section of the ECLIPSE datafile. The information
       found in restart files is only for reporting/visaulization+++,
       i.e. the code in libwell can unfortunately not be used for well
       modelling.

     segmented wells: The segment information is used to understand
       the branch structure of the well - but nothing else.

*/

/*
  Usage:
  ------

  1. Create a new well_info_type instance with well_info_alloc().

  2. Add restart data - using one of the three functions:

       - well_info_add_wells()       - ecl_file: One report step
       - well_info_add_UNRST_wells() - ecl_file: Many report steps
       - well_info_load_rstfile()    - Restart file name; single file or unified

     There are more details about this in a comment section above the
     well_info_add_wells() function.

  3. Query the well_info instance for information about the wells at
     different times; either through the indirect function
     well_info_get_ts() to get the full timeseries for one named well;
     or one of the functions:

      - well_info_get_state_from_time()
      - well_info_get_state_from_report()
      - well_info_iget_state()
      - well_info_iiget_state()

  4. well_info_free() before you go home.

*/


#define WELL_INFO_TYPE_ID 91777451


struct well_info_struct {
  hash_type           * wells;                /* Hash table of well_ts_type instances; indexed by well name. */
  stringlist_type     * well_names;           /* A list of all the well names. */
  const ecl_grid_type * grid;
};


/**
   The grid pointer is currently not used; but the intention is to use
   it to resolve lgr names.
*/

well_info_type * well_info_alloc( const ecl_grid_type * grid) {
  well_info_type * well_info = util_malloc( sizeof * well_info );
  well_info->wells      = hash_alloc();
  well_info->well_names = stringlist_alloc_new();
  well_info->grid       = grid;
  return well_info;
}


bool well_info_has_well( well_info_type * well_info , const char * well_name ) {
  return hash_has_key( well_info->wells , well_name );
}

well_ts_type * well_info_get_ts( const well_info_type * well_info , const char *well_name) {
  return hash_get( well_info->wells , well_name );
}

static void well_info_add_new_ts( well_info_type * well_info , const char * well_name) {
  well_ts_type * well_ts = well_ts_alloc( well_name ) ;
  hash_insert_hash_owned_ref( well_info->wells , well_name , well_ts , well_ts_free__);
  stringlist_append_copy( well_info->well_names , well_name );
}

static void well_info_add_state( well_info_type * well_info , well_state_type * well_state) {
  const char * well_name = well_state_get_name( well_state );
  if (!well_info_has_well( well_info , well_name))
    well_info_add_new_ts( well_info , well_name );

  {
    well_ts_type * well_ts = well_info_get_ts( well_info , well_name );
    well_ts_add_well( well_ts , well_state );
  }
}


/**
   Which function to use for adding wells?

   There are three different functions which can be used to add wells
   to the well_info structure:

     - well_info_add_wells()
     - well_info_add_UNRST_wells()
     - well_info_load_rstfile()

   The two first functions expect an open ecl_file instance as input;
   whereas the last funtion expects the name of a restart file as
   input.

   If you need ecl_file access to the restart files for another reason
   it might be convenient to use one of the first functions; however
   due to the workings of the ecl_file type it might not be entirely
   obvious: The ecl_file structure will load the needed keywords on
   demand; the keywords needed to initialize well structures will
   typically not be loaded for other purposes, so the only gain from
   using an existing ecl_file instance is that you do not have to
   rebuild the index. The disadvantage of using an existing ecl_file
   instance is that after the call to add_wells() the well related
   kewywords will stay in (probaly unused) in memory.

   The three different methods to add restart data can be
   interchganged, and also called repeatedly. All the relevant data is
   internalized in the well_xxx structures; and the restart files can
   be discarded afterwards.
*/


/**
   This function assumes that (sub)select_block() has been used on the
   ecl_file instance @rst_file; and the function will load well
   information from the first block available in the file only. To
   load all the well information from a unified restart file it is
   easier to use the well_info_add_UNRST_wells() function; which works
   by calling this function repeatedly.

   This function will go through all the wells by number and call the
   well_state_alloc_from_file() function to create a well state object for each
   well. The well_state_alloc_from_file() function will iterate through all the
   grids and assign well properties corresponding to each of the
   grids, the global grid special-cased to determine is consulted to
   determine the number of wells.
 */

void well_info_add_wells( well_info_type * well_info , ecl_file_type * rst_file , int report_nr) {
  int well_nr;
  ecl_rsthead_type * global_header = ecl_rsthead_alloc( rst_file );
  for (well_nr = 0; well_nr < global_header->nwells; well_nr++) {
    well_state_type * well_state = well_state_alloc_from_file( rst_file , well_info->grid , report_nr , well_nr );
    if (well_state != NULL)
      well_info_add_state( well_info , well_state );
  }
  ecl_rsthead_free( global_header );
}

/**
   Observe that this function will fail if the rst_file instance
   corresponds to a non-unified restart file, because these files do
   not have the SEQNUM keyword.
*/

void well_info_add_UNRST_wells( well_info_type * well_info , ecl_file_type * rst_file) {
  {
    int num_blocks = ecl_file_get_num_named_kw( rst_file , SEQNUM_KW );
    int block_nr;
    for (block_nr = 0; block_nr < num_blocks; block_nr++) {
      ecl_file_push_block( rst_file );      // <-------------------------------------------------------
      {                                                                                               //
        ecl_file_subselect_block( rst_file , SEQNUM_KW , block_nr );                                  //  Ensure that the status
        {                                                                                             //  is not changed as a side
          const ecl_kw_type * seqnum_kw = ecl_file_iget_named_kw( rst_file , SEQNUM_KW , 0);          //  effect.
          int report_nr = ecl_kw_iget_int( seqnum_kw , 0 );                                           //
          well_info_add_wells( well_info , rst_file , report_nr );                                    //
        }                                                                                             //
      }                                                                                               //
      ecl_file_pop_block( rst_file );       // <-------------------------------------------------------
    }
  }
}


/**
   The @filename argument should be the name of a restart file; in
   unified or not-unified format - if that is not the case we will
   have crash and burn.
*/

void well_info_load_rstfile( well_info_type * well_info , const char * filename) {
  int report_nr;
  ecl_file_enum file_type = ecl_util_get_file_type( filename , NULL , &report_nr);
  if ((file_type == ECL_RESTART_FILE) || (file_type == ECL_UNIFIED_RESTART_FILE))
  {
    ecl_file_type * ecl_file = ecl_file_open( filename , 0);

    if (file_type == ECL_RESTART_FILE)
      well_info_add_wells( well_info , ecl_file , report_nr );
    else
      well_info_add_UNRST_wells( well_info , ecl_file );
    
    ecl_file_close( ecl_file );
  } else
    util_abort("%s: invalid file type:%s - must be a restart file\n",__func__ , filename);
}


void well_info_free( well_info_type * well_info ) {
  hash_free( well_info->wells );
  stringlist_free( well_info->well_names );
  free( well_info );
}

int well_info_get_well_size( const well_info_type * well_info , const char * well_name ) {
  well_ts_type * well_ts = well_info_get_ts( well_info , well_name );
  return well_ts_get_size( well_ts );
}

/*****************************************************************/

well_state_type * well_info_get_state_from_time( const well_info_type * well_info , const char * well_name , time_t sim_time) {
  well_ts_type * well_ts = well_info_get_ts( well_info , well_name );
  return well_ts_get_state_from_sim_time( well_ts , sim_time );
}


well_state_type * well_info_get_state_from_report( const well_info_type * well_info , const char * well_name , int report_step ) {
  well_ts_type * well_ts = well_info_get_ts( well_info , well_name );
  return well_ts_get_state_from_report( well_ts , report_step);
}

well_state_type * well_info_iget_state( const well_info_type * well_info , const char * well_name , int time_index) {
  well_ts_type * well_ts = well_info_get_ts( well_info , well_name );
  return well_ts_iget_state( well_ts , time_index);
}

well_state_type * well_info_iiget_state( const well_info_type * well_info , int well_index , int time_index) {
  const char * well_name = stringlist_iget( well_info->well_names , well_index );
  return well_info_iget_state( well_info , well_name , time_index );
}

/*****************************************************************/

int well_info_get_num_wells( const well_info_type * well_info ) {
  return stringlist_get_size( well_info->well_names );
}

const char * well_info_iget_well_name( const well_info_type * well_info, int well_index) {
  return stringlist_iget( well_info->well_names , well_index);
}
