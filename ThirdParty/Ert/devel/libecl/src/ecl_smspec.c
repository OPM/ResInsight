/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
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

#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/float_vector.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_smspec.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/smspec_node.h>
#include <ert/ecl/ecl_endian_flip.h>

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
     smspec_node_type instance when called with the new var_type.

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



struct ecl_smspec_struct {
  UTIL_TYPE_ID_DECLARATION;
  /*
    All the hash tables listed below here are different ways to access
    smspec_node instances. The actual smspec_node instances are
    owned by the smspec_nodes vector;
  */
  hash_type          * well_var_index;             /* Indexes for all well variables: {well1: {var1: index1 , var2: index2} , well2: {var1: index1 , var2: index2}} */
  hash_type          * well_completion_var_index;  /* Indexes for completion indexes .*/
  hash_type          * group_var_index;            /* Indexes for group variables.*/
  hash_type          * field_var_index;               
  hash_type          * region_var_index;           /* The stored index is an offset. */
  hash_type          * misc_var_index;             /* Variables like 'TCPU' and 'NEWTON'. */
  hash_type          * block_var_index;            /* Block variables like BPR */ 
  hash_type          * gen_var_index;              /* This is "everything" - things can either be found as gen_var("WWCT:OP_X") or as well_var("WWCT" , "OP_X") */


  vector_type        * smspec_nodes;
  bool                 write_mode;
  bool                 need_nums;
  bool                 locked;
  int_vector_type    * index_map;

  /*-----------------------------------------------------------------*/

  int               grid_dims[3];                 /* Grid dimensions - in DIMENS[1,2,3] */ 
  int               num_regions;
  int               Nwells , param_offset;
  int               params_size;
  const char      * key_join_string;               /* The string used to join keys when building gen_key keys - typically ":" - 
                                                      but arbitrary - NOT necessary to be able to invert the joining. */
  char            * header_file;                   /* FULL path to the currenbtly loaded header_file. */

  bool                formatted;                     /* Has this summary instance been loaded from a formatted (i.e. FSMSPEC file) or unformatted (i.e. SMSPEC) file. */
  time_t              sim_start_time;                /* When did the simulation start - worldtime. */

  int                 time_index;                    /* The fields time_index, day_index, month_index and year_index */
  int                 day_index;                     /* are used by the ecl_sum_data object to locate per. timestep */  
  int                 month_index;                   /* time information. */ 
  int                 year_index;
  bool                has_lgr;
  float_vector_type * params_default;
  
