/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'smspec_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/set.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/stringlist.h>
#include <ert/util/type_macros.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_smspec.h>
#include <ert/ecl/smspec_node.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>




/**
   This struct contains meta-information about one element in the smspec
   file; the content is based on the smspec vectors WGNAMES, KEYWORDS, UNIT
   and NUMS. The index field of this struct points to where the actual data
   can be found in the PARAMS vector of the *.Snnnn / *.UNSMRY files;
   probably the most important field.  
*/

#define SMSPEC_TYPE_ID 61550451


struct smspec_node_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                 * gen_key1;           /* The main composite key, i.e. WWCT:OP3 for this element. */ 
  char                 * gen_key2;           /* Some of the ijk based elements will have both a xxx:i,j,k and a xxx:num key. Some of the region_2_region elements will have both a xxx:num and a xxx:r2-r2 key. Mostly NULL. */
  ecl_smspec_var_type    var_type;           /* The variable type */
  char                 * wgname;             /* The value of the WGNAMES vector for this element. */
  char                 * keyword;            /* The value of the KEYWORDS vector for this elements. */
  char                 * unit;               /* The value of the UNITS vector for this elements. */
  int                    num;                /* The value of the NUMS vector for this elements - NB this will have the value SMSPEC_NUMS_INVALID if the smspec file does not have a NUMS vector. */
  int                  * ijk;                /* The ijk coordinates (NB: OFFSET 1) corresponding to the nums value - will be NULL if not relevant. */
  char                 * lgr_name;           /* The lgr name of the current variable - will be NULL for non-lgr variables. */
  int                  * lgr_ijk;            /* The (i,j,k) coordinate, in the local grid, if this is a LGR variable. WIll be NULL for no-lgr variables. */
  bool                   rate_variable;      /* Is this a rate variable (i.e. WOPR) or a state variable (i.e. BPR). Relevant when doing time interpolation. */
  bool                   total_variable;     /* Is this a total variable like WOPT? */
  bool                   need_nums;          /* Do we use the NUMS vector - relevant for storing. */
  bool                   historical;         /* Does the name end with 'H'? */
  int                    params_index;       /* The index of this variable (applies to all the vectors - in particular the PARAMS vectors of the summary files *.Snnnn / *.UNSMRY ). */
  float                  default_value;      /* Default value for this variable. */
};


/*****************************************************************/
/*
  The key formats for the combined keys like e.g. 'WWCT:OP_5' should
  have the keyword, i.e. 'WWCT', as the first part of the string. That
  guarantees that the function ecl_smspec_identify_var_type() can take
  both a pure ECLIPSE variable name, like .e.g 'WWCT' and also an
  ecl_sum combined key like 'WWCT:OPX' as input.
*/

#define ECL_SUM_KEYFMT_BLOCK_IJK              "%s%s%d,%d,%d"
#define ECL_SUM_KEYFMT_BLOCK_NUM              "%s%s%d"
#define ECL_SUM_KEYFMT_LOCAL_BLOCK            "%s%s%s%s%d,%d,%d"
#define ECL_SUM_KEYFMT_COMPLETION_IJK         "%s%s%s%s%d,%d,%d" 
#define ECL_SUM_KEYFMT_COMPLETION_NUM         "%s%s%s%s%d"
#define ECL_SUM_KEYFMT_LOCAL_COMPLETION       "%s%s%s%s%s%s%d,%d,%d"
#define ECL_SUM_KEYFMT_GROUP                  "%s%s%s"
#define ECL_SUM_KEYFMT_WELL                   "%s%s%s"
#define ECL_SUM_KEYFMT_REGION                 "%s%s%d"
#define ECL_SUM_KEYFMT_REGION_2_REGION_R1R2   "%s%s%d-%d"
#define ECL_SUM_KEYFMT_REGION_2_REGION_NUM    "%s%s%d"
#define ECL_SUM_KEYFMT_SEGMENT                "%s%s%s%s%d"
#define ECL_SUM_KEYFMT_LOCAL_WELL             "%s%s%s%s%s"

