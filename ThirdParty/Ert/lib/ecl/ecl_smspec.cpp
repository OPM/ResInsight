/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_smspec.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <memory>

#include <ert/util/hash.hpp>
#include <ert/util/util.h>
#include <ert/util/float_vector.hpp>
#include <ert/util/stringlist.hpp>
#include "detail/util/path.hpp"

#include <ert/ecl/ecl_smspec.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/smspec_node.hpp>
#include <ert/ecl/ecl_endian_flip.hpp>
#include <ert/ecl/ecl_type.hpp>

#ifdef HAVE_FNMATCH
#include <fnmatch.h>
#endif


/**
   This file implements the indexing into the ECLIPSE summary files.
*/

/*
  Supporting a new variable type:
  -------------------------------

  1. The function smspec_node_alloc() must be updated to return a valid
     ecl::smspec_node instance when called with the new var_type.

  2. Update the function ecl_smpec_install_gen_key() to install smpec_index
     instances of this particular type. The format of the general key is
     implicitly defined in this function.

  3. The ecl_smspec structure supports two different types of lookup:

       a) Lookup based on general key like e.g. WWCT:OP6
       b) Specific lookup based on the variable type, i.e. :

              ecl_smspec_get_well_var( xxx , well_name , var).

      Historically everything started with specific lookup as in case b);
      however the general lookup proved to be very convenient, and the
      specfic lookup method has seen less use[*]. The final step in
      supporting a new variable is to update the function
      ecl_smspec_fread_header().

      If you want to support specific lookup of the new variable type you
      must add the necessary datastructures to the ecl_smspec_struct
      structure and then subsequently fill that structure in the big
      switch() in ecl_smspec_fread_header() - if you do not care about
      specific lookup you just have to add an empty case() slot to the
      switch in ecl_smspec_fread_header(). The LGR variables, and also
      ECL_SMSPEC_SEGMENT_VAR do not support specific lookup.

      [*]: The advantage of the specific lookup is that it is possible
           to supply better error messages (The well 'XX' does not
           exist, instead of just unknown key: 'WWCT:XX'), and it is
           also possible to support queries like: give me all the
           well names.

  4. Mark the variable type as supported with a 'X' in the defintion of
     ecl_smspec_var_type in ecl_smspec.h.

 */



#define ECL_SMSPEC_ID          806647
#define PARAMS_GLOBAL_DEFAULT  -99

typedef std::map<std::string, const ecl::smspec_node * > node_map;

struct ecl_smspec_struct {
  UTIL_TYPE_ID_DECLARATION;
  /*
    All the hash tables listed below here are different ways to access
    smspec_node instances. The actual smspec_node instances are
    owned by the smspec_nodes vector;
  */
  node_map             field_var_index;
  node_map             misc_var_index;            /* Variables like 'TCPU' and 'NEWTON'. */
  node_map             gen_var_index              /* This is "everything" - things can either be found as gen_var("WWCT:OP_X") or as well_var("WWCT" , "OP_X") */;

  std::map<std::string, node_map> well_var_index; /* Indexes for all well variables:
                                                     {well1: {var1: index1 , var2: index2} , well2: {var1: index1 , var2: index2}} */
  std::map<std::string, node_map> group_var_index;   /* Indexes for group variables.*/
  std::map<int,         node_map> region_var_index;  /* The stored index is an offset. */
  std::map<int,         node_map> block_var_index;   /* Block variables like BPR */
  std::map<std::string, std::map<int, node_map>> well_completion_var_index; /* Indexes for completion indexes .*/


  std::vector<std::unique_ptr<ecl::smspec_node>> smspec_nodes;
  bool                 write_mode;
  bool                 need_nums;
  std::vector<int>     index_map;
  std::map<int, int>   inv_index_map;
  int                  params_size;
  /*-----------------------------------------------------------------*/

  int               time_seconds;
  int               grid_dims[3];                 /* Grid dimensions - in DIMENS[1,2,3] */
  int               num_regions;
  int               Nwells , param_offset;
  std::string       key_join_string;               /* The string used to join keys when building gen_key keys - typically ":" -
                                                      but arbitrary - NOT necessary to be able to invert the joining. */
  std::string         header_file;                   /* FULL path to the currenbtly loaded header_file. */

  bool                formatted;                     /* Has this summary instance been loaded from a formatted (i.e. FSMSPEC file) or unformatted (i.e. SMSPEC) file. */
  time_t              sim_start_time;                /* When did the simulation start - worldtime. */

  int                 time_index;                    /* The fields time_index, day_index, month_index and year_index */
  int                 day_index;                     /* are used by the ecl_sum_data object to locate per. timestep */
  int                 month_index;                   /* time information. */
  int                 year_index;
  bool                has_lgr;
  std::vector<float>  params_default;

  std::string         restart_case;
  ert_ecl_unit_enum   unit_system;
  int                 restart_step;
};


/**
About indexing:
---------------

The ECLISPE summary files are organised (roughly) like this:

 1. A header-file called xxx.SMPSEC is written, which is common to
    every timestep.

 2. For each timestep the summary vector is written in the form of a
    vector 'PARAMS' of length N with floats. In the PARAMS vector all
    types of data are stacked togeheter, and one must use the header
    info in the SMSPEC file to disentangle the summary data.

Here I will try to describe how the header in SMSPEC is organised, and
how that support is imlemented here. The SMSPEC header is organized
around three character vectors, of length N. To find the position in
the PARAMS vector of a certain quantity, you must consult one, two or
all three of these vectors. The most important vecor - which must
always be consulted is the KEYWORDS vector, then it is the WGNAMES and
NUMS (integer) vectors whcih must be consulted for some variable
types.


Let us a consider a system consisting of:

  * Two wells: P1 and P2 - for each well we have variables WOPR, WWCT
    and WGOR.

  * Three regions: For each region we have variables RPR and RXX(??)

  * We have stored field properties FOPT and FWPT


KEYWORDS = ['TIME','FOPR','FPR','FWCT','WOPR','WOPR,'WWCT','WWCT]
       ....



general_var:
------------
VAR_TYPE:(WELL_NAME|GROUP_NAME|NUMBER):NUMBER

Field var:         VAR_TYPE
Misc var:          VAR_TYPE
Well var:          VAR_TYPE:WELL_NAME
Group var:         VAR_TYPE:GROUP_NAME
Block var:         VAR_TYPE:i,j,k  (where i,j,k is calculated form NUM)
Region var         VAR_TYPE:index  (where index is NOT from the nums vector, it it is just an offset).
Completion var:    VAR_TYPE:WELL_NAME:NUM
....

*/




/*
  The smspec_required_keywords variable contains a list of keywords
  which are *absolutely* required in the SMSPEC file, but observe that
  depending on the content of the "KEYWORDS" array other keywords
  might bre requred as well - this typically includes the NUMS
  keyword. Such 'second-order' dependencies are not accounted for with
  this simple list.
*/

static const size_t num_req_keywords = 5;
static const char* smspec_required_keywords[] = {
                                                   WGNAMES_KW,
                                                   KEYWORDS_KW,
                                                   STARTDAT_KW,
                                                   UNITS_KW,
                                                   DIMENS_KW
                                                 };

namespace {

const ecl::smspec_node * ecl_smspec_get_var_node( const node_map& mp, const char * var) {
    const auto it = mp.find(var);
    if (it == mp.end())
      return nullptr;

    return it->second;
  }

  const ecl::smspec_node * ecl_smspec_get_str_key_var_node( const std::map<std::string, node_map>& mp , const char * key , const char * var) {
    const auto key_it = mp.find(key);
    if (key_it == mp.end())
      return nullptr;

    const node_map& var_map = key_it->second;
    return ecl_smspec_get_var_node(var_map, var);
  }

  const ecl::smspec_node * ecl_smspec_get_int_key_var_node(const std::map<int, node_map>& mp , int key , const char * var) {
    const auto key_it = mp.find(key);
    if (key_it == mp.end())
      return nullptr;

    const auto& var_map = key_it->second;
    return ecl_smspec_get_var_node(var_map, var);
  }

} //end namespace