  stringlist_type   * restart_list;                  /* List of ECLBASE names of restart files this case has been restarted from (if any). */ 
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


/**
   The special_vars list is used to associate keywords with special
   types, when the kewyord name is in conflict with the default vector
   naming convention; all the variables mentioned in the list below
   are given the type ECL_SMSPEC_MISC_VAR.
   
   For instance the keyword 'NEWTON' starts with 'N' and is
   classified as a NETWORK type variable. However it should rather
   be classified as a MISC type variable. (What a fucking mess).

   The special_vars list is used in the functions
   ecl_smspec_identify_special_var() and ecl_smspec_identify_var_type().
*/

static const char* special_vars[] = {"NEWTON",
                                     "NLINEARS",
                                     "ELAPSED",
                                     "MAXDPR",
                                     "MAXDSO",
                                     "MAXDSG",
                                     "MAXDSW",
                                     "STEPTYPE"};



/*****************************************************************/

static ecl_smspec_type * ecl_smspec_alloc_empty(bool write_mode , const char * key_join_string) {
  ecl_smspec_type *ecl_smspec;
  ecl_smspec = util_malloc(sizeof *ecl_smspec );
  UTIL_TYPE_ID_INIT(ecl_smspec , ECL_SMSPEC_ID);

  ecl_smspec->well_var_index                 = hash_alloc();
  ecl_smspec->well_completion_var_index      = hash_alloc();
  ecl_smspec->group_var_index                = hash_alloc();
  ecl_smspec->field_var_index                = hash_alloc();
  ecl_smspec->region_var_index               = hash_alloc();
  ecl_smspec->misc_var_index                 = hash_alloc();
  ecl_smspec->block_var_index                = hash_alloc();
  ecl_smspec->gen_var_index                  = hash_alloc();
  ecl_smspec->sim_start_time                 = -1;
  ecl_smspec->key_join_string                = key_join_string;
  ecl_smspec->header_file                    = NULL;
  
  ecl_smspec->smspec_nodes                   = vector_alloc_new();
  
  ecl_smspec->time_index  = -1;
  ecl_smspec->day_index   = -1;
  ecl_smspec->year_index  = -1;
  ecl_smspec->month_index = -1;
  ecl_smspec->locked      = false;
  
  ecl_smspec->index_map = int_vector_alloc(0,0);
  ecl_smspec->restart_list = stringlist_alloc_new();
  ecl_smspec->params_default = float_vector_alloc(0 , PARAMS_GLOBAL_DEFAULT);
  ecl_smspec->write_mode = write_mode;
  ecl_smspec->need_nums = false;

  return ecl_smspec;
}


/**
   Observe that the index here is into the __INTERNAL__ indexing in
   the smspec_nodes vector; and in general widely different from the
   params_index of the returned smspec_node instance.
*/


const smspec_node_type * ecl_smspec_iget_node( const ecl_smspec_type * smspec , int index ) {
  return vector_iget_const( smspec->smspec_nodes , index );
}

int ecl_smspec_num_nodes( const ecl_smspec_type * smspec) {
  return vector_get_size( smspec->smspec_nodes );
}


/*
  In the current implementation it is impossible to mix calls to
  ecl_sum_add_var() and ecl_sum_add_tstep() - i.e. one must first add
  *all* the variables with ecl_sum_add_var() calls, and then
  subsequently add timesteps with ecl_sum_add_tstep().

  The locked property of the smspec structure is to ensure that no new
  variables are added to the ecl_smspec structure after the first
  timestep has been added.
*/

void ecl_smspec_lock( ecl_smspec_type * smspec ) {
  smspec->locked = true;
}


// DIMENS
// KEYWORDS
// WGNAMES
// NUMS    - optional
// UNITS
// STARTDAT


static void ecl_smspec_fortio_fwrite( const ecl_smspec_type * smspec , fortio_type * fortio) {
  int num_nodes           = ecl_smspec_num_nodes( smspec );
  {
    ecl_kw_type * restart_kw = ecl_kw_alloc( RESTART_KW , SUMMARY_RESTART_SIZE , ECL_CHAR_TYPE );
    int i;
    for (i=0; i < SUMMARY_RESTART_SIZE; i++)
      ecl_kw_iset_string8( restart_kw , i , "");
    
    ecl_kw_fwrite( restart_kw , fortio );
    ecl_kw_free( restart_kw );
  }
  {
    ecl_kw_type * dimens_kw = ecl_kw_alloc( DIMENS_KW , DIMENS_SIZE , ECL_INT_TYPE );
    ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_SIZE_INDEX , num_nodes );
    ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_NX_INDEX   , smspec->grid_dims[0] );
    ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_NY_INDEX   , smspec->grid_dims[1] );
    ecl_kw_iset_int( dimens_kw , DIMENS_SMSPEC_NZ_INDEX   , smspec->grid_dims[2] );

    /* Do not know what these two last items are for. */
    ecl_kw_iset_int( dimens_kw , 4  , 0 );
    ecl_kw_iset_int( dimens_kw , 5 , -1 );

    ecl_kw_fwrite( dimens_kw , fortio );
    ecl_kw_free( dimens_kw );
  }


  {
    ecl_kw_type * keywords_kw = ecl_kw_alloc( KEYWORDS_KW , num_nodes , ECL_CHAR_TYPE );
    ecl_kw_type * wgnames_kw  = ecl_kw_alloc( WGNAMES_KW  , num_nodes , ECL_CHAR_TYPE );
    ecl_kw_type * units_kw    = ecl_kw_alloc( UNITS_KW    , num_nodes , ECL_CHAR_TYPE );
    ecl_kw_type * nums_kw     = NULL;
    
    if (smspec->need_nums)
      nums_kw = ecl_kw_alloc( NUMS_KW , num_nodes , ECL_INT_TYPE);
    {
      int i;
      for (i=0; i < ecl_smspec_num_nodes( smspec ); i++) {
        const smspec_node_type * smspec_node = ecl_smspec_iget_node( smspec , i );
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
        if (smspec_node_get_var_type( smspec_node ) == ECL_SMSPEC_INVALID_VAR) {
          ecl_kw_iset_string8( keywords_kw , i , "WWCT" );
          ecl_kw_iset_string8( units_kw , i , "????????");
          ecl_kw_iset_string8( wgnames_kw , i , DUMMY_WELL);
        } else {
          ecl_kw_iset_string8( keywords_kw , i , smspec_node_get_keyword( smspec_node ));
          ecl_kw_iset_string8( units_kw , i , smspec_node_get_unit( smspec_node ));
          {
            const char * wgname = DUMMY_WELL;
            if (smspec_node_get_wgname( smspec_node ) != NULL)
              wgname = smspec_node_get_wgname( smspec_node );
            ecl_kw_iset_string8( wgnames_kw , i , wgname);
          }
        }

        if (nums_kw != NULL)
          ecl_kw_iset_int( nums_kw , i , smspec_node_get_num( smspec_node ));
      }
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
  }

  {
    ecl_kw_type * startdat_kw = ecl_kw_alloc( STARTDAT_KW , STARTDAT_SIZE , ECL_INT_TYPE );
    int day,month,year;
    util_set_date_values( smspec->sim_start_time , &day, &month , &year);
    
    ecl_kw_iset_int( startdat_kw , STARTDAT_DAY_INDEX   , day );
    ecl_kw_iset_int( startdat_kw , STARTDAT_MONTH_INDEX , month );
    ecl_kw_iset_int( startdat_kw , STARTDAT_YEAR_INDEX  , year );
    
    ecl_kw_fwrite( startdat_kw , fortio );
    ecl_kw_free( startdat_kw );
  }
}


void ecl_smspec_fwrite( const ecl_smspec_type * smspec , const char * ecl_case , bool fmt_file ) {
  char * filename = ecl_util_alloc_filename( NULL , ecl_case , ECL_SUMMARY_HEADER_FILE , fmt_file , 0 );
  fortio_type * fortio = fortio_open_writer( filename , fmt_file , ECL_ENDIAN_FLIP);
  
  ecl_smspec_fortio_fwrite( smspec , fortio );
  
  fortio_fclose( fortio );
  free( filename );
}

ecl_smspec_type * ecl_smspec_alloc_writer( const char * key_join_string , time_t sim_start , int nx , int ny , int nz) {
  ecl_smspec_type * ecl_smspec = ecl_smspec_alloc_empty( true , key_join_string );
  
  ecl_smspec->grid_dims[0] = nx;
  ecl_smspec->grid_dims[1] = ny;
  ecl_smspec->grid_dims[2] = nz;
  ecl_smspec->sim_start_time = sim_start;
  
  {
    smspec_node_type * time_node = smspec_node_alloc( ECL_SMSPEC_MISC_VAR , 
                                                      NULL , 
                                                      "TIME" , 
                                                      "DAYS" , // There are significant hardcoded assumptions of days around; would have been cool to be able to set seconds.
                                                      key_join_string , 
                                                      ecl_smspec->grid_dims , 
                                                      0  , 
                                                      -1 ,    
                                                      0 );
    ecl_smspec_add_node( ecl_smspec , time_node );
    ecl_smspec->time_index = smspec_node_get_params_index( time_node );
  }
  return ecl_smspec;
}


UTIL_SAFE_CAST_FUNCTION( ecl_smspec , ECL_SMSPEC_ID )