UTIL_SAFE_CAST_FUNCTION( smspec_node , SMSPEC_TYPE_ID )


char * smspec_alloc_block_num_key( const char * join_string , const char * keyword , int num) {
  return util_alloc_sprintf(ECL_SUM_KEYFMT_BLOCK_NUM,
                            keyword , 
                            join_string , 
                            num );
}

char * smspec_alloc_local_block_key( const char * join_string , const char * keyword , const char * lgr_name , int i , int j , int k) {
  return util_alloc_sprintf(ECL_SUM_KEYFMT_LOCAL_BLOCK , 
                            keyword , 
                            join_string , 
                            lgr_name , 
                            join_string , 
                            i,j,k);
}


char * smspec_alloc_region_key( const char * join_string , const char * keyword , int num) {
  return util_alloc_sprintf(ECL_SUM_KEYFMT_REGION , 
                            keyword , 
                            join_string , 
                            num );
}

char * smspec_alloc_region_2_region_r1r2_key( const char * join_string , const char * keyword , int r1, int r2) {
  return util_alloc_sprintf(ECL_SUM_KEYFMT_REGION_2_REGION_R1R2,
                            keyword,
                            join_string, 
                            r1, 
                            r2);
}

char * smspec_alloc_region_2_region_num_key( const char * join_string , const char * keyword , int num) {
  return util_alloc_sprintf(ECL_SUM_KEYFMT_REGION_2_REGION_NUM,  
                            keyword , 
                            join_string , 
                            num);
}
  


char * smspec_alloc_block_ijk_key( const char * join_string , const char * keyword , int i , int j , int k) {
  return util_alloc_sprintf(ECL_SUM_KEYFMT_BLOCK_IJK , 
                            keyword,
                            join_string , 
                            i,j,k);
}



char * smspec_alloc_completion_ijk_key( const char * join_string , const char * keyword, const char * wgname , int i , int j , int k) {
  if (wgname != NULL)
    return util_alloc_sprintf( ECL_SUM_KEYFMT_COMPLETION_IJK , 
                               keyword , 
                               join_string , 
                               wgname , 
                               join_string , 
                               i , j , k );
  else
    return NULL;
}



char * smspec_alloc_completion_num_key( const char * join_string , const char * keyword, const char * wgname , int num) {
  if (wgname != NULL)
    return util_alloc_sprintf(ECL_SUM_KEYFMT_COMPLETION_NUM,
                              keyword , 
                              join_string , 
                              wgname , 
                              join_string , 
                              num );
  else
    return NULL;
}

/*
  To support ECLIPSE behaviour where new wells/groups can be created
  during the simulation it must be possible to create a smspec node
  with an initially unknown well/group name; all gen_key formats which
  use the wgname value must therefor accept a NULL value for wgname.
*/

static char * smspec_alloc_wgname_key( const char * join_string , const char * keyword , const char * wgname) {
  if (wgname != NULL) 
    return util_alloc_sprintf(ECL_SUM_KEYFMT_WELL,
                              keyword , 
                              join_string , 
                              wgname );
  else
    return NULL;
}

char * smspec_alloc_group_key( const char * join_string , const char * keyword , const char * wgname) {
  return smspec_alloc_wgname_key( join_string , keyword , wgname );
}

char * smspec_alloc_well_key( const char * join_string , const char * keyword , const char * wgname) {
  return smspec_alloc_wgname_key( join_string , keyword , wgname );
}

char * smspec_alloc_segment_key( const char * join_string , const char * keyword , const char * wgname , int num) {
  if (wgname != NULL)
    return util_alloc_sprintf(ECL_SUM_KEYFMT_SEGMENT , 
                              keyword , 
                              join_string , 
                              wgname , 
                              join_string , 
                              num );
  else
    return NULL;
}


char * smspec_alloc_local_well_key( const char * join_string , const char * keyword , const char * lgr_name , const char * wgname) {
  if (wgname != NULL)
    return util_alloc_sprintf( ECL_SUM_KEYFMT_LOCAL_WELL , 
                               keyword , 
                               join_string , 
                               lgr_name , 
                               join_string , 
                               wgname);
  else
    return NULL;
}