int ecl_smspec_num_nodes( const ecl_smspec_type * smspec) {
  return smspec->smspec_nodes.size();
}

/*
  When loading a summary case from file many of the nodes can be ignored, in
  that case the size of PARAMS vector in the data files is larger than the
  number of internalized nodes. Therefor we need to maintain the
  params_size member.
*/

int ecl_smspec_get_params_size( const ecl_smspec_type * smspec ) {
  return smspec->params_size;
}



/*****************************************************************/

ecl_smspec_type * ecl_smspec_alloc_empty(bool write_mode , const char * key_join_string) {
  ecl_smspec_type * ecl_smspec = new ecl_smspec_type();
  UTIL_TYPE_ID_INIT(ecl_smspec , ECL_SMSPEC_ID);

  ecl_smspec->sim_start_time                 = -1;
  ecl_smspec->key_join_string                = key_join_string;
  ecl_smspec->header_file                    = "";

  ecl_smspec->time_index   = -1;
  ecl_smspec->day_index    = -1;
  ecl_smspec->year_index   = -1;
  ecl_smspec->month_index  = -1;
  ecl_smspec->time_seconds = -1;
  ecl_smspec->params_size  = -1;

  /*
    The unit system is given as an integer in the INTEHEAD keyword. The INTEHEAD
    keyword is optional, and we have for a long time been completely oblivious
    to the possibility of extracting unit system information from the SMSPEC file.
  */
  ecl_smspec->unit_system  = ECL_METRIC_UNITS;

  ecl_smspec->restart_step = -1;
  ecl_smspec->write_mode = write_mode;
  ecl_smspec->need_nums = false;

  return ecl_smspec;
}


int * ecl_smspec_alloc_mapping( const ecl_smspec_type * self, const ecl_smspec_type * other) {
  int params_size = ecl_smspec_get_params_size( self );
  int * mapping = (int*) util_malloc( params_size * sizeof * mapping );

  for (int i = 0; i < params_size; i++)
    mapping[i] = -1;


  for (int i=0; i < ecl_smspec_num_nodes( self ); i++) {
    const ecl::smspec_node& self_node = ecl_smspec_iget_node_w_node_index( self , i );
    int self_index = self_node.get_params_index();
    const char * key = self_node.get_gen_key1();
    if (ecl_smspec_has_general_var( other , key)) {
      const ecl::smspec_node& other_node = ecl_smspec_get_general_var_node( other , key);
      int other_index = other_node.get_params_index();
      mapping[ self_index ]  =  other_index;
    }
  }

  return mapping;
}


/**
   Observe that the index here is into the __INTERNAL__ indexing in
   the smspec_nodes vector; and in general widely different from the
   params_index of the returned smspec_node instance.
*/


const ecl::smspec_node& ecl_smspec_iget_node_w_node_index( const ecl_smspec_type * smspec , int node_index ) {
  const auto& node = smspec->smspec_nodes[node_index];
  return *node.get();
}


/*
  The ecl_smspec_iget_node() function is only retained for compatibility; should be
  replaced with calls to the more explicit: ecl_smspec_iget_node_w_node_index().
*/

const ecl::smspec_node& ecl_smspec_iget_node(const ecl_smspec_type * smspec, int index) {
  return ecl_smspec_iget_node_w_node_index(smspec, index);
}

const ecl::smspec_node& ecl_smspec_iget_node_w_params_index( const ecl_smspec_type * smspec , int params_index ) {
  int node_index = smspec->inv_index_map.at(params_index);
  return ecl_smspec_iget_node_w_node_index(smspec, node_index);
}

/**
 * Returns an ecl data type for which all names will fit. If the maximum name
 * length is at most 8, an ECL_CHAR is returned and otherwise a large enough
 * ECL_STRING.
 */
static ecl_data_type get_wgnames_type(const ecl_smspec_type * smspec) {
  size_t max_len = 0;
  for(int i = 0; i < ecl_smspec_num_nodes(smspec); ++i) {
    const ecl::smspec_node& node = ecl_smspec_iget_node_w_node_index(smspec, i);
    const char * name = smspec_node_get_wgname( &node );
    if(name)
      max_len = util_size_t_max(max_len, strlen(name));
  }

  return max_len <= ECL_STRING8_LENGTH ? ECL_CHAR : ECL_STRING(max_len);
}

static void ecl_smspec_fwrite_INTEHEAD(const ecl_smspec_type * smspec, fortio_type * fortio) {
  ecl_kw_type * intehead = ecl_kw_alloc( INTEHEAD_KW, INTEHEAD_SMSPEC_SIZE, ECL_INT);
  ecl_kw_iset_int(intehead, INTEHEAD_SMSPEC_UNIT_INDEX, smspec->unit_system);
  /*
    The simulator type is just hardcoded to ECLIPSE100.
  */
  ecl_kw_iset_int(intehead, INTEHEAD_SMSPEC_IPROG_INDEX, INTEHEAD_ECLIPSE100_VALUE);
  ecl_kw_fwrite(intehead, fortio);
  ecl_kw_free(intehead);
}


static void ecl_smspec_fwrite_RESTART(const ecl_smspec_type * smspec, fortio_type * fortio) {
  ecl_kw_type * restart_kw = ecl_kw_alloc( RESTART_KW , SUMMARY_RESTART_SIZE , ECL_CHAR );
  for (int i=0; i < SUMMARY_RESTART_SIZE; i++)
    ecl_kw_iset_string8( restart_kw , i , "");

  if (smspec->restart_case.size() > 0) {
    size_t restart_case_len = smspec->restart_case.size();

    size_t offset = 0;
    for (size_t i = 0; i < SUMMARY_RESTART_SIZE ; i++) {
      if (offset < restart_case_len)
        ecl_kw_iset_string8( restart_kw , i , &smspec->restart_case[ offset ]);
      offset += ECL_STRING8_LENGTH;
    }
  }
  ecl_kw_fwrite( restart_kw , fortio );
  ecl_kw_free( restart_kw );
}



static void ecl_smspec_fwrite_DIMENS(const ecl_smspec_type * smspec, fortio_type * fortio) {
  ecl_kw_type * dimens_kw = ecl_kw_alloc( DIMENS_KW , DIMENS_SIZE , ECL_INT );
  int num_nodes = ecl_smspec_num_nodes( smspec );
  ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_SIZE_INDEX , num_nodes );
  ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_NX_INDEX   , smspec->grid_dims[0] );
  ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_NY_INDEX   , smspec->grid_dims[1] );
  ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_NZ_INDEX   , smspec->grid_dims[2] );
  ecl_kw_iset_int( dimens_kw , 4  , 0 );  // Do not know what this is for.
  ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_RESTART_STEP_INDEX , smspec->restart_step );

  ecl_kw_fwrite( dimens_kw , fortio );
  ecl_kw_free( dimens_kw );
}


static void ecl_smspec_fwrite_STARTDAT(const ecl_smspec_type * smspec, fortio_type * fortio) {
  ecl_kw_type * startdat_kw = ecl_kw_alloc( STARTDAT_KW , STARTDAT_SIZE , ECL_INT );
  int day,month,year;
  ecl_util_set_date_values( smspec->sim_start_time , &day, &month , &year);

  ecl_kw_iset_int( startdat_kw , STARTDAT_DAY_INDEX   , day );
  ecl_kw_iset_int( startdat_kw , STARTDAT_MONTH_INDEX , month );
  ecl_kw_iset_int( startdat_kw , STARTDAT_YEAR_INDEX  , year );

  ecl_kw_fwrite( startdat_kw , fortio );
  ecl_kw_free( startdat_kw );
}