/**
   Goes through the special_vars static table to check if @var is one
   the special variables which does not follow normal naming
   convention. If the test eavulates to true the function will return
   ECL_SMSPEC_MISC_VAR, otherwise the function will return
   ECL_SMSPEC_INVALID_VAR and the variable type will be determined
   from the var name according to standard naming conventions.

   It is important that this function is called before the standard
   method.
*/

static ecl_smspec_var_type ecl_smspec_identify_special_var( const char * var ) {
  ecl_smspec_var_type var_type = ECL_SMSPEC_INVALID_VAR;

  int num_special = sizeof( special_vars ) / sizeof( special_vars[0] );
  int i;
  for (i=0; i < num_special; i++) {
    if (strcmp( var , special_vars[i]) == 0) {
      var_type = ECL_SMSPEC_MISC_VAR;
      break;
    }
  }
  
  return var_type;
}


/* 
   See table 3.4 in the ECLIPSE file format reference manual.

   Observe that the combined ecl_sum style keys like e.g. WWCT:OP1
   should be formatted with the keyword first, so that this function
   will identify both 'WWCT' and 'WWCT:OP_1' as a ECL_SMSPEC_WELL_VAR
   instance.
*/


ecl_smspec_var_type  ecl_smspec_identify_var_type(const char * var) {
  ecl_smspec_var_type var_type = ecl_smspec_identify_special_var( var );
  int str_length = strlen(var); 
  if (var_type == ECL_SMSPEC_INVALID_VAR) {
    switch(var[0]) {
    case('A'):
      var_type = ECL_SMSPEC_AQUIFER_VAR;
      break;
    case('B'):
      var_type = ECL_SMSPEC_BLOCK_VAR;
      break;
    case('C'):
      var_type = ECL_SMSPEC_COMPLETION_VAR;
      break;
    case('F'):
      var_type = ECL_SMSPEC_FIELD_VAR;
      break;
    case('G'):
      var_type = ECL_SMSPEC_GROUP_VAR;
      break;
    case('L'):
      switch(var[1]) {
      case('B'):
        var_type = ECL_SMSPEC_LOCAL_BLOCK_VAR;
        break;
      case('C'):
        var_type = ECL_SMSPEC_LOCAL_COMPLETION_VAR;
        break;
      case('W'):
        var_type = ECL_SMSPEC_LOCAL_WELL_VAR;
        break;
      default:
        util_abort("%s: not recognized: %s \n",__func__ , var);
      }
      break;
    case('N'):
      var_type = ECL_SMSPEC_NETWORK_VAR;
      break;
    case('R'):
      if (((3 == str_length) && var[2] == 'F') || 
          ((4 == str_length) && var[3] == 'F')) {
       var_type  = ECL_SMSPEC_REGION_2_REGION_VAR;
      }
      else {
        var_type  = ECL_SMSPEC_REGION_VAR;
      }
      break;
    case('S'):
      var_type = ECL_SMSPEC_SEGMENT_VAR;
      break;
    case('W'):
      var_type = ECL_SMSPEC_WELL_VAR;
      break;
    default:
      /*
        It is unfortunately impossible to recognize an error situation -
        the rest just goes in "other" variables.
      */
      var_type = ECL_SMSPEC_MISC_VAR;
    }
  }
  return var_type;
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

static void ecl_smspec_install_gen_keys( ecl_smspec_type * smspec , smspec_node_type * smspec_node ) {
  /* Insert the default general mapping. */
  {
    const char * gen_key1 = smspec_node_get_gen_key1( smspec_node );
    if (gen_key1 != NULL)
      hash_insert_ref(smspec->gen_var_index , gen_key1 , smspec_node);
  }

  /* Insert the (optional) extra mapping for block related variables and region_2_region variables: */
  {
    const char * gen_key2 = smspec_node_get_gen_key2( smspec_node );
    if (gen_key2 != NULL) 
      hash_insert_ref(smspec->gen_var_index , gen_key2 , smspec_node);
  }
}

static void ecl_smspec_install_special_keys( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node) {
  /** 
      This large switch is for installing keys which have custom lookup
      paths, in addition to the lookup based on general keys. Examples
      of this is e.g. well variables which can be looked up through: 
      
      ecl_smspec_get_well_var_index( smspec , well_name , var );
  */

  const char * well            = smspec_node_get_wgname( smspec_node );
  const char * group           = well;
  const int num                = smspec_node_get_num(smspec_node);
  const char * keyword         = smspec_node_get_keyword(smspec_node);
  ecl_smspec_var_type var_type = smspec_node_get_var_type( smspec_node );
  
  switch(var_type) {
  case(ECL_SMSPEC_COMPLETION_VAR):
    /* Three level indexing: variable -> well -> string(cell_nr)*/
    if (!hash_has_key(ecl_smspec->well_completion_var_index , well))
      hash_insert_hash_owned_ref(ecl_smspec->well_completion_var_index , well , hash_alloc() , hash_free__);
    {
      hash_type * cell_hash = hash_get(ecl_smspec->well_completion_var_index , well);
      char cell_str[16];
      sprintf(cell_str , "%d" , num);
      if (!hash_has_key(cell_hash , cell_str))
        hash_insert_hash_owned_ref(cell_hash , cell_str , hash_alloc() , hash_free__);
      {
        hash_type * var_hash = hash_get(cell_hash , cell_str);
        hash_insert_ref(var_hash , keyword , smspec_node );
      }
    }
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    /*
      Field variable
    */
    hash_insert_ref( ecl_smspec->field_var_index , keyword , smspec_node );
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    if (!hash_has_key(ecl_smspec->group_var_index , group))
      hash_insert_hash_owned_ref(ecl_smspec->group_var_index , group, hash_alloc() , hash_free__);
    {
      hash_type * var_hash = hash_get(ecl_smspec->group_var_index , group);
      hash_insert_ref(var_hash , keyword , smspec_node );
    }
    break;
  case(ECL_SMSPEC_REGION_VAR):
    if (!hash_has_key(ecl_smspec->region_var_index , keyword)) 
      hash_insert_hash_owned_ref( ecl_smspec->region_var_index , keyword , hash_alloc() , hash_free__);
    {
      hash_type * var_hash = hash_get(ecl_smspec->region_var_index , keyword);
      char num_str[16];
      sprintf( num_str , "%d" , num);
      hash_insert_ref(var_hash , num_str , smspec_node);
    }
    ecl_smspec->num_regions = util_int_max(ecl_smspec->num_regions , num);
    break;
  case (ECL_SMSPEC_WELL_VAR):
    if (!hash_has_key(ecl_smspec->well_var_index , well))
      hash_insert_hash_owned_ref(ecl_smspec->well_var_index , well , hash_alloc() , hash_free__);
    {
      hash_type * var_hash = hash_get(ecl_smspec->well_var_index , well);
      hash_insert_ref(var_hash , keyword , smspec_node );
    }
    break;
  case(ECL_SMSPEC_MISC_VAR):
    /* Misc variable - i.e. date or CPU time ... */
    hash_insert_ref(ecl_smspec->misc_var_index , keyword , smspec_node );
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    /* A block variable */
    if (!hash_has_key(ecl_smspec->block_var_index , keyword))
      hash_insert_hash_owned_ref(ecl_smspec->block_var_index , keyword , hash_alloc() , hash_free__);
    {
      hash_type * block_hash = hash_get(ecl_smspec->block_var_index , keyword);
      char block_nr[16];
      sprintf( block_nr , "%d" , num );
      hash_insert_ref(block_hash , block_nr , smspec_node);
    }
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
  default:
    util_abort("%: Internal error - should never be here ?? \n",__func__);
    break;
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


static bool ecl_smspec_kw_equal(const ecl_file_type * header , const ecl_file_type * restart_header , const char * kw , int cmp_elements) {
  if (ecl_file_has_kw( header , kw ) == ecl_file_has_kw( restart_header , kw )) {
    if (ecl_file_has_kw( header , kw)) {
      ecl_kw_type *ecl_kw1  = ecl_file_iget_named_kw(header, kw , 0);
      ecl_kw_type *ecl_kw2  = ecl_file_iget_named_kw(restart_header, kw , 0);
      
      return ecl_kw_block_equal( ecl_kw1 , ecl_kw2 , cmp_elements);
    } else
      return true;  // None of the headers have this keyword - that is equality!
  } else
    return false;
}


/**
   When loading historical summary results the SMSPEC header of the
   historical results is not internalized, i.e. it is essential that
   the historical case has identical header as the main case. This
   function compares the ecl_file represeantation of two SMSPEC
   headers. 

   Unfortunately there are legitimate reasons why some of the headers
   can be different; in particular new well can appear. In the code
   below we therefor only check a limited set of keywords for
   equality.
*/

static bool ecl_smspec_file_equal( const ecl_file_type * header1 , const ecl_file_type * header2) {
  if (! ecl_smspec_kw_equal( header1 , header2 , KEYWORDS_KW , 0))
    return false;
  
  if (! ecl_smspec_kw_equal( header1 , header2 , STARTDAT_KW , 0))
    return false;

  if (! ecl_smspec_kw_equal( header1 , header2 , UNITS_KW , 0))
    return false;
  
  if (! ecl_smspec_kw_equal( header1 , header2 , DIMENS_KW , 4))  // Only the first four elements are compared.
    return false;
  
  if (!ecl_smspec_kw_equal( header1, header2 , LGRS_KW , 0))
    return false;
  
  return true;
}


/**
   This will iterate backwards through the RESTART header in the
   SMSPEC files to find names of the case(s) this case has been
   restarted from. 

   The case names are internalized in the restart_list field of the
   ecl_smspec instance. The actual loading of the restart summary data
   is subsequently handled by the ecl_sum_data function.
*/

static void ecl_smspec_load_restart( ecl_smspec_type * ecl_smspec , const ecl_file_type * header ) {
  if (ecl_file_has_kw( header , RESTART_KW )) {
    const ecl_kw_type * restart_kw = ecl_file_iget_kw( header , 0 );
    char   tmp_base[73];   /* To accomodate a maximum of 9 items which consist of 8 characters each. */
    char * restart_base;
    int i;
    tmp_base[0] = '\0';
    for (i=0; i < ecl_kw_get_size( restart_kw ); i++) 
      strcat( tmp_base , ecl_kw_iget_ptr( restart_kw , i ));
    
    restart_base = util_alloc_strip_copy( tmp_base );
    if (strlen(restart_base)) {  /* We ignore the empty ones. */
      char * path;
      char * smspec_header;

      util_alloc_file_components( ecl_smspec->header_file , &path , NULL , NULL );
      smspec_header = ecl_util_alloc_exfilename( path , restart_base , ECL_SUMMARY_HEADER_FILE , ecl_smspec->formatted , 0);
      if (smspec_header == NULL) 
        fprintf(stderr,"Warning - the header file: %s refers to restart from case: %s - which was not found.... \n", ecl_smspec->header_file , restart_base);
      
      else {
        if (!util_same_file(smspec_header , ecl_smspec->header_file)) {   /* Restart from the current case is ignored. */
          /* 
             Verify that this smspec_header is not already in the list of restart
             cases. Don't know if this is at all possible, but this test
             nevertheless prevents against a recursive death.
          */
          if (!stringlist_contains( ecl_smspec->restart_list , restart_base)) {
            ecl_file_type * restart_header = ecl_file_open( smspec_header , 0);
            
            if (ecl_smspec_file_equal( header , restart_header)) {
              stringlist_insert_copy( ecl_smspec->restart_list , 0 , restart_base );
              ecl_smspec_load_restart( ecl_smspec , restart_header);   /* Recursive call */ 
            } else 
              fprintf(stderr,"** Warning: the historical case: %s is not compatible with the current case - ignored.\n" , 
                      ecl_file_get_src_file( restart_header));
            
            ecl_file_close( restart_header );
          }
        }
      }
      util_safe_free( path );
      util_safe_free( smspec_header );
    }
    free( restart_base );
  }
}

  

void ecl_smspec_index_node( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node) {
  /*
    It is possible crate a node which is not fully specified, e.g. the
    well or group name can be left at NULL. In that case the node is
    not installed in the different indexes. 
  */
  // var_type == ECL_SMSPEC_INVALID_VAR??
  if (smspec_node_get_gen_key1( smspec_node ) != NULL) {
    ecl_smspec_install_gen_keys( ecl_smspec , smspec_node );
    ecl_smspec_install_special_keys( ecl_smspec , smspec_node );
  }
  if (smspec_node_need_nums( smspec_node ))
    ecl_smspec->need_nums = true;
}


static void ecl_smspec_set_params_size( ecl_smspec_type * ecl_smspec , int params_size) {
  ecl_smspec->params_size = params_size;
  float_vector_iset( ecl_smspec->params_default , ecl_smspec->params_size - 1 , PARAMS_GLOBAL_DEFAULT);
}



void ecl_smspec_insert_node(ecl_smspec_type * ecl_smspec, smspec_node_type * smspec_node){ 
  if (!ecl_smspec->locked) {
    int internal_index = vector_get_size( ecl_smspec->smspec_nodes );
    
    /* This IF test should only apply in write_mode. */
    if (smspec_node_get_params_index( smspec_node ) < 0) {
      if (!ecl_smspec->write_mode)
        util_abort("%s: internal error \n",__func__);
      smspec_node_set_params_index( smspec_node , internal_index);
      
      if (internal_index >= ecl_smspec->params_size)
        ecl_smspec_set_params_size( ecl_smspec , internal_index + 1);
    }
    vector_append_owned_ref( ecl_smspec->smspec_nodes , smspec_node , smspec_node_free__ );
    
    {
      int params_index = smspec_node_get_params_index( smspec_node );
      
      /* This indexing must be used when writing. */
      int_vector_iset( ecl_smspec->index_map , internal_index , params_index);
      
      float_vector_iset( ecl_smspec->params_default , params_index , smspec_node_get_default(smspec_node) );
    }
  } else
    util_abort("%s: sorry - the smspec header has been locked (can not mix ecl_sum_add_var() and ecl_sum_add_tstep() calls.)\n",__func__);
}


void ecl_smspec_add_node( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node ) {
  ecl_smspec_insert_node( ecl_smspec , smspec_node );
  ecl_smspec_index_node( ecl_smspec , smspec_node );
}



void ecl_smspec_init_var( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node , const char * keyword , const char * wgname , int num, const char * unit ) {
  if (smspec_node_init( smspec_node , ecl_smspec_identify_var_type( keyword ) , wgname , keyword , unit , ecl_smspec->key_join_string , ecl_smspec->grid_dims , num ))
    ecl_smspec_index_node( ecl_smspec , smspec_node );
  else
    util_abort("%s: initializing node failed \n",__func__);
}


const int_vector_type * ecl_smspec_get_index_map( const ecl_smspec_type * smspec ) {
  return smspec->index_map;
}




static void ecl_smspec_fread_header(ecl_smspec_type * ecl_smspec, const char * header_file , bool include_restart) {
  ecl_file_type * header = ecl_file_open( header_file , 0);
  {
    ecl_kw_type *wells     = ecl_file_iget_named_kw(header, WGNAMES_KW  , 0);
    ecl_kw_type *keywords  = ecl_file_iget_named_kw(header, KEYWORDS_KW , 0);
    ecl_kw_type *startdat  = ecl_file_iget_named_kw(header, STARTDAT_KW , 0);
    ecl_kw_type *units     = ecl_file_iget_named_kw(header, UNITS_KW    , 0);
    ecl_kw_type *dimens    = ecl_file_iget_named_kw(header, DIMENS_KW   , 0);
    ecl_kw_type *nums      = NULL;
    ecl_kw_type *lgrs      = NULL;
    ecl_kw_type *numlx     = NULL;
    ecl_kw_type *numly     = NULL;
    ecl_kw_type *numlz     = NULL;
    int params_index;
    ecl_smspec->num_regions     = 0;
    if (startdat == NULL) 
      util_abort("%s: could not locate STARTDAT keyword in header - aborting \n",__func__);
    
    if (ecl_file_has_kw(header , NUMS_KW))
      nums = ecl_file_iget_named_kw(header , NUMS_KW , 0);
    
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
    ecl_smspec_set_params_size( ecl_smspec , ecl_kw_get_size(keywords));
    
    ecl_util_get_file_type( header_file , &ecl_smspec->formatted , NULL );
    
    {
      for (params_index=0; params_index < ecl_kw_get_size(wells); params_index++) {
        float default_value          = PARAMS_GLOBAL_DEFAULT;
        int num                      = SMSPEC_NUMS_INVALID;
        char * well                  = util_alloc_strip_copy(ecl_kw_iget_ptr(wells    , params_index));
        char * kw                    = util_alloc_strip_copy(ecl_kw_iget_ptr(keywords , params_index));
        char * unit                  = util_alloc_strip_copy(ecl_kw_iget_ptr(units    , params_index));
        char * lgr_name              = NULL;  

        smspec_node_type * smspec_node;
        ecl_smspec_var_type var_type = ecl_smspec_identify_var_type( kw );
        if (nums != NULL) num        = ecl_kw_iget_int(nums , params_index);
        if (ecl_smspec_lgr_var_type( var_type )) {        
          int lgr_i = ecl_kw_iget_int( numlx , params_index );
          int lgr_j = ecl_kw_iget_int( numly , params_index );
          int lgr_k = ecl_kw_iget_int( numlz , params_index );
          lgr_name  = util_alloc_strip_copy(  ecl_kw_iget_ptr( lgrs , params_index ));
          smspec_node = smspec_node_alloc_lgr( var_type , well , kw , unit , lgr_name , ecl_smspec->key_join_string , lgr_i , lgr_j , lgr_k , params_index, default_value);
        } else 
          smspec_node = smspec_node_alloc( var_type , well , kw , unit , ecl_smspec->key_join_string , ecl_smspec->grid_dims , num , params_index , default_value);

        
        if (smspec_node != NULL) {
          /** OK - we know this is valid shit. */
          ecl_smspec_add_node( ecl_smspec , smspec_node );
        }
        
        free( kw );
        free( well );
        free( unit );
        util_safe_free( lgr_name );
      }
    }
  }
  ecl_smspec->header_file = util_alloc_realpath( header_file );
  if (include_restart)
    ecl_smspec_load_restart( ecl_smspec , header );
  
  ecl_file_close( header );
}



ecl_smspec_type * ecl_smspec_fread_alloc(const char *header_file, const char * key_join_string , bool include_restart) {
  ecl_smspec_type *ecl_smspec;
  
  {
    char *path;
    util_alloc_file_components(header_file , &path , NULL , NULL);
    ecl_smspec = ecl_smspec_alloc_empty(false , key_join_string);
    util_safe_free(path);
  }
  
  ecl_smspec_fread_header(ecl_smspec , header_file , include_restart);
  
  if (hash_has_key( ecl_smspec->misc_var_index , "TIME"))
    ecl_smspec->time_index = smspec_node_get_params_index( hash_get(ecl_smspec->misc_var_index , "TIME") );
  
  if (hash_has_key(ecl_smspec->misc_var_index , "DAY")) {
    ecl_smspec->day_index   = smspec_node_get_params_index( hash_get(ecl_smspec->misc_var_index , "DAY") );
    ecl_smspec->month_index = smspec_node_get_params_index( hash_get(ecl_smspec->misc_var_index , "MONTH") );
    ecl_smspec->year_index  = smspec_node_get_params_index( hash_get(ecl_smspec->misc_var_index , "YEAR") );
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
}


int ecl_smspec_get_num_groups(const ecl_smspec_type * ecl_smspec) {
  return hash_get_size(ecl_smspec->group_var_index);
}


char ** ecl_smspec_alloc_group_names(const ecl_smspec_type * ecl_smspec) {
  return hash_alloc_keylist(ecl_smspec->group_var_index);
}

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

#define NODE_RETURN_INDEX(node)                         \
  if (node == NULL)                                     \
    return -1;                                          \
  else                                                  \
    return smspec_node_get_params_index( node );


#define NODE_RETURN_EXISTS(node)                \
  if (node == NULL)                             \
    return false;                               \
  else                                          \
    return true;



/******************************************************************/
/* Well variables */

const smspec_node_type * ecl_smspec_get_well_var_node( const ecl_smspec_type * smspec , const char * well , const char * var) {
  const smspec_node_type * node = NULL;
  if (hash_has_key( smspec->well_var_index , well)) {
    hash_type * var_hash = hash_get(smspec->well_var_index , well);
    if (hash_has_key(var_hash , var))
      node = hash_get(var_hash , var);
  }
  return node;
}


int ecl_smspec_get_well_var_params_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_well_var_node( ecl_smspec , well , var );
  NODE_RETURN_INDEX(node);
}


bool ecl_smspec_has_well_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_well_var_node(ecl_smspec , well ,var);
  NODE_RETURN_EXISTS(node);
}



/*****************************************************************/
/* Group variables */

const smspec_node_type * ecl_smspec_get_group_var_node( const ecl_smspec_type * smspec , const char * group , const char * var) {
  const smspec_node_type * node = NULL;
  if (hash_has_key(smspec->group_var_index , group)) {
    hash_type * var_hash = hash_get(smspec->group_var_index , group);
    if (hash_has_key(var_hash , var))
      node = hash_get(var_hash , var);
  }
  return node;
}


int ecl_smspec_get_group_var_params_index(const ecl_smspec_type * ecl_smspec , const char * group , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_group_var_node( ecl_smspec , group , var );
  NODE_RETURN_INDEX(node);
}


bool ecl_smspec_has_group_var(const ecl_smspec_type * ecl_smspec , const char * group , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_group_var_node(ecl_smspec , group ,var);
  NODE_RETURN_EXISTS(node);
}


/*****************************************************************/
/* Field variables */

const smspec_node_type * ecl_smspec_get_field_var_node(const ecl_smspec_type * ecl_smspec , const char *var) {
  const smspec_node_type * node = NULL;
  if (hash_has_key(ecl_smspec->field_var_index , var))
    node = hash_get(ecl_smspec->field_var_index , var);
  
  return node;
}


int ecl_smspec_get_field_var_params_index(const ecl_smspec_type * ecl_smspec , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_field_var_node( ecl_smspec , var );
  NODE_RETURN_INDEX(node);
}




bool ecl_smspec_has_field_var(const ecl_smspec_type * ecl_smspec , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_field_var_node( ecl_smspec , var );
  NODE_RETURN_EXISTS(node);
}

/*****************************************************************/
/* Block variables */

/**
   Observe that block_nr is represented as char literal,
   i.e. "2345". This is because it will be used as a hash key.
   
   This is the final low level function which actually consults the
   hash tables.
*/

static const smspec_node_type * ecl_smspec_get_block_var_node_string(const ecl_smspec_type * ecl_smspec , const char * block_var , const char * block_str) {
  const smspec_node_type * node = NULL;

  if (hash_has_key(ecl_smspec->block_var_index , block_var)) {
    hash_type * block_hash = hash_get(ecl_smspec->block_var_index , block_var);
    if (hash_has_key(block_hash , block_str))
      node = hash_get(block_hash , block_str);
  }

  return node;
}


const smspec_node_type * ecl_smspec_get_block_var_node(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr) {
  const smspec_node_type * node;
  char * block_str = util_alloc_sprintf("%d" , block_nr);
  node = ecl_smspec_get_block_var_node_string(ecl_smspec , block_var , block_str);
  free( block_str );
  return node;
}


const smspec_node_type * ecl_smspec_get_block_var_node_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k) {
  return ecl_smspec_get_block_var_node( ecl_smspec , block_var , ecl_smspec_get_global_grid_index( ecl_smspec , i,j,k) );
}


bool ecl_smspec_has_block_var(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr) {
  const smspec_node_type * node = ecl_smspec_get_block_var_node( ecl_smspec , block_var , block_nr );
  NODE_RETURN_EXISTS(node);
}  


bool ecl_smspec_has_block_var_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k) {
  const smspec_node_type * node = ecl_smspec_get_block_var_node_ijk( ecl_smspec , block_var , i,j,k );
  NODE_RETURN_EXISTS(node);
}