char * smspec_alloc_local_completion_key( const char * join_string, const char * keyword , const char * lgr_name , const char * wgname , int i , int j , int k) {
  if (wgname != NULL)
    return util_alloc_sprintf(ECL_SUM_KEYFMT_LOCAL_COMPLETION , 
                              keyword  , 
                              join_string , 
                              lgr_name , 
                              join_string ,
                              wgname , 
                              join_string ,
                              i,j,k);
  else
    return NULL;
}

/*****************************************************************/

static void smspec_node_set_keyword( smspec_node_type * smspec_node , const char * keyword ) {
  // ECLIPSE Standard: Max eight characters - everything beyond is silently dropped
  // This function can __ONLY__ be called on time; run-time chaning of keyword is not
  // allowed.
  if (smspec_node->keyword == NULL)
    smspec_node->keyword = util_alloc_substring_copy( keyword , 0 , 8);       
  else
    util_abort("%s: fatal error - attempt to change keyword runtime detected - aborting\n",__func__);
}


static void smspec_node_set_invalid_flags( smspec_node_type * smspec_node) {
  smspec_node->rate_variable  = false;
  smspec_node->total_variable = false;
  smspec_node->need_nums      = false;
  smspec_node->historical     = false;
}

static char LAST_CHAR(const char * s) {
  return s[ strlen(s) - 1];
}

static void smspec_node_set_flags( smspec_node_type * smspec_node) {
  /* 
     Check if this is a rate variabel - that info is used when
     interpolating results to true_time between ministeps. 
  */
  {
    const char *rate_vars[] = {"OPR" , "GPR" , "WPR" , "GOR" , "WCT"};
    int num_rate_vars = sizeof( rate_vars ) / sizeof( rate_vars[0] );
    bool  is_rate           = false;
    int ivar;
    for (ivar = 0; ivar < num_rate_vars; ivar++) {
      const char * var_substring = &smspec_node->keyword[1];
      if (strncmp( rate_vars[ivar] , var_substring , strlen( rate_vars[ivar] )) == 0) {
        is_rate = true;
        break;
      }
    }
    smspec_node->rate_variable = is_rate;
  }
  
  {
    if (LAST_CHAR(smspec_node->keyword) == 'H')
      smspec_node->historical = true;
  }

  /*
    This code checks in a predefined list whether a certain WGNAMES
    variable represents a total accumulated quantity. Only the last three
    characters in the variable is considered (i.e. the leading 'W', 'G' or
    'F' is discarded).
    
    The list below is all the keyowrds with 'Total' in the information from
    the tables 2.7 - 2.11 in the ECLIPSE fileformat documentation.  Have
    skipped some of the most exotic keywords.
  */
  {
    bool is_total = false;
    if (smspec_node->var_type == ECL_SMSPEC_WELL_VAR || smspec_node->var_type == ECL_SMSPEC_GROUP_VAR || smspec_node->var_type == ECL_SMSPEC_FIELD_VAR) {
      const char *total_vars[] = {"OPT"  , "GPT"  , "WPT" , "OPTF" , "OPTS" , "OIT"  , "OVPT" , "OVIT" , "MWT" , "WIT" ,
                                  "WVPT" , "WVIT" , "GMT"  , "GPTF" , "GIT"  , "SGT"  , "GST" , "FGT" , "GCT" , "GIMT" , 
                                  "WGPT" , "WGIT" , "EGT"  , "EXGT" , "GVPT" , "GVIT" , "LPT" , "VPT" , "VIT" };

      int num_total_vars = sizeof( total_vars ) / sizeof( total_vars[0] );
      int ivar;
      for (ivar = 0; ivar < num_total_vars; ivar++) {
        const char * var_substring = &smspec_node->keyword[1];
        /*
          We want to mark both FOPT and FOPTH as historical variables;
          we use strncmp() to make certain that the trailing 'H' is
          not included in the comparison.
        */
        if (strncmp( total_vars[ivar] , var_substring , strlen( total_vars[ivar] )) == 0) {
          is_total = true;
          break;
        }
      }
    }
    smspec_node->total_variable = is_total;
  }

  /*
    Check if this node needs the nums field; if at least one of the
    nodes need the NUMS field must be stored when writing a SMSPEC
    file. 
  */
  {
    if (smspec_node->var_type == ECL_SMSPEC_COMPLETION_VAR      || 
        smspec_node->var_type == ECL_SMSPEC_SEGMENT_VAR         || 
        smspec_node->var_type == ECL_SMSPEC_REGION_VAR          ||
        smspec_node->var_type == ECL_SMSPEC_REGION_2_REGION_VAR ||
        smspec_node->var_type == ECL_SMSPEC_BLOCK_VAR           ||
        smspec_node->var_type == ECL_SMSPEC_AQUIFER_VAR) 
      smspec_node->need_nums = true;
    else
      smspec_node->need_nums = false;
  }
}