static void ecl_smspec_fortio_fwrite( const ecl_smspec_type * smspec , fortio_type * fortio) {
  ecl_smspec_fwrite_INTEHEAD(smspec, fortio);
  ecl_smspec_fwrite_RESTART(smspec, fortio);
  ecl_smspec_fwrite_DIMENS(smspec, fortio);

  int num_nodes = ecl_smspec_num_nodes( smspec );
  ecl_kw_type * keywords_kw = ecl_kw_alloc( KEYWORDS_KW , num_nodes , ECL_CHAR );
  ecl_kw_type * units_kw    = ecl_kw_alloc( UNITS_KW    , num_nodes , ECL_CHAR );
  ecl_kw_type * nums_kw     = NULL;

  // If the names_type is an ECL_STRING we expect this to be an INTERSECT
  // summary, otherwise an ECLIPSE summary.
  ecl_data_type names_type  = get_wgnames_type(smspec);
  ecl_kw_type * wgnames_kw = ecl_kw_alloc( ecl_type_is_char(names_type) ? WGNAMES_KW : NAMES_KW,
                                           num_nodes,
                                           names_type );

  if (smspec->need_nums)
    nums_kw = ecl_kw_alloc( NUMS_KW , num_nodes , ECL_INT);

  for (int i=0; i < ecl_smspec_num_nodes( smspec ); i++) {
    const ecl::smspec_node& smspec_node = ecl_smspec_iget_node_w_node_index( smspec , i );
    /*
      It is possible to add variables with deferred initialisation
      with the ecl_sum_add_blank_var() function. Before these
      variables can be actually used for anything interesting they
      must be initialized with the ecl_sum_init_var() function.

      If a call to save the smspec file comes before all the
      variable have been initialized things will potentially go
      belly up. This is solved with the following uber-hack:

      o One of the well related keywords is chosen; in
        particular 'WWCT' in this case.

      o The wgname value is set to DUMMY_WELL

      The use of DUMMY_WELL ensures that this field will be
      ignored when/if this smspec file is read in at a later
      stage.
    */
    if (smspec_node.get_var_type() == ECL_SMSPEC_INVALID_VAR) {
      ecl_kw_iset_string8( keywords_kw , i , "WWCT" );
      ecl_kw_iset_string8( units_kw , i , "????????");
      ecl_kw_iset_string_ptr( wgnames_kw , i , DUMMY_WELL);
    } else {
      ecl_kw_iset_string8( keywords_kw , i , smspec_node_get_keyword( &smspec_node ));
      ecl_kw_iset_string8( units_kw , i , smspec_node_get_unit( &smspec_node ));
      {
        const char * wgname = DUMMY_WELL;
        if (smspec_node_get_wgname( &smspec_node ))
          wgname = smspec_node_get_wgname( &smspec_node );
        ecl_kw_iset_string_ptr( wgnames_kw , i , wgname);
      }
    }

    if (nums_kw != NULL)
      ecl_kw_iset_int( nums_kw , i , smspec_node.get_num());
  }
  ecl_kw_fwrite( keywords_kw , fortio );
  ecl_kw_fwrite( wgnames_kw , fortio );
  if (nums_kw != NULL)
    ecl_kw_fwrite( nums_kw , fortio );
  ecl_kw_fwrite( units_kw , fortio );

  ecl_kw_free( keywords_kw );
  ecl_kw_free( wgnames_kw );
  ecl_kw_free( units_kw );
  if (nums_kw != NULL)
    ecl_kw_free( nums_kw );

  ecl_smspec_fwrite_STARTDAT(smspec, fortio);
}


void ecl_smspec_fwrite( const ecl_smspec_type * smspec , const char * ecl_case , bool fmt_file ) {
  char * filename = ecl_util_alloc_filename( NULL , ecl_case , ECL_SUMMARY_HEADER_FILE , fmt_file , 0 );
  fortio_type * fortio = fortio_open_writer( filename , fmt_file , ECL_ENDIAN_FLIP);

  if (!fortio) {
    const char * error_fmt_msg = "%s: Unable to open fortio file %s, error: %s .\n";
    util_abort( error_fmt_msg , __func__ , filename , strerror( errno ) );
  }

  ecl_smspec_fortio_fwrite( smspec , fortio );

  fortio_fclose( fortio );
  free( filename );
}



static ecl_smspec_type * ecl_smspec_alloc_writer__( const char * key_join_string , const char * restart_case, int restart_step, time_t sim_start , bool time_in_days , int nx , int ny , int nz) {
 ecl_smspec_type * ecl_smspec = ecl_smspec_alloc_empty( true , key_join_string );
  /*
    Only a total of 9 * 8 characters is set aside for the restart keyword, if
    the supplied restart case is longer than that we silently ignore it.
  */
  if (restart_case) {
    if (strlen(restart_case) <= (SUMMARY_RESTART_SIZE * ECL_STRING8_LENGTH)) {
      ecl_smspec->restart_case = restart_case;
      ecl_smspec->restart_step = restart_step;
    }
  }
  ecl_smspec->grid_dims[0] = nx;
  ecl_smspec->grid_dims[1] = ny;
  ecl_smspec->grid_dims[2] = nz;
  ecl_smspec->sim_start_time = sim_start;

  {
    const ecl::smspec_node * time_node;

    if (time_in_days) {
      ecl_smspec->time_seconds = 3600 * 24;
      time_node = ecl_smspec_add_node(ecl_smspec, "TIME", "DAYS", 0);
    } else {
      ecl_smspec->time_seconds = 3600;
      time_node = ecl_smspec_add_node(ecl_smspec, "TIME", "HOURS", 0);
    }
    ecl_smspec->time_index = time_node->get_params_index();
  }
  return ecl_smspec;
}

ecl_smspec_type * ecl_smspec_alloc_restart_writer( const char * key_join_string , const char * restart_case, int restart_step, time_t sim_start , bool time_in_days , int nx , int ny , int nz) {
  return ecl_smspec_alloc_writer__(key_join_string, restart_case, restart_step, sim_start, time_in_days, nx, ny, nz);
}

ecl_smspec_type * ecl_smspec_alloc_writer(const char * key_join_string, time_t sim_start, bool time_in_days, int nx, int ny , int nz) {
  return ecl_smspec_alloc_writer__(key_join_string, NULL, 0, sim_start, time_in_days, nx, ny, nz);
}


UTIL_SAFE_CAST_FUNCTION( ecl_smspec , ECL_SMSPEC_ID )

ecl_smspec_var_type  ecl_smspec_identify_var_type(const char * var) {
  return ecl::smspec_node::identify_var_type(var);
}


static bool ecl_smspec_lgr_var_type( ecl_smspec_var_type var_type) {
  if ((var_type == ECL_SMSPEC_LOCAL_BLOCK_VAR) ||
      (var_type == ECL_SMSPEC_LOCAL_WELL_VAR) ||
      (var_type == ECL_SMSPEC_LOCAL_COMPLETION_VAR))

    return true;
  else
    return false;
}


/**
   Takes a ecl_smspec_var_type variable as input, and return a string
   representation of this var_type. Suitable for debug messages +++
*/

const char * ecl_smspec_get_var_type_name( ecl_smspec_var_type var_type ) {
  switch(var_type) {
  case(ECL_SMSPEC_INVALID_VAR):
    return "INVALID_VAR";
    break;
  case(ECL_SMSPEC_AQUIFER_VAR):
    return "AQUIFER_VAR";
    break;
  case(ECL_SMSPEC_WELL_VAR):
    return "WELL_VAR";
    break;
  case(ECL_SMSPEC_REGION_VAR):
    return "REGION_VAR";
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    return "FIELD_VAR";
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    return "GROUP_VAR";
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    return "BLOCK_VAR";
    break;
  case(ECL_SMSPEC_COMPLETION_VAR):
    return "COMPLETION_VAR";
    break;
  case(ECL_SMSPEC_LOCAL_BLOCK_VAR):
    return "LOCAL_BLOCK_VAR";
    break;
  case(ECL_SMSPEC_LOCAL_COMPLETION_VAR):
    return "LOCAL_COMPLETION_VAR";
    break;
  case(ECL_SMSPEC_LOCAL_WELL_VAR):
    return "LOCAL_WELL_VAR";
    break;
  case(ECL_SMSPEC_NETWORK_VAR):
    return "NETWORK_VAR";
    break;
  case(ECL_SMSPEC_REGION_2_REGION_VAR):
    return "REGION_2_REGION_VAR";
    break;
  case(ECL_SMSPEC_SEGMENT_VAR):
    return "SEGMENT_VAR";
    break;
  case(ECL_SMSPEC_MISC_VAR):
    return "MISC_VAR";
    break;
  default:
    util_abort("%s: Unrecognized variable type:%d \n",__func__ , var_type);
    return NULL;
  }
}