int ecl_smspec_get_block_var_params_index(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr) {
  const smspec_node_type * node = ecl_smspec_get_block_var_node( ecl_smspec , block_var , block_nr );
  NODE_RETURN_INDEX(node);
}  


int ecl_smspec_get_block_var_params_index_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k) {
  const smspec_node_type * node = ecl_smspec_get_block_var_node_ijk( ecl_smspec , block_var , i,j,k );
  NODE_RETURN_INDEX(node);
}


/*****************************************************************/
/* Region variables */
/**
   region_nr: [1...num_regions] (NOT C-based indexing)
*/



const smspec_node_type * ecl_smspec_get_region_var_node(const ecl_smspec_type * ecl_smspec , const char *region_var , int region_nr) {
  const smspec_node_type * node = NULL;

  if (hash_has_key(ecl_smspec->region_var_index , region_var)) {
    char * nr_str = util_alloc_sprintf( "%d" , region_nr );
    hash_type * nr_hash = hash_get(ecl_smspec->region_var_index , region_var);
    if (hash_has_key( nr_hash , nr_str)) 
      node = hash_get( nr_hash , nr_str );
    free( nr_str );
  }
  
  return node;
}


bool ecl_smspec_has_region_var(const ecl_smspec_type * ecl_smspec , const char *region_var, int region_nr) {
  const smspec_node_type * node = ecl_smspec_get_region_var_node( ecl_smspec , region_var , region_nr );
  NODE_RETURN_EXISTS(node);
}