/**
   It is possible to change the default value of an smspec node
   runtime, but observe that the new value will only be applied to the
   new timesteps you add after the change. Already created timesteps
   will not be updated if the default value is changed.
*/
void smspec_node_set_default( smspec_node_type * smspec_node , float default_value) {
  smspec_node->default_value = default_value;
}


float smspec_node_get_default( const smspec_node_type * smspec_node ) {
  return smspec_node->default_value;
}


smspec_node_type * smspec_node_alloc_new(int params_index, float default_value) {
  smspec_node_type * node = util_malloc( sizeof * node );
  
  UTIL_TYPE_ID_INIT( node , SMSPEC_TYPE_ID);
  node->params_index  = params_index;
  smspec_node_set_default( node , default_value );

  node->wgname        = NULL;
  node->num           = SMSPEC_NUMS_INVALID;
  node->ijk           = NULL; 
  
  node->gen_key1      = NULL;
  node->gen_key2      = NULL;

  node->var_type      = ECL_SMSPEC_INVALID_VAR;
  node->unit          = NULL;
  node->keyword       = NULL;
  node->lgr_name      = NULL;
  node->lgr_ijk       = NULL;  
    
  smspec_node_set_invalid_flags( node );
  return node;               // This is NOT usable
}


/**
   Observe that the wellname can have max 8 characters; anything
   beyond that is silently dropped.  
*/

static void smspec_node_set_wgname( smspec_node_type * index , const char * wgname ) {
  if (wgname == NULL) {
    util_safe_free( index->wgname );
    index->wgname = NULL;
  } else {
    if (strlen(wgname) > 8) 
      index->wgname = util_realloc_substring_copy(index->wgname , wgname , 8);
    else
      index->wgname = util_realloc_string_copy(index->wgname , wgname );
  }
}



static void smspec_node_set_lgr_name( smspec_node_type * index , const char * lgr_name ) {
  index->lgr_name = util_realloc_string_copy(index->lgr_name , lgr_name);
}


static void smspec_node_set_lgr_ijk( smspec_node_type * index , int lgr_i , int lgr_j , int lgr_k) {
  if (index->lgr_ijk == NULL)
    index->lgr_ijk = util_calloc( 3 , sizeof * index->lgr_ijk );
  
  index->lgr_ijk[0] = lgr_i;
  index->lgr_ijk[1] = lgr_j;
  index->lgr_ijk[2] = lgr_k;
}


static void smspec_node_set_num( smspec_node_type * index , const int grid_dims[3] , int num) {
  if (num == SMSPEC_NUMS_INVALID)
    util_abort("%s: explicitly trying to set nums == SMSPEC_NUMS_INVALID - seems like a bug?!\n",__func__);
  
  index->num = num;
  if ((index->var_type == ECL_SMSPEC_COMPLETION_VAR) || (index->var_type == ECL_SMSPEC_BLOCK_VAR)) {
    int global_index = num - 1;
    index->ijk = util_calloc( 3 , sizeof * index->ijk );
    
    index->ijk[2] = global_index / ( grid_dims[0] * grid_dims[1] );   global_index -= index->ijk[2] * (grid_dims[0] * grid_dims[1]);
    index->ijk[1] = global_index /  grid_dims[0] ;                    global_index -= index->ijk[1] * grid_dims[0];
    index->ijk[0] = global_index;

    index->ijk[0] += 1;
    index->ijk[1] += 1;
    index->ijk[2] += 1;
  }
}