/**
  Input i,j,k are assumed to be in the interval [1..nx] , [1..ny],
  [1..nz], return value is a global index which can be used in the
  xxx_block_xxx routines.
*/


static int ecl_smspec_get_global_grid_index(const ecl_smspec_type * smspec , int i , int j , int k) {
  return i + (j - 1) * smspec->grid_dims[0] + (k - 1) * smspec->grid_dims[0] * smspec->grid_dims[1];
}




/**
   This function takes a fully initialized smspec_node instance, generates the
   corresponding key and inserts smspec_node instance in the main hash table
   smspec->gen_var_index.

   The format strings used, i.e. VAR:WELL for well based variables is implicitly
   defined through the format strings used in this function.
*/

static void ecl_smspec_install_gen_keys( ecl_smspec_type * smspec , const ecl::smspec_node& smspec_node ) {
  /* Insert the default general mapping. */
  {
    const char * gen_key1 = smspec_node.get_gen_key1();
    if (gen_key1)
      smspec->gen_var_index[gen_key1] = &smspec_node;
  }

  /* Insert the (optional) extra mapping for block related variables and region_2_region variables: */
  {
    const char * gen_key2 = smspec_node.get_gen_key2();
    if (gen_key2)
      smspec->gen_var_index[gen_key2] = &smspec_node;
  }
}

static void ecl_smspec_install_special_keys( ecl_smspec_type * ecl_smspec , const ecl::smspec_node& smspec_node) {
  /**
      This large switch is for installing keys which have custom lookup
      paths, in addition to the lookup based on general keys. Examples
      of this is e.g. well variables which can be looked up through:

      ecl_smspec_get_well_var_index( smspec , well_name , var );
  */

  const char * well            = smspec_node_get_wgname( &smspec_node );
  const char * group           = well;
  const int num                = smspec_node_get_num(&smspec_node);
  const char * keyword         = smspec_node_get_keyword(&smspec_node);
  ecl_smspec_var_type var_type = smspec_node_get_var_type(&smspec_node );

  switch(var_type) {
  case(ECL_SMSPEC_COMPLETION_VAR):
    ecl_smspec->well_completion_var_index[well][num][keyword] = &smspec_node;
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    ecl_smspec->field_var_index[keyword] = &smspec_node;
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    ecl_smspec->group_var_index[group][keyword] = &smspec_node;
    break;
  case(ECL_SMSPEC_REGION_VAR):
    ecl_smspec->region_var_index[num][keyword] = &smspec_node;
    ecl_smspec->num_regions = util_int_max(ecl_smspec->num_regions , num);
    break;
  case (ECL_SMSPEC_WELL_VAR):
    ecl_smspec->well_var_index[well][keyword] = &smspec_node;
    break;
  case(ECL_SMSPEC_MISC_VAR):
    /* Misc variable - i.e. date or CPU time ... */
    ecl_smspec->misc_var_index[keyword] = &smspec_node;
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    ecl_smspec->block_var_index[num][keyword] = &smspec_node;
    break;
    /**
        The variables below are ONLY accesable through the gen_key
        setup; but the must be mentioned in this switch statement,
        otherwise they will induce a hard failure in the default: target
        below.
    */
  case(ECL_SMSPEC_LOCAL_BLOCK_VAR):
    break;
  case(ECL_SMSPEC_LOCAL_COMPLETION_VAR):
    break;
  case(ECL_SMSPEC_LOCAL_WELL_VAR):
    break;
  case(ECL_SMSPEC_SEGMENT_VAR):
    break;
  case(ECL_SMSPEC_REGION_2_REGION_VAR):
    break;
  case(ECL_SMSPEC_AQUIFER_VAR):
    break;
  default:
    throw std::invalid_argument("Internal error - should not be here \n");
  }
}

/**
   The usage of this functon breaks down completely if LGR's are involved.
*/

bool ecl_smspec_needs_wgname( ecl_smspec_var_type var_type ) {
  switch( var_type ) {
  case(ECL_SMSPEC_COMPLETION_VAR):
    return true;
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    return false;
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    return true;
    break;
  case(ECL_SMSPEC_WELL_VAR):
    return true;
    break;
  case(ECL_SMSPEC_REGION_VAR):
    return false;
    break;
  case(ECL_SMSPEC_REGION_2_REGION_VAR):
    return false;
    break;
  case(ECL_SMSPEC_MISC_VAR):
    return false;
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    return false;
    break;
  case(ECL_SMSPEC_AQUIFER_VAR):
    return false;
    break;
  case(ECL_SMSPEC_SEGMENT_VAR):
    return true;
    break;
  default:
    util_exit("Sorry: support for variables of type:%s is not implemented in %s.\n",ecl_smspec_get_var_type_name( var_type ), __FILE__);
  }
  /* Really should not be here. */
  return false;
}


/**
   The usage of this functon breaks down completely if LGR's are involved.
*/
bool ecl_smspec_needs_num( ecl_smspec_var_type var_type ) {
  switch( var_type ) {
  case(ECL_SMSPEC_COMPLETION_VAR):
    return true;
    break;
  case(ECL_SMSPEC_AQUIFER_VAR):
    return true;
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    return false;
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    return false;
    break;
  case(ECL_SMSPEC_WELL_VAR):
    return false;
    break;
  case(ECL_SMSPEC_REGION_VAR):
    return true;
    break;
  case(ECL_SMSPEC_REGION_2_REGION_VAR):
    return true;
    break;
  case(ECL_SMSPEC_MISC_VAR):
    return false;
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    return true;
    break;
  default:
    util_exit("Sorry: support for variables of type:%s is not implemented in %s.\n",ecl_smspec_get_var_type_name( var_type ), __FILE__);
  }
  return false;
}




bool ecl_smspec_equal(const ecl_smspec_type * self,
                      const ecl_smspec_type * other) {
  if (self->smspec_nodes.size() != other->smspec_nodes.size())
    return false;

  for (size_t i=0; i < self->smspec_nodes.size(); i++) {
    const ecl::smspec_node* node1 = self->smspec_nodes[i].get();
    const ecl::smspec_node* node2 = other->smspec_nodes[i].get();

    if (node1->cmp(*node2) != 0)
      return false;
  }

  return true;
}




static void ecl_smspec_load_restart( ecl_smspec_type * ecl_smspec , const ecl_file_type * header ) {
  if (ecl_file_has_kw( header , RESTART_KW )) {
    const ecl_kw_type * restart_kw = ecl_file_iget_named_kw(header, RESTART_KW , 0);
    char   tmp_base[73];   /* To accomodate a maximum of 9 items which consist of 8 characters each. */
    char * restart_base;
    int i;
    tmp_base[0] = '\0';
    for (i=0; i < ecl_kw_get_size( restart_kw ); i++)
      strcat( tmp_base , (const char*)ecl_kw_iget_ptr( restart_kw , i ));

    restart_base = util_alloc_strip_copy( tmp_base );
    if (strlen(restart_base)) {  /* We ignore the empty ones. */
      char * smspec_header;

      /*
        The conditional block here is to support the following situation:

           1. A simulation with a restart has been performed on Posix with path
              separator '/'.

           2. The simulation is loaded on windows, where the native path
              separator is '\'.

        This code block will translate '/' -> '\' in the restart keyword which
        is read from the summary file.
      */

#ifdef ERT_WINDOWS
      for (int i=0; i < strlen(restart_base); i++) {
        if (restart_base[i] == UTIL_POSIX_PATH_SEP_CHAR)
          restart_base[i] = UTIL_PATH_SEP_CHAR;
      }
#endif

      std::string path = ecl::util::path::dirname( ecl_smspec->header_file );
      smspec_header = ecl_util_alloc_exfilename( path.c_str() , restart_base , ECL_SUMMARY_HEADER_FILE , ecl_smspec->formatted , 0);
      if (smspec_header) {
        if (!util_same_file(smspec_header , ecl_smspec->header_file.c_str()))    /* Restart from the current case is ignored. */ {
          if (util_is_abs_path(restart_base))
            ecl_smspec->restart_case = restart_base;
          else {
            char * tmp_path = util_alloc_filename( path.c_str() , restart_base , NULL );
            char * abs_path = util_alloc_abs_path(tmp_path);
            ecl_smspec->restart_case = abs_path;
            free( abs_path );
            free( tmp_path );
          }
        }
        free( smspec_header );
      }
    }
    free( restart_base );
  }
}