int ecl_smspec_get_region_var_params_index(const ecl_smspec_type * ecl_smspec , const char *region_var, int region_nr) {
  const smspec_node_type * node = ecl_smspec_get_region_var_node( ecl_smspec , region_var , region_nr );
  NODE_RETURN_INDEX(node);
}

/*****************************************************************/
/* Misc variables */

const smspec_node_type * ecl_smspec_get_misc_var_node(const ecl_smspec_type * ecl_smspec , const char *var) {
  const smspec_node_type * node = NULL;

  if (hash_has_key(ecl_smspec->misc_var_index , var))
    node = hash_get(ecl_smspec->misc_var_index , var);
  
  return node;
}


bool ecl_smspec_has_misc_var(const ecl_smspec_type * ecl_smspec , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_misc_var_node( ecl_smspec , var );
  NODE_RETURN_EXISTS(node);
}

int ecl_smspec_get_misc_var_params_index(const ecl_smspec_type * ecl_smspec , const char *var) {
  const smspec_node_type * node = ecl_smspec_get_misc_var_node( ecl_smspec , var );
  NODE_RETURN_INDEX(node);
}


/*****************************************************************/
/* Well completion - not fully implemented ?? */


const smspec_node_type * ecl_smspec_get_well_completion_var_node(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const smspec_node_type * node = NULL;

  char * cell_str = util_alloc_sprintf("%d" , cell_nr);
  if (hash_has_key(ecl_smspec->well_completion_var_index , well)) {
    hash_type * cell_hash = hash_get(ecl_smspec->well_completion_var_index , well);
    
    if (hash_has_key(cell_hash , cell_str)) {
      hash_type * var_hash = hash_get(cell_hash , cell_str);
      if (hash_has_key(var_hash , var))
        node = hash_get( var_hash , var);
    }
  }
  free(cell_str);
  return node;
}