/**
   This function will init the gen_key field of the smspec_node
   instance; this is the keyw which is used to install the
   smspec_node instance in the gen_var dictionary. The node related
   to grid locations are installed with both a XXX:num and XXX:i,j,k
   in the gen_var dictionary; this function will initializethe XXX:num
   form.
*/


static void smspec_node_set_gen_keys( smspec_node_type * smspec_node , const char * key_join_string) {
  switch( smspec_node->var_type) {
  case(ECL_SMSPEC_COMPLETION_VAR):
    // KEYWORD:WGNAME:NUM
    smspec_node->gen_key1 = smspec_alloc_completion_num_key( key_join_string , smspec_node->keyword , smspec_node->wgname , smspec_node->num);
    smspec_node->gen_key2 = smspec_alloc_completion_ijk_key( key_join_string , smspec_node->keyword , smspec_node->wgname , smspec_node->ijk[0], smspec_node->ijk[1], smspec_node->ijk[2]);
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    // KEYWORD
    smspec_node->gen_key1 = util_alloc_string_copy( smspec_node->keyword );
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    // KEYWORD:WGNAME 
    smspec_node->gen_key1 = smspec_alloc_group_key( key_join_string , smspec_node->keyword , smspec_node->wgname);
    break;
  case(ECL_SMSPEC_WELL_VAR):
    // KEYWORD:WGNAME 
    smspec_node->gen_key1 = smspec_alloc_well_key( key_join_string , smspec_node->keyword , smspec_node->wgname);
    break;
  case(ECL_SMSPEC_REGION_VAR):
    // KEYWORD:NUM
    smspec_node->gen_key1 = smspec_alloc_region_key( key_join_string , smspec_node->keyword , smspec_node->num);
    break;
  case (ECL_SMSPEC_SEGMENT_VAR): 
    // KEYWORD:WGNAME:NUM  
    smspec_node->gen_key1 = smspec_alloc_segment_key( key_join_string , smspec_node->keyword , smspec_node->wgname , smspec_node->num);  
    break;
  case(ECL_SMSPEC_REGION_2_REGION_VAR):
    // KEYWORDS:RXF:NUM and RXF:R1-R2
    smspec_node->gen_key1 = smspec_alloc_region_2_region_num_key( key_join_string , smspec_node->keyword , smspec_node->num);
    {
      int r1 = smspec_node->num % 32768;
      int r2 = ((smspec_node->num-r1) / 32768)-10;
      smspec_node->gen_key2 = smspec_alloc_region_2_region_r1r2_key( key_join_string , smspec_node->keyword , r1, r2);
    }
    break;
  case(ECL_SMSPEC_MISC_VAR):
    // KEYWORD
    /* Misc variable - i.e. date or CPU time ... */
    smspec_node->gen_key1 = util_alloc_string_copy( smspec_node->keyword );
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    // KEYWORD:NUM
    smspec_node->gen_key1 = smspec_alloc_block_num_key( key_join_string , smspec_node->keyword , smspec_node->num);
    smspec_node->gen_key2 = smspec_alloc_block_ijk_key( key_join_string , smspec_node->keyword , smspec_node->ijk[0], smspec_node->ijk[1], smspec_node->ijk[2]);
    break;
  case(ECL_SMSPEC_LOCAL_WELL_VAR):
    /** KEYWORD:LGR:WGNAME */
    smspec_node->gen_key1 = smspec_alloc_local_well_key( key_join_string , smspec_node->keyword , smspec_node->lgr_name , smspec_node->wgname);
    break;
  case(ECL_SMSPEC_LOCAL_BLOCK_VAR):
    /* KEYWORD:LGR:i,j,k */
    smspec_node->gen_key1 = smspec_alloc_local_block_key( key_join_string , 
                                                         smspec_node->keyword , 
                                                         smspec_node->lgr_name , 
                                                         smspec_node->lgr_ijk[0] , 
                                                         smspec_node->lgr_ijk[1] , 
                                                         smspec_node->lgr_ijk[2] );
    break;
  case(ECL_SMSPEC_LOCAL_COMPLETION_VAR):
    /* KEYWORD:LGR:WELL:i,j,k */
    smspec_node->gen_key1 = smspec_alloc_local_completion_key( key_join_string , 
                                                              smspec_node->keyword , 
                                                              smspec_node->lgr_name , 
                                                              smspec_node->wgname , 
                                                              smspec_node->lgr_ijk[0],
                                                              smspec_node->lgr_ijk[1],
                                                              smspec_node->lgr_ijk[2]);
    
    break;
  default:
    util_abort("%s: internal error - should not be here? \n" , __func__);
  }
}