static const ecl::smspec_node * ecl_smspec_insert_node(ecl_smspec_type * ecl_smspec, std::unique_ptr<ecl::smspec_node> smspec_node){
  int params_index = smspec_node->get_params_index();

  /* This indexing must be used when writing. */
  ecl_smspec->index_map.push_back(params_index);
  ecl_smspec->params_default.resize( params_index+1, PARAMS_GLOBAL_DEFAULT );
  ecl_smspec->params_default[params_index] = smspec_node->get_default();
  ecl_smspec->inv_index_map.insert( std::make_pair(params_index, ecl_smspec->smspec_nodes.size()));

  ecl_smspec_install_gen_keys( ecl_smspec, *smspec_node.get() );
  ecl_smspec_install_special_keys( ecl_smspec, *smspec_node.get() );

  if (smspec_node->need_nums())
    ecl_smspec->need_nums = true;

  ecl_smspec->smspec_nodes.push_back(std::move(smspec_node));

  if (params_index > ecl_smspec->params_size)
    ecl_smspec->params_size = params_index + 1;

  if (static_cast<int>(ecl_smspec->smspec_nodes.size()) > ecl_smspec->params_size)
    ecl_smspec->params_size = ecl_smspec->smspec_nodes.size();

  const auto& node = ecl_smspec->smspec_nodes.back();
  return node.get();
}


const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec, const char * keyword, int num, const char * unit, float default_value) {
  int params_index = ecl_smspec->smspec_nodes.size();
  return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                              keyword,
                                                                                                              num,
                                                                                                              unit,
                                                                                                              ecl_smspec->grid_dims,
                                                                                                              default_value,
                                                                                                              ecl_smspec->key_join_string.c_str())));
}

//copy given node with a new index
const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec, const ecl::smspec_node& node) {
    int params_index = ecl_smspec->smspec_nodes.size();
    return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(node, params_index)));
}


const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec, const char * keyword, const char * unit, float default_value) {
  int params_index = ecl_smspec->smspec_nodes.size();
  return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                              keyword,
                                                                                                              unit,
                                                                                                              default_value)));
}


const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec, const char * keyword, const char * wgname, const char * unit, float default_value) {
  int params_index = ecl_smspec->smspec_nodes.size();
  return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                              keyword,
                                                                                                              wgname,
                                                                                                              unit,
                                                                                                              default_value,
                                                                                                              ecl_smspec->key_join_string.c_str())));
}


const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec,
                                                  const char * keyword,
                                                  const char * wgname,
                                                  int num,
                                                  const char * unit,
                                                  float default_value)
{
  int params_index = ecl_smspec->smspec_nodes.size();
  return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                              keyword,
                                                                                                              wgname,
                                                                                                              num,
                                                                                                              unit,
                                                                                                              ecl_smspec->grid_dims,
                                                                                                              default_value,
                                                                                                              ecl_smspec->key_join_string.c_str())));
}


const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec,
                                                  int params_index,
                                                  const char * keyword,
                                                  const char * wgname,
                                                  int num,
                                                  const char * unit,
                                                  float default_value)
{
  return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                              keyword,
                                                                                                              wgname,
                                                                                                              num,
                                                                                                              unit,
                                                                                                              ecl_smspec->grid_dims,
                                                                                                              default_value,
                                                                                                              ecl_smspec->key_join_string.c_str())));
}

const ecl::smspec_node * ecl_smspec_add_node(ecl_smspec_type * ecl_smspec,
                                                  int params_index,
                                                  const char * keyword,
                                                  const char * wgname,
                                                  int num,
                                                  const char * unit,
                                                  const char * lgr,
                                                  int lgr_i, int lgr_j, int lgr_k,
                                                  float default_value)
{
  return ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                              keyword,
                                                                                                              wgname,
                                                                                                              unit,
                                                                                                              lgr,
                                                                                                              lgr_i, lgr_j, lgr_k,
                                                                                                              default_value,
                                                                                                              ecl_smspec->key_join_string.c_str())));
}




const int * ecl_smspec_get_index_map( const ecl_smspec_type * smspec ) {
  return smspec->index_map.data();
}

/**
 * This function is to support the NAMES alias for WGNAMES. If similar
 * situations occur in the future, this is a sane starting point for general
 * support.
 */
static const char * get_active_keyword_alias(ecl_file_type * header, const char * keyword) {
  if (strcmp(keyword, WGNAMES_KW) == 0 || strcmp(keyword, NAMES_KW) == 0)
    return ecl_file_has_kw(header, WGNAMES_KW) ? WGNAMES_KW : NAMES_KW;

  return keyword;
}

static bool ecl_smspec_check_header( ecl_file_type * header ) {
  bool OK = true;
  for (size_t i=0; i < num_req_keywords && OK; i++) {
    OK &= ecl_file_has_kw(
            header,
            get_active_keyword_alias(header, smspec_required_keywords[i])
            );
  }

  return OK;
}