bool  ecl_smspec_has_well_completion_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const smspec_node_type * node = ecl_smspec_get_well_completion_var_node( ecl_smspec , well , var , cell_nr );
  NODE_RETURN_EXISTS( node );
}


int  ecl_smspec_get_well_completion_var_params_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr) {
  const smspec_node_type * node = ecl_smspec_get_well_completion_var_node( ecl_smspec , well , var , cell_nr );
  NODE_RETURN_INDEX( node );
}


/*****************************************************************/
/* General variables ... */


/* There is a quite wide range of error which are just returned as
   "Not found" (i.e. -1). */
/* Completions not supported yet. */



const smspec_node_type * ecl_smspec_get_general_var_node( const ecl_smspec_type * smspec , const char * lookup_kw ) {
  if (hash_has_key( smspec->gen_var_index , lookup_kw )) {
    const smspec_node_type * smspec_node = hash_get( smspec->gen_var_index , lookup_kw );
    return smspec_node;
  } else
    return NULL;
}


int ecl_smspec_get_general_var_params_index(const ecl_smspec_type * ecl_smspec , const char * lookup_kw) {
  const smspec_node_type * node = ecl_smspec_get_general_var_node( ecl_smspec , lookup_kw );
  NODE_RETURN_INDEX( node );
}