void smspec_node_update_wgname( smspec_node_type * index , const char * wgname , const char * key_join_string) {
  smspec_node_set_wgname( index , wgname );
  util_safe_free( index->gen_key1 );
  util_safe_free( index->gen_key2 );
  smspec_node_set_gen_keys( index , key_join_string );
}

static void smspec_node_common_init( smspec_node_type * node , ecl_smspec_var_type var_type , const char * keyword , const char * unit ) {
  if (node->var_type == ECL_SMSPEC_INVALID_VAR) {
    smspec_node_set_unit( node , unit );
    smspec_node_set_keyword( node , keyword);
    node->var_type = var_type;
    smspec_node_set_flags( node );
  } else
    util_abort("%s: trying to re-init smspec node with keyword:%s - invalid \n",__func__ , keyword );
}



bool smspec_node_init( smspec_node_type * smspec_node, 
                        ecl_smspec_var_type var_type , 
                        const char * wgname  , 
                        const char * keyword , 
                        const char * unit    , 
                        const char * key_join_string , 
                        const int grid_dims[3] , 
                        int num) {
  
  bool initOK    = true;
  bool wgnameOK = true;
  if ((wgname != NULL) && (IS_DUMMY_WELL(wgname)))
    wgnameOK = false;
  
  smspec_node_common_init( smspec_node , var_type , keyword , unit );
  switch (var_type) {
  case(ECL_SMSPEC_COMPLETION_VAR):
    /* Completion variable : WGNAME & NUM */
    if (wgnameOK) {
      smspec_node_set_num( smspec_node , grid_dims , num );
      smspec_node_set_wgname( smspec_node , wgname );
    } else
      initOK = false;
    break;
  case(ECL_SMSPEC_GROUP_VAR):
    /* Group variable : WGNAME */
    if (wgnameOK) 
      smspec_node_set_wgname( smspec_node , wgname );
    else
      initOK = false;
    break;
  case(ECL_SMSPEC_WELL_VAR):
    /* Well variable : WGNAME */
    if (wgnameOK) 
      smspec_node_set_wgname( smspec_node , wgname );
    else
      initOK = false;
    break;
  case(ECL_SMSPEC_SEGMENT_VAR):
    if (wgnameOK && num >= 0) {
      smspec_node_set_wgname( smspec_node , wgname );
      smspec_node_set_num( smspec_node , grid_dims , num );
    } else
      initOK = false;
    break;
  case(ECL_SMSPEC_FIELD_VAR):
    /* Field variable : */
    /* Fully initialized with the smspec_common_init() function */
    break;
  case(ECL_SMSPEC_REGION_VAR):
    /* Region variable : NUM */
    smspec_node_set_num( smspec_node , grid_dims , num );
    break;
  case(ECL_SMSPEC_REGION_2_REGION_VAR):
    /* Region 2 region variable : NUM */
    smspec_node_set_num( smspec_node , grid_dims , num );
    break;
  case(ECL_SMSPEC_BLOCK_VAR):
    /* A block variable : NUM*/
    smspec_node_set_num( smspec_node , grid_dims , num );
    break;
  case(ECL_SMSPEC_MISC_VAR):
    /* Misc variable : */
    /* Fully initialized with the smspec_common_init() function */
    break;
  default:
    /* Lots of legitimate alternatives which are not internalized. */
    initOK = false;
    break;
  }
  if (initOK)
    smspec_node_set_gen_keys( smspec_node , key_join_string );
  return initOK;
}
  