static bool ecl_smspec_fread_header(ecl_smspec_type * ecl_smspec, const char * header_file , bool include_restart) {
  ecl_file_type * header = ecl_file_open( header_file , 0);
  if (header && ecl_smspec_check_header( header )) {
    const char * names_alias = get_active_keyword_alias(header, WGNAMES_KW);
    ecl_kw_type *wells       = ecl_file_iget_named_kw(header, names_alias , 0);
    ecl_kw_type *keywords    = ecl_file_iget_named_kw(header, KEYWORDS_KW , 0);
    ecl_kw_type *startdat    = ecl_file_iget_named_kw(header, STARTDAT_KW , 0);
    ecl_kw_type *units       = ecl_file_iget_named_kw(header, UNITS_KW    , 0);
    ecl_kw_type *dimens      = ecl_file_iget_named_kw(header, DIMENS_KW   , 0);
    ecl_kw_type *nums        = NULL;
    ecl_kw_type *lgrs        = NULL;
    ecl_kw_type *numlx       = NULL;
    ecl_kw_type *numly       = NULL;
    ecl_kw_type *numlz       = NULL;

    int params_index;
    ecl_smspec->num_regions     = 0;
    ecl_smspec->params_size     = ecl_kw_get_size(keywords);
    if (startdat == NULL)
      util_abort("%s: could not locate STARTDAT keyword in header - aborting \n",__func__);

    if (ecl_file_has_kw(header , NUMS_KW))
      nums = ecl_file_iget_named_kw(header , NUMS_KW , 0);

    if (ecl_file_has_kw(header, INTEHEAD_KW)) {
      const ecl_kw_type * intehead = ecl_file_iget_named_kw(header, INTEHEAD_KW, 0);
      ecl_smspec->unit_system = (ert_ecl_unit_enum)ecl_kw_iget_int(intehead, INTEHEAD_SMSPEC_UNIT_INDEX);
      /*
        The second item in the INTEHEAD vector is an integer designating which
        simulator has been used for the current simulation, that is currently
        ignored.
      */
    }

    if (ecl_file_has_kw( header , LGRS_KW )) {/* The file has LGR information. */
      lgrs  = ecl_file_iget_named_kw( header , LGRS_KW  , 0 );
      numlx = ecl_file_iget_named_kw( header , NUMLX_KW , 0 );
      numly = ecl_file_iget_named_kw( header , NUMLY_KW , 0 );
      numlz = ecl_file_iget_named_kw( header , NUMLZ_KW , 0 );
      ecl_smspec->has_lgr = true;
    } else
      ecl_smspec->has_lgr = false;

    {
      int * date = ecl_kw_get_int_ptr(startdat);
      ecl_smspec->sim_start_time = ecl_util_make_date(date[STARTDAT_DAY_INDEX]   ,
          date[STARTDAT_MONTH_INDEX] ,
          date[STARTDAT_YEAR_INDEX]);
    }

    ecl_smspec->grid_dims[0] = ecl_kw_iget_int(dimens , DIMENS_SMSPEC_NX_INDEX );
    ecl_smspec->grid_dims[1] = ecl_kw_iget_int(dimens , DIMENS_SMSPEC_NY_INDEX );
    ecl_smspec->grid_dims[2] = ecl_kw_iget_int(dimens , DIMENS_SMSPEC_NZ_INDEX );
    ecl_smspec->restart_step = ecl_kw_iget_int(dimens , DIMENS_SMSPEC_RESTART_STEP_INDEX);

    ecl_util_get_file_type( header_file , &ecl_smspec->formatted , NULL );

    {
      for (params_index=0; params_index < ecl_kw_get_size(wells); params_index++) {
        float default_value          = PARAMS_GLOBAL_DEFAULT;
        int num                      = SMSPEC_NUMS_INVALID;
        char * well                  = (char*)util_alloc_strip_copy((const char*)ecl_kw_iget_ptr(wells    , params_index));
        char * kw                    = (char*)util_alloc_strip_copy((const char*)ecl_kw_iget_ptr(keywords , params_index));
        char * unit                  = (char*)util_alloc_strip_copy((const char*)ecl_kw_iget_ptr(units    , params_index));

        ecl_smspec_var_type var_type;
        if (nums != NULL) num        = ecl_kw_iget_int(nums , params_index);
        var_type = ecl::smspec_node::valid_type(kw, well, num);
        if (var_type == ECL_SMSPEC_INVALID_VAR) {
          free( kw );
          free( well );
          free( unit );
          continue;
        }

        if (ecl_smspec_lgr_var_type( var_type )) {
          int lgr_i = ecl_kw_iget_int( numlx , params_index );
          int lgr_j = ecl_kw_iget_int( numly , params_index );
          int lgr_k = ecl_kw_iget_int( numlz , params_index );
          char * lgr_name  = (char*)util_alloc_strip_copy(  (const char*)ecl_kw_iget_ptr( lgrs , params_index ));

          ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                               kw,
                                                                                                               well,
                                                                                                               unit,
                                                                                                               lgr_name,
                                                                                                               lgr_i, lgr_j, lgr_k,
                                                                                                               default_value,
                                                                                                               ecl_smspec->key_join_string.c_str())));
          free(lgr_name);
        } else
          ecl_smspec_insert_node(ecl_smspec, std::unique_ptr<ecl::smspec_node>( new ecl::smspec_node(params_index,
                                                                                                               kw,
                                                                                                               well,
                                                                                                               num,
                                                                                                               unit,
                                                                                                               ecl_smspec->grid_dims,
                                                                                                               default_value,
                                                                                                               ecl_smspec->key_join_string.c_str())));

        free( kw );
        free( well );
        free( unit );
      }
    }

    char * header_str = util_alloc_realpath( header_file );
    ecl_smspec->header_file = header_str;
    free(header_str);
    if (include_restart)
      ecl_smspec_load_restart( ecl_smspec , header );

    ecl_file_close( header );

    return true;
  } else
    return false;
}



ecl_smspec_type * ecl_smspec_fread_alloc(const char *header_file, const char * key_join_string , bool include_restart) {
  ecl_smspec_type *ecl_smspec;

  {
    char *path;
    util_alloc_file_components(header_file , &path , NULL , NULL);
    ecl_smspec = ecl_smspec_alloc_empty(false , key_join_string);
    free(path);
  }

  if (ecl_smspec_fread_header(ecl_smspec , header_file , include_restart)) {

    const ecl::smspec_node * time_node = ecl_smspec_get_var_node(ecl_smspec->misc_var_index, "TIME");
    if (time_node) {
      const char * time_unit = time_node->get_unit();
      ecl_smspec->time_index = time_node->get_params_index();

      if (util_string_equal( time_unit , "DAYS"))
        ecl_smspec->time_seconds = 3600 * 24;
      else if (util_string_equal( time_unit , "HOURS"))
        ecl_smspec->time_seconds = 3600;
      else
        util_abort("%s: time_unit:%s not recognized \n",__func__ , time_unit);

    }

    const ecl::smspec_node * day_node = ecl_smspec_get_var_node(ecl_smspec->misc_var_index, "DAY");
    if (day_node != NULL) {
      ecl_smspec->day_index   = smspec_node_get_params_index( day_node );
      ecl_smspec->month_index = smspec_node_get_params_index( &ecl_smspec->misc_var_index["MONTH"] );
      ecl_smspec->year_index  = smspec_node_get_params_index( &ecl_smspec->misc_var_index["YEAR"] );
    }

    if ((ecl_smspec->time_index == -1) && ( ecl_smspec->day_index == -1)) {
      /* Unusable configuration.

         Seems the ECLIPSE file can also have time specified with
         'YEARS' as basic time unit; that mode is not supported.
      */

      util_abort("%s: Sorry the SMSPEC file seems to lack all time information, need either TIME, or DAY/MONTH/YEAR information. Can not proceed.",__func__);
      return NULL;
    }
    return ecl_smspec;
  } else {
    /** Failed to load from disk. */
    ecl_smspec_free( ecl_smspec );
    return NULL;
  }
}


int ecl_smspec_get_num_groups(const ecl_smspec_type * ecl_smspec) {
  return ecl_smspec->group_var_index.size();
}


/*char ** ecl_smspec_alloc_group_names(const ecl_smspec_type * ecl_smspec) {
  return hash_alloc_keylist(ecl_smspec->group_var_index);
}*/

int ecl_smspec_get_num_regions(const ecl_smspec_type * ecl_smspec) {
  return ecl_smspec->num_regions;
}



/******************************************************************/
/*
   For each type of summary data (according to the types in
   ecl_smcspec_var_type there are a set accessor functions:

   xx_get_xx: This function will take the apropriate input, and
   return a double value. The function will fail with util_abort()
   if the ecl_smspec object can not recognize the input. THis
   function is not here.

   xxx_has_xx: Ths will return true / false depending on whether the
   ecl_smspec object the variable we ask for.

   xxx_get_xxx_index: This function will rerturn an (internal)
   integer index of where the variable in question is stored, this
   index can then be subsequently used for faster lookup. If the
   variable can not be found, the function will return -1.

   In general the index function is the real function, the others are
   only wrappers around this. In addition there are specialized
   functions, like get_well_names() and so on.
*/


/*****************************************************************/

#define NODE_RETURN_INDEX(node_ptr)                     \
  if (!node_ptr)                                        \
    return -1;                                          \
  else                                                \
    return smspec_node_get_params_index( node_ptr );


#define NODE_RETURN_EXISTS(node_ptr)            \
  if (!node_ptr)                                \
    return false;                               \
  else                                          \
    return true;



/******************************************************************/
/* Well variables */

const ecl::smspec_node& ecl_smspec_get_well_var_node( const ecl_smspec_type * smspec , const char * well , const char * var) {
  const auto node_ptr = ecl_smspec_get_str_key_var_node(smspec->well_var_index, well, var);
  if (!node_ptr)
    throw std::out_of_range("The well: " + std::string(well) + " variable: " + std::string(var) + " combination does not exist.");

  return *node_ptr;
}


int ecl_smspec_get_well_var_params_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var) {
  const auto node_ptr = ecl_smspec_get_str_key_var_node(ecl_smspec->well_var_index, well, var);
  NODE_RETURN_INDEX(node_ptr);
}


bool ecl_smspec_has_well_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var) {
  const auto node_ptr = ecl_smspec_get_str_key_var_node(ecl_smspec->well_var_index, well, var);
  NODE_RETURN_EXISTS(node_ptr);
}



/*****************************************************************/
/* Group variables */

const ecl::smspec_node& ecl_smspec_get_group_var_node( const ecl_smspec_type * smspec , const char * group , const char * var) {
  const auto node_ptr = ecl_smspec_get_str_key_var_node(smspec->group_var_index, group, var);
  if (!node_ptr)
    throw std::out_of_range("The group: " + std::string(group) + " variable: " + std::string(var) + " combination does not exist.");

  return *node_ptr;
}