bool ecl_smspec_has_general_var(const ecl_smspec_type * ecl_smspec , const char * lookup_kw) {
  const smspec_node_type * node = ecl_smspec_get_general_var_node( ecl_smspec , lookup_kw );
  NODE_RETURN_EXISTS( node );
} 


/** DIES if the lookup_kw is not present. */
const char * ecl_smspec_get_general_var_unit( const ecl_smspec_type * ecl_smspec , const char * lookup_kw) {
  const smspec_node_type * smspec_node = hash_get( ecl_smspec->gen_var_index , lookup_kw );
  return smspec_node_get_unit( smspec_node );
}


/*****************************************************************/
/* 
   Pure indexed lookup - these functions can be used after one of the
   ecl_smspec_get_xxx_index() functions has been used first.
*/

//const char * ecl_smspec_iget_unit( const ecl_smspec_type * smspec , int node_index ) {
//  const smspec_node_type * smspec_node = ecl_smspec_iget_node( smspec , node_index );
//  return smspec_node_get_unit( smspec_node );
//}
//
//int ecl_smspec_iget_num( const ecl_smspec_type * smspec , int node_index ) {
//  const smspec_node_type * smspec_node = ecl_smspec_iget_node( smspec , node_index );
//  return smspec_node_get_num( smspec_node );
//}
//
//const char * ecl_smspec_iget_wgname( const ecl_smspec_type * smspec , int node_index ) {
//  const smspec_node_type * smspec_node = ecl_smspec_iget_node( smspec , node_index );
//  return smspec_node_get_wgname( smspec_node );
//}
//
//const char * ecl_smspec_iget_keyword( const ecl_smspec_type * smspec , int index ) {
//  const smspec_node_type * smspec_node = ecl_smspec_iget_node( smspec , index );
//  return smspec_node_get_keyword( smspec_node );
//}