/**
   This function will allocate a smspec_node instance, and initialize
   all the elements. Observe that the function can return NULL, in the
   case we do not care to internalize the variable in question,
   i.e. if it is a well_rate from a dummy well or a variable type we
   do not support at all.

   This function initializes a valid smspec_node instance based on
   the supplied var_type, and the input. Observe that when a new
   variable type is supported, the big switch() statement must be
   updated in the functions ecl_smspec_install_gen_key() and
   ecl_smspec_fread_header() functions in addition. UGGGLY
*/



smspec_node_type * smspec_node_alloc( ecl_smspec_var_type var_type , 
                                       const char * wgname  , 
                                       const char * keyword , 
                                       const char * unit    , 
                                       const char * key_join_string , 
                                       const int grid_dims[3] , 
                                       int num , int param_index, float default_value) {
  /*
    Well and group names in the wgname parameter is quite messy. The
    situation is as follows:

     o The ECLIPSE SMSPEC files are very verbose, and contain many
       entries like this:
 
            KEYWORD : "WWCT"
            WGNAME  : ":+:+:+:+"
  
       i.e. the keyword indicates that this is a perfectly legitimate
       well variable, however the special wgname value ":+:+:+:+"
       shows that this just a rubbish entry. We do not want to
       internalize these rubbish entries and this function should just
       return NULL.

       The ":+:+:+:+" string is in the #define symbol DUMMY_WELL and
       the macro IS_DUMMY_WELL(wgname) can used to compare with the
       DUMMY_WELL value.

     o When the ecl_sum instance is created in write mode; it must be
       possible to add smspec nodes for wells/groups which do not have
       a name yet. In this case we accept NULL as input value for the
       wgname parameter.

     o In the case of variables which do not use the wgname variable
       at all, e.g. like "FOPT" - the wgname input value is ignored
       completely.
  */
  
  smspec_node_type * smspec_node = smspec_node_alloc_new( param_index , default_value );
  if (smspec_node_init( smspec_node , var_type , wgname , keyword , unit , key_join_string , grid_dims, num))
    return smspec_node;
  else {
    smspec_node_free( smspec_node );
    return NULL;
  }
}



bool smspec_node_init_lgr( smspec_node_type * smspec_node , 
                           ecl_smspec_var_type var_type , 
                           const char * wgname  , 
                           const char * keyword , 
                           const char * unit    , 
                           const char * lgr , 
                           const char * key_join_string , 
                           int   lgr_i, int lgr_j , int lgr_k
                           ) {
  bool initOK = true;
  bool wgnameOK = true;
  if ((wgname != NULL) && (IS_DUMMY_WELL(wgname)))
    wgnameOK = false;
  
  smspec_node_common_init( smspec_node , var_type , keyword , unit );
  switch (var_type) {
  case(ECL_SMSPEC_LOCAL_WELL_VAR):
    if (wgnameOK) {
      smspec_node_set_wgname( smspec_node , wgname );
      smspec_node_set_lgr_name( smspec_node , lgr );
    } else
      initOK = false;
    break;
  case(ECL_SMSPEC_LOCAL_BLOCK_VAR):
    smspec_node_set_lgr_name( smspec_node , lgr );
    smspec_node_set_lgr_ijk( smspec_node , lgr_i, lgr_j , lgr_k );
    break;
  case(ECL_SMSPEC_LOCAL_COMPLETION_VAR):
    if (wgnameOK) {
      smspec_node_set_lgr_name( smspec_node , lgr );
      smspec_node_set_wgname( smspec_node , wgname );
      smspec_node_set_lgr_ijk( smspec_node , lgr_i, lgr_j , lgr_k );
    } else
      initOK = false;
    break;
  default:
    util_abort("%s: internal error:  in LGR function with  non-LGR keyword:%s \n",__func__ , keyword);
  }
  if (initOK)
    smspec_node_set_gen_keys( smspec_node , key_join_string );
  return initOK;
}