int ecl_smspec_get_group_var_params_index(const ecl_smspec_type * ecl_smspec , const char * group , const char *var) {
  const auto node_ptr = ecl_smspec_get_str_key_var_node(ecl_smspec->group_var_index, group, var);
  NODE_RETURN_INDEX(node_ptr);
}


bool ecl_smspec_has_group_var(const ecl_smspec_type * ecl_smspec , const char * group , const char *var) {
  const auto node_ptr = ecl_smspec_get_str_key_var_node(ecl_smspec->group_var_index, group, var);
  NODE_RETURN_EXISTS(node_ptr);
}


/*****************************************************************/
/* Field variables */

const ecl::smspec_node& ecl_smspec_get_field_var_node(const ecl_smspec_type * ecl_smspec , const char *var) {
  const auto node_ptr = ecl_smspec_get_var_node( ecl_smspec->field_var_index, var);
  if (!node_ptr)
    throw std::out_of_range("The field variable: " + std::string(var) + " does not exist.");

  return *node_ptr;
}


int ecl_smspec_get_field_var_params_index(const ecl_smspec_type * ecl_smspec , const char *var) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->field_var_index, var);
  NODE_RETURN_INDEX(node_ptr);
}




bool ecl_smspec_has_field_var(const ecl_smspec_type * ecl_smspec , const char *var) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->field_var_index, var);
  NODE_RETURN_EXISTS(node_ptr);
}

/*****************************************************************/
/* Block variables */

/**
   Observe that block_nr is represented as char literal,
   i.e. "2345". This is because it will be used as a hash key.

   This is the final low level function which actually consults the
   hash tables.
*/


const ecl::smspec_node& ecl_smspec_get_block_var_node(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr) {
  const auto node_ptr = ecl_smspec_get_int_key_var_node(ecl_smspec->block_var_index, block_nr, block_var);
  if (!node_ptr)
    throw std::out_of_range("No such block variable");

  return *node_ptr;
}


const ecl::smspec_node& ecl_smspec_get_block_var_node_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k) {
  return ecl_smspec_get_block_var_node( ecl_smspec , block_var , ecl_smspec_get_global_grid_index( ecl_smspec , i,j,k) );
}


bool ecl_smspec_has_block_var(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr) {
  const auto node_ptr = ecl_smspec_get_int_key_var_node(ecl_smspec->block_var_index, block_nr, block_var);
  NODE_RETURN_EXISTS(node_ptr);
}


bool ecl_smspec_has_block_var_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k) {
  return ecl_smspec_has_block_var(ecl_smspec, block_var, ecl_smspec_get_global_grid_index(ecl_smspec, i, j, k));
}


int ecl_smspec_get_block_var_params_index(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr) {
  const auto node_ptr = ecl_smspec_get_int_key_var_node(ecl_smspec->block_var_index, block_nr, block_var);
  NODE_RETURN_INDEX(node_ptr);
}


int ecl_smspec_get_block_var_params_index_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k) {
  return ecl_smspec_get_block_var_params_index(ecl_smspec, block_var, ecl_smspec_get_global_grid_index(ecl_smspec, i, j, k));
}


/*****************************************************************/
/* Region variables */
/**
   region_nr: [1...num_regions] (NOT C-based indexing)
*/



const ecl::smspec_node& ecl_smspec_get_region_var_node(const ecl_smspec_type * ecl_smspec , const char *region_var , int region_nr) {
  const auto node_ptr = ecl_smspec_get_int_key_var_node(ecl_smspec->region_var_index, region_nr, region_var);
  if (!node_ptr)
    throw std::out_of_range("No such block variable");

  return *node_ptr;
}


bool ecl_smspec_has_region_var(const ecl_smspec_type * ecl_smspec , const char *region_var, int region_nr) {
  const auto node_ptr = ecl_smspec_get_int_key_var_node(ecl_smspec->region_var_index, region_nr, region_var);
  NODE_RETURN_EXISTS(node_ptr);
}


int ecl_smspec_get_region_var_params_index(const ecl_smspec_type * ecl_smspec , const char *region_var, int region_nr) {
  const auto node_ptr = ecl_smspec_get_int_key_var_node(ecl_smspec->region_var_index, region_nr, region_var);
  NODE_RETURN_INDEX(node_ptr);
}

/*****************************************************************/
/* Misc variables */

const ecl::smspec_node& ecl_smspec_get_misc_var_node(const ecl_smspec_type * ecl_smspec , const char *var) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->misc_var_index, var);
  if (!node_ptr)
    throw std::out_of_range("No such misc variable");

  return *node_ptr;
}


bool ecl_smspec_has_misc_var(const ecl_smspec_type * ecl_smspec , const char *var) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->misc_var_index , var );
  NODE_RETURN_EXISTS(node_ptr);
}

int ecl_smspec_get_misc_var_params_index(const ecl_smspec_type * ecl_smspec , const char *var) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->misc_var_index , var );
  NODE_RETURN_INDEX(node_ptr);
}


/*****************************************************************/
/* Well completion - not fully implemented ?? */


const ecl::smspec_node * ecl_smspec_get_well_completion_var_node__(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const auto well_iter = ecl_smspec->well_completion_var_index.find(well);
  if (well_iter == ecl_smspec->well_completion_var_index.end())
    return nullptr;

  const auto& num_map = well_iter->second;
  return ecl_smspec_get_int_key_var_node( num_map, cell_nr, var );
}

const ecl::smspec_node& ecl_smspec_get_well_completion_var_node(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const auto node_ptr = ecl_smspec_get_well_completion_var_node__(ecl_smspec, well, var, cell_nr);
  if (!node_ptr)
    throw std::out_of_range("No such well/var/completion");

  return *node_ptr;
}


bool  ecl_smspec_has_well_completion_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const auto node_ptr = ecl_smspec_get_well_completion_var_node__( ecl_smspec , well , var , cell_nr );
  NODE_RETURN_EXISTS( node_ptr );
}


int  ecl_smspec_get_well_completion_var_params_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const auto node_ptr = ecl_smspec_get_well_completion_var_node__( ecl_smspec , well , var , cell_nr );
  NODE_RETURN_INDEX( node_ptr );
}


/*****************************************************************/
/* General variables ... */


/* There is a quite wide range of error which are just returned as
   "Not found" (i.e. -1). */
/* Completions not supported yet. */



const ecl::smspec_node& ecl_smspec_get_general_var_node( const ecl_smspec_type * smspec , const char * lookup_kw ) {
  const auto node_ptr = ecl_smspec_get_var_node(smspec->gen_var_index, lookup_kw);
  if (!node_ptr)
    throw std::out_of_range("No such variable: " + std::string(lookup_kw));

  return *node_ptr;
}


int ecl_smspec_get_general_var_params_index(const ecl_smspec_type * ecl_smspec , const char * lookup_kw) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->gen_var_index , lookup_kw );
  NODE_RETURN_INDEX( node_ptr );
}


bool ecl_smspec_has_general_var(const ecl_smspec_type * ecl_smspec , const char * lookup_kw) {
  const auto node_ptr = ecl_smspec_get_var_node(ecl_smspec->gen_var_index , lookup_kw );
  NODE_RETURN_EXISTS( node_ptr );
}


/** DIES if the lookup_kw is not present. */
const char * ecl_smspec_get_general_var_unit( const ecl_smspec_type * ecl_smspec , const char * lookup_kw) {
  const auto smspec_node = ecl_smspec_get_general_var_node(ecl_smspec, lookup_kw);
  return smspec_node_get_unit( &smspec_node );
}


/*****************************************************************/
/*
   Pure indexed lookup - these functions can be used after one of the
   ecl_smspec_get_xxx_index() functions has been used first.
*/