/*****************************************************************/

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
  return ecl_smspec->header_file;
}



const stringlist_type * ecl_smspec_get_restart_list( const ecl_smspec_type * ecl_smspec) {
  return ecl_smspec->restart_list;
}


const float_vector_type * ecl_smspec_get_params_default( const ecl_smspec_type * ecl_smspec ) {
  return ecl_smspec->params_default;
}



void ecl_smspec_free(ecl_smspec_type *ecl_smspec) {
  hash_free(ecl_smspec->well_var_index);
  hash_free(ecl_smspec->well_completion_var_index);
  hash_free(ecl_smspec->group_var_index);
  hash_free(ecl_smspec->field_var_index);
  hash_free(ecl_smspec->region_var_index);
  hash_free(ecl_smspec->misc_var_index);
  hash_free(ecl_smspec->block_var_index);
  hash_free(ecl_smspec->gen_var_index);
  util_safe_free( ecl_smspec->header_file );
  int_vector_free( ecl_smspec->index_map );
  float_vector_free( ecl_smspec->params_default );
  vector_free( ecl_smspec->smspec_nodes );
  stringlist_free( ecl_smspec->restart_list );
  free( ecl_smspec );
}


void ecl_smspec_free__(void * __ecl_smspec) {
  ecl_smspec_type * ecl_smspec = ecl_smspec_safe_cast( __ecl_smspec);
  ecl_smspec_free( ecl_smspec );
}

int ecl_smspec_get_sim_days_index( const ecl_smspec_type * smspec ) {
  return smspec->time_index;
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
  const  smspec_node_type * smspec_node = hash_get( smspec->gen_var_index , gen_key );
  return smspec_node_is_total( smspec_node );
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
  hash_type * ex_keys = hash_alloc( );
  int i;
  for (i=0; i < stringlist_get_size( keys ); i++) 
    hash_insert_int( ex_keys , stringlist_iget( keys , i ) , 1);

  {
    hash_iter_type * iter = hash_iter_alloc( smspec->gen_var_index );
    while (!hash_iter_is_complete( iter )) {
      const char * key = hash_iter_get_next_key( iter );
      if ((pattern == NULL) || (util_fnmatch( pattern , key ) == 0)) {
        if (!hash_has_key( ex_keys , key))
          stringlist_append_copy( keys , key );
      }
    }
    hash_iter_free( iter );
  }
  
  hash_free( ex_keys );
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
  return smspec->key_join_string;
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

stringlist_type * ecl_smspec_alloc_well_list( const ecl_smspec_type * smspec , const char * pattern) {
  stringlist_type * well_list = stringlist_alloc_new( );
  {
    hash_iter_type * iter = hash_iter_alloc( smspec->well_var_index );
    
    while (!hash_iter_is_complete( iter )) {
      const char * well_name = hash_iter_get_next_key( iter );
      if (pattern == NULL)
        stringlist_append_copy( well_list , well_name );
      else if (util_fnmatch( pattern , well_name) == 0) 
        stringlist_append_copy( well_list , well_name );
    }
    hash_iter_free(iter);
  }
  stringlist_sort( well_list , (string_cmp_ftype *) util_strcmp_int );
  return well_list;
}


/** 
    Returns a stringlist instance with all the (valid) group names. It
    is the responsability of the calling scope to free the stringlist
    with stringlist_free();
*/

stringlist_type * ecl_smspec_alloc_group_list( const ecl_smspec_type * smspec , const char * pattern) {
  stringlist_type * group_list = stringlist_alloc_new( );
  {
    hash_iter_type * iter = hash_iter_alloc( smspec->group_var_index );
    
    while (!hash_iter_is_complete( iter )) {
      const char * group_name = hash_iter_get_next_key( iter );
      if (pattern == NULL)
        stringlist_append_copy( group_list , group_name );
      else if (util_fnmatch( pattern , group_name) == 0) 
        stringlist_append_copy( group_list , group_name );
    }
    hash_iter_free(iter);
  }
  stringlist_sort( group_list , (string_cmp_ftype *) util_strcmp_int );
  return group_list;
}



/** 
    Returns a stringlist instance with all the well variables.  It is
    the responsability of the calling scope to free the stringlist
    with stringlist_free();
*/

stringlist_type * ecl_smspec_alloc_well_var_list( const ecl_smspec_type * smspec ) {
  hash_iter_type * well_iter = hash_iter_alloc( smspec->well_var_index );
  hash_type      * var_hash = hash_iter_get_next_value( well_iter );
  hash_iter_free( well_iter );
  return hash_alloc_stringlist( var_hash );
}



int ecl_smspec_get_params_size( const ecl_smspec_type * smspec ) {
  return smspec->params_size;
}



const int * ecl_smspec_get_grid_dims( const ecl_smspec_type * smspec ) {
  return smspec->grid_dims;
}


void ecl_smspec_update_wgname( ecl_smspec_type * smspec , smspec_node_type * node , const char * wgname ) {
  smspec_node_update_wgname( node , wgname , smspec->key_join_string);
  ecl_smspec_index_node( smspec , node );
}


/*****************************************************************/

char * ecl_smspec_alloc_well_key( const ecl_smspec_type * smspec , const char * keyword , const char * wgname) {
  return smspec_alloc_well_key( smspec->key_join_string , keyword , wgname );
}