smspec_node_type * smspec_node_alloc_lgr( ecl_smspec_var_type var_type , 
                                          const char * wgname  , 
                                          const char * keyword , 
                                          const char * unit    , 
                                          const char * lgr , 
                                          const char * key_join_string , 
                                          int   lgr_i, int lgr_j , int lgr_k,
                                          int param_index , float default_value) {
  
  smspec_node_type * smspec_node = smspec_node_alloc_new( param_index , default_value );
  if (smspec_node_init_lgr( smspec_node , var_type , wgname , keyword , lgr , unit , key_join_string , lgr_i, lgr_j , lgr_k))
    return smspec_node;
  else {
    smspec_node_free( smspec_node );
    return NULL;
  }
}


void smspec_node_free( smspec_node_type * index ) {
  util_safe_free( index->unit );
  util_safe_free( index->keyword );
  util_safe_free( index->ijk );
  util_safe_free( index->gen_key1 );
  util_safe_free( index->gen_key2 );
  util_safe_free( index->wgname );
  util_safe_free( index->lgr_name );
  util_safe_free( index->lgr_ijk );
  free( index );
}

void smspec_node_free__( void * arg ) {
  smspec_node_type * node = smspec_node_safe_cast( arg );
  smspec_node_free( node );
}


/*****************************************************************/



int smspec_node_get_params_index( const smspec_node_type * smspec_node ) {
  return smspec_node->params_index;
}


void smspec_node_set_params_index( smspec_node_type * smspec_node , int params_index) {
  smspec_node->params_index = params_index;
}

const char * smspec_node_get_gen_key1( const smspec_node_type * smspec_node) {
  return smspec_node->gen_key1;
}

const char * smspec_node_get_gen_key2( const smspec_node_type * smspec_node) {
  return smspec_node->gen_key2;
}


const char * smspec_node_get_wgname( const smspec_node_type * smspec_node) {
  return smspec_node->wgname;
}

const char * smspec_node_get_keyword( const smspec_node_type * smspec_node) {
  return smspec_node->keyword;
}



ecl_smspec_var_type smspec_node_get_var_type( const smspec_node_type * smspec_node) {
  return smspec_node->var_type;
}

int smspec_node_get_num( const smspec_node_type * smspec_node) {
  return smspec_node->num;
}

bool smspec_node_is_rate( const smspec_node_type * smspec_node ) {
  return smspec_node->rate_variable;
}


bool smspec_node_is_total( const smspec_node_type * smspec_node ){ 
  return smspec_node->total_variable;
}

bool smspec_node_is_historical( const smspec_node_type * smspec_node ){ 
  return smspec_node->historical;
}


const char  * smspec_node_get_unit( const smspec_node_type * smspec_node) {
  return smspec_node->unit;
}

void smspec_node_set_unit( smspec_node_type * smspec_node , const char * unit ) {
  // ECLIPSE Standard: Max eight characters - everything beyond is silently dropped
  util_safe_free( smspec_node->unit );
  smspec_node->unit = util_alloc_substring_copy( unit , 0 , 8);       
}






bool smspec_node_need_nums( const smspec_node_type * smspec_node ) {
  return smspec_node->need_nums;
}

void smspec_node_fprintf( const smspec_node_type * smspec_node , FILE * stream) {
  fprintf(stream, "KEYWORD: %s \n",smspec_node->keyword);
  fprintf(stream, "WGNAME : %s \n",smspec_node->wgname);
  fprintf(stream, "UNIT   : %s \n",smspec_node->unit);
}