//const char * ecl_smspec_iget_unit( const ecl_smspec_type * smspec , int node_index ) {
//  const ecl::smspec_node * smspec_node = ecl_smspec_iget_node( smspec , node_index );
//  return smspec_node_get_unit( smspec_node );
//}
//
//int ecl_smspec_iget_num( const ecl_smspec_type * smspec , int node_index ) {
//  const ecl::smspec_node * smspec_node = ecl_smspec_iget_node( smspec , node_index );
//  return smspec_node_get_num( smspec_node );
//}
//
//const char * ecl_smspec_iget_wgname( const ecl_smspec_type * smspec , int node_index ) {
//  const ecl::smspec_node * smspec_node = ecl_smspec_iget_node( smspec , node_index );
//  return smspec_node_get_wgname( smspec_node );
//}
//
//const char * ecl_smspec_iget_keyword( const ecl_smspec_type * smspec , int index ) {
//  const ecl::smspec_node * smspec_node = ecl_smspec_iget_node( smspec , index );
//  return smspec_node_get_keyword( smspec_node );
//}


/*****************************************************************/

int ecl_smspec_get_time_seconds( const ecl_smspec_type * ecl_smspec ) {
  return ecl_smspec->time_seconds;
}

int ecl_smspec_get_time_index( const ecl_smspec_type * ecl_smspec ) {
  return ecl_smspec->time_index;
}

time_t ecl_smspec_get_start_time(const ecl_smspec_type * ecl_smspec) {
  return ecl_smspec->sim_start_time;
}

bool ecl_smspec_get_formatted( const ecl_smspec_type * ecl_smspec) {
  return ecl_smspec->formatted;
}

const char * ecl_smspec_get_header_file( const ecl_smspec_type * ecl_smspec ) {
  return ecl_smspec->header_file.c_str();
}


int ecl_smspec_get_restart_step(const ecl_smspec_type * ecl_smspec) {
  return ecl_smspec->restart_step;
}

int ecl_smspec_get_first_step(const ecl_smspec_type * ecl_smspec) {
  if (ecl_smspec->restart_step > 0)
    return ecl_smspec->restart_step + 1;
  else
    return 1;
}


const char * ecl_smspec_get_restart_case( const ecl_smspec_type * ecl_smspec) {
  if (ecl_smspec->restart_case.size() > 0)
    return ecl_smspec->restart_case.c_str();
  else
    return NULL;
}

const std::vector<float>& ecl_smspec_get_params_default( const ecl_smspec_type * ecl_smspec ) {
  return ecl_smspec->params_default;
}

void ecl_smspec_free(ecl_smspec_type *ecl_smspec) {
  delete ecl_smspec;
}


void ecl_smspec_free__(void * __ecl_smspec) {
  ecl_smspec_type * ecl_smspec = ecl_smspec_safe_cast( __ecl_smspec);
  ecl_smspec_free( ecl_smspec );
}


int ecl_smspec_get_date_day_index( const ecl_smspec_type * smspec ) {
  return smspec->day_index;
}

int ecl_smspec_get_date_month_index( const ecl_smspec_type * smspec ) {
  return smspec->month_index;
}

int ecl_smspec_get_date_year_index( const ecl_smspec_type * smspec ) {
  return smspec->year_index;
}



/**
   This function checks whether an input general key (i.e. FWPR or
   GGPT:NORTH) represents an accumulated total. If the variable is not
   internalized the function will fail hard.
*/


bool ecl_smspec_general_is_total( const ecl_smspec_type * smspec , const char * gen_key) {
  const  ecl::smspec_node& smspec_node = ecl_smspec_get_general_var_node(smspec, gen_key);
  return smspec_node_is_total( &smspec_node );
}




/*****************************************************************/


/**
   Fills a stringlist instance with all the gen_key string matching
   the supplied pattern. I.e.

     ecl_smspec_alloc_matching_general_var_list( smspec , "WGOR:*");

   will give a list of WGOR for ALL the wells. The function is
   unfortunately not as useful as one might think because ECLIPSE is
   quite stupid; it will for instance happily give you the WOPR for a
   water injector or WWIR for an oil producer.

   The function can be called several times with different patterns,
   the stringlist is not cleared on startup; the keys in the list are
   unique - keys are not added multiple times. If pattern == NULL all
   keys will match.
*/


void ecl_smspec_select_matching_general_var_list( const ecl_smspec_type * smspec , const char * pattern , stringlist_type * keys) {
  std::set<std::string> ex_keys;
  for (int i=0; i < stringlist_get_size( keys ); i++)
    ex_keys.insert( stringlist_iget(keys, i));

  {
    for (const auto& pair : smspec->gen_var_index) {
      const char * key = pair.first.c_str();

      /*
         The TIME is typically special cased by output and will not
         match the 'all keys' wildcard.
      */
      if (util_string_equal( key , "TIME")) {
        if ((pattern == NULL) || (util_string_equal( pattern , "*")))
          continue;
      }


      if ((pattern == NULL) || (util_fnmatch( pattern , key ) == 0)) {
        if (ex_keys.find(key) == ex_keys.end())
          stringlist_append_copy( keys , key );
      }
    }
  }

  stringlist_sort( keys , (string_cmp_ftype *) util_strcmp_int );
}


/**
   Allocates a new stringlist and initializes it with the
   ecl_smspec_select_matching_general_var_list() function.
*/

stringlist_type * ecl_smspec_alloc_matching_general_var_list(const ecl_smspec_type * smspec , const char * pattern) {
  stringlist_type * keys = stringlist_alloc_new();
  ecl_smspec_select_matching_general_var_list( smspec , pattern , keys );
  return keys;
}



const char * ecl_smspec_get_join_string( const ecl_smspec_type * smspec) {
  return smspec->key_join_string.c_str();
}



/**
    Returns a stringlist instance with all the (valid) well names. It
    is the responsability of the calling scope to free the stringlist
    with stringlist_free();


    If @pattern is different from NULL only wells which 'match' the
    pattern is included; if @pattern == NULL all wells are
    included. The match is done with function fnmatch() -
    i.e. standard shell wildcards.
*/

static stringlist_type * ecl_smspec_alloc_map_list( const std::map<std::string, node_map>& mp , const char * pattern) {
  stringlist_type * map_list = stringlist_alloc_new( );

  for (const auto& pair : mp) {
    const char * map_name = pair.first.c_str();

    if (pattern == NULL)
      stringlist_append_copy( map_list , map_name );
    else if (util_fnmatch( pattern , map_name) == 0)
      stringlist_append_copy( map_list , map_name );

  }
  stringlist_sort( map_list , (string_cmp_ftype *) util_strcmp_int );
  return map_list;
}

stringlist_type * ecl_smspec_alloc_well_list( const ecl_smspec_type * smspec , const char * pattern) {
  return ecl_smspec_alloc_map_list( smspec->well_var_index, pattern );
}

/**
    Returns a stringlist instance with all the (valid) group names. It
    is the responsability of the calling scope to free the stringlist
    with stringlist_free();
*/

stringlist_type * ecl_smspec_alloc_group_list( const ecl_smspec_type * smspec , const char * pattern) {
  return ecl_smspec_alloc_map_list( smspec->group_var_index, pattern );
}



/**
    Returns a stringlist instance with all the well variables.  It is
    the responsability of the calling scope to free the stringlist
    with stringlist_free();
*/

stringlist_type * ecl_smspec_alloc_well_var_list( const ecl_smspec_type * smspec ) {
  stringlist_type * stringlist = stringlist_alloc_new();
  for (const auto& pair : smspec->well_var_index)
    stringlist_append_copy(stringlist, pair.first.c_str());

  return stringlist;
}





const int * ecl_smspec_get_grid_dims( const ecl_smspec_type * smspec ) {
  return smspec->grid_dims;
}




/*****************************************************************/

char * ecl_smspec_alloc_well_key( const ecl_smspec_type * smspec , const char * keyword , const char * wgname) {
  return smspec_alloc_well_key( smspec->key_join_string.c_str() , keyword , wgname );
}


/*void ecl_smspec_sort( ecl_smspec_type * smspec ) {
  std::sort(smspec->smspec_nodes.begin(), smspec->smspec_nodes.end(), smspec_node_lt);

  for (int i=0; i < static_cast<int>(smspec->smspec_nodes.size()); i++) {
    ecl::smspec_node& node = *smspec->smspec_nodes[i].get();
    smspec_node_set_params_index( &node , i );
  }
}
*/

ert_ecl_unit_enum ecl_smspec_get_unit_system(const ecl_smspec_type * smspec) {
  return smspec->unit_system;
}
