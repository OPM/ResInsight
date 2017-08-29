/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_sum.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <locale.h>

#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/set.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/stringlist.h>
#include <ert/util/time_interval.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_smspec.h>
#include <ert/ecl/ecl_sum_data.h>
#include <ert/ecl/smspec_node.h>


/**
   The ECLIPSE summary data is organised in a header file (.SMSPEC)
   and the actual summary data. This file implements a data structure
   ecl_sum_type which holds ECLIPSE summary data. Most of the actual
   implementation is in separate files ecl_smspec.c for the SMSPEC
   header, and ecl_sum_data for the actual data.

   Observe that this datastructure is built up around internalizing
   ECLIPSE summary data, the code has NO AMBITION of being able to
   write summary data.


   Header in ECLIPSE.SMSPEC file                  Actual data; many 'PARAMS' blocks in .Snnnn or .UNSMRY file

   ------------------------------.........       -----------   -----------   -----------   -----------   -----------
   | WGNAMES    KEYWORDS   NUMS | INDEX  :       | PARAMS  |   | PARAMS  |   | PARAMS  |   | PARAMS  |   | PARAMS  |
   |----------------------------|........:       |---------|   |---------|   |---------|   |---------|   |---------|
   | OP-1       WOPR       X    |   0    :       |  652    |   |   752   |   |  862    |   |   852   |   |    962  |
   | OP-1       WWPR       X    |   1    :       |   45    |   |    47   |   |   55    |   |    59   |   |     62  |
   | GI-1       WGIR       X    |   2    :       |  500    |   |   500   |   |  786    |   |   786   |   |    486  |
   | :+:+:+:+   FOPT       X    |   3    :       | 7666    |   |  7666   |   | 8811    |   |  7688   |   |   8649  |
   | :+:+:+:+   RPR        5    |   4    :       |  255    |   |   255   |   |  266    |   |   257   |   |    277  |
   | :+:+:+:+   BPR        3457 |   5    :       |  167    |   |   167   |   |  189    |   |   201   |   |    166  |
   ------------------------------.........       -----------   -----------   -----------   -----------   -----------

                                                 <------------------------ Time direction ------------------------->

   As illustrated in the figure above header information is stored in
   the SMSPEC file; the header information is organised in several
   keywords; at least the WGNAMES and KEYWORDS arrays, and often also
   the NUMS array. Together these three arrays uniquely specify a
   summary vector.

   The INDEX column in the header information is NOT part of the
   SMSPEC file, but an important part of the ecl_smspec
   implementation. The the values from WGNAMES/KEYWORDS/NUMS are
   combined to create a unique key; and the corresponding index is
   used to lookup a numerical value from the PARAMS vector with actual
   data.

   These matters are documented further in the ecl_smspec.c and
   ecl_sum_data.c files.
*/




#define ECL_SUM_ID          89067

/*****************************************************************/

struct ecl_sum_struct {
  UTIL_TYPE_ID_DECLARATION;
  ecl_smspec_type   * smspec;     /* Internalized version of the SMSPEC file. */
  ecl_sum_data_type * data;       /* The data - can be NULL. */


  bool                fmt_case;
  bool                unified;
  char              * key_join_string;
  char              * path;       /* The path - as given for the case input. Can be NULL for cwd. */
  char              * abs_path;   /* Absolute path. */
  char              * base;       /* Only the basename. */
  char              * ecl_case;   /* This is the current case, with optional path component. == path + base*/
  char              * ext;        /* Only to support selective loading of formatted|unformatted and unified|multiple. (can be NULL) */
};


UTIL_SAFE_CAST_FUNCTION( ecl_sum , ECL_SUM_ID );
UTIL_IS_INSTANCE_FUNCTION( ecl_sum , ECL_SUM_ID );



/**
   Reads the data from ECLIPSE summary files, can either be a list of
   files BASE.S0000, BASE.S0001, BASE.S0002,.. or one unified
   file. Formatted/unformatted is detected automagically.

   The actual loading is implemented in the ecl_sum_data.c file.
*/

void ecl_sum_set_case( ecl_sum_type * ecl_sum , const char * ecl_case) {
  util_safe_free( ecl_sum->ecl_case );
  util_safe_free( ecl_sum->path );
  util_safe_free( ecl_sum->abs_path );
  util_safe_free( ecl_sum->base );
  util_safe_free( ecl_sum->ext );
  {
    char  *path , *base, *ext;

    util_alloc_file_components( ecl_case , &path , &base , &ext);

    ecl_sum->ecl_case = util_alloc_string_copy( ecl_case );
    ecl_sum->path     = util_alloc_string_copy( path );
    ecl_sum->base     = util_alloc_string_copy( base );
    ecl_sum->ext      = util_alloc_string_copy( ext );
    if (path != NULL)
      ecl_sum->abs_path = util_alloc_abs_path( path );
    else
      ecl_sum->abs_path = util_alloc_cwd();

    util_safe_free( base );
    util_safe_free( path );
    util_safe_free( ext );
  }
}


static ecl_sum_type * ecl_sum_alloc__( const char * input_arg , const char * key_join_string) {
  ecl_sum_type * ecl_sum = util_malloc( sizeof * ecl_sum );
  UTIL_TYPE_ID_INIT( ecl_sum , ECL_SUM_ID );

  ecl_sum->ecl_case  = NULL;
  ecl_sum->path      = NULL;
  ecl_sum->base      = NULL;
  ecl_sum->ext       = NULL;
  ecl_sum->abs_path  = NULL;
  ecl_sum_set_case( ecl_sum , input_arg );
  ecl_sum->key_join_string = util_alloc_string_copy( key_join_string );

  ecl_sum->smspec = NULL;
  ecl_sum->data   = NULL;

  return ecl_sum;
}


static bool ecl_sum_fread_data( ecl_sum_type * ecl_sum , const stringlist_type * data_files , bool include_restart) {
  if (ecl_sum->data != NULL)
    ecl_sum_free_data( ecl_sum );

  ecl_sum->data = ecl_sum_data_alloc( ecl_sum->smspec );
  if (ecl_sum_data_fread( ecl_sum->data , data_files )) {
    if (include_restart) {

    }
    return true;
  } else
    return false;
}


static void ecl_sum_fread_history( ecl_sum_type * ecl_sum ) {
  ecl_sum_type * history = ecl_sum_fread_alloc_case__( ecl_smspec_get_restart_case( ecl_sum->smspec ) , ":" , true);
  if (history) {
    ecl_sum_data_add_case(ecl_sum->data , history->data );
    ecl_sum_free( history );
  }
}



static bool ecl_sum_fread(ecl_sum_type * ecl_sum , const char *header_file , const stringlist_type *data_files , bool include_restart) {
  ecl_sum->smspec = ecl_smspec_fread_alloc( header_file , ecl_sum->key_join_string , include_restart);
  if (ecl_sum->smspec) {
    bool fmt_file;
    ecl_util_get_file_type( header_file , &fmt_file , NULL);
    ecl_sum_set_fmt_case( ecl_sum , fmt_file );
  } else
    return false;

  if (ecl_sum_fread_data( ecl_sum , data_files , include_restart )) {
    ecl_file_enum file_type = ecl_util_get_file_type( stringlist_iget( data_files , 0 ) , NULL , NULL);

    if (file_type == ECL_SUMMARY_FILE)
      ecl_sum_set_unified( ecl_sum , false );
    else if (file_type == ECL_UNIFIED_SUMMARY_FILE)
      ecl_sum_set_unified( ecl_sum , true);
    else
      util_abort("%s: what the fuck? \n",__func__);
  } else
    return false;

  if (include_restart && ecl_smspec_get_restart_case( ecl_sum->smspec ))
    ecl_sum_fread_history( ecl_sum );

  return true;
}


static bool ecl_sum_fread_case( ecl_sum_type * ecl_sum , bool include_restart) {
  char * header_file;
  stringlist_type * summary_file_list = stringlist_alloc_new();

  bool caseOK = false;

  ecl_util_alloc_summary_files( ecl_sum->path , ecl_sum->base , ecl_sum->ext , &header_file , summary_file_list );
  if ((header_file != NULL) && (stringlist_get_size( summary_file_list ) > 0)) {
    caseOK = ecl_sum_fread( ecl_sum , header_file , summary_file_list , include_restart );
  }
  util_safe_free( header_file );
  stringlist_free( summary_file_list );

  return caseOK;
}


/**
   This will explicitly load the summary specified by @header_file and
   @data_files, i.e. if the case has been restarted from another case,
   it will NOT look for old summary information - that functionality
   is only invoked when using ecl_sum_fread_alloc_case() function;
   however the list of data_files could in principle be achieved by
   initializing the data_files list with files from the restarted
   case.
*/


ecl_sum_type * ecl_sum_fread_alloc(const char *header_file , const stringlist_type *data_files , const char * key_join_string) {
  ecl_sum_type * ecl_sum = ecl_sum_alloc__( header_file , key_join_string );
  ecl_sum_fread( ecl_sum , header_file , data_files , false );
  return ecl_sum;
}

/*****************************************************************/

void ecl_sum_set_unified( ecl_sum_type * ecl_sum , bool unified ) {
  ecl_sum->unified = unified;
}


void ecl_sum_set_fmt_case( ecl_sum_type * ecl_sum , bool fmt_case ) {
  ecl_sum->fmt_case = fmt_case;
}


void ecl_sum_init_var( ecl_sum_type * ecl_sum , smspec_node_type * smspec_node , const char * keyword , const char * wgname , int num , const char * unit) {
  ecl_smspec_init_var( ecl_sum->smspec , smspec_node , keyword , wgname , num, unit );
}


smspec_node_type * ecl_sum_add_var( ecl_sum_type * ecl_sum , const char * keyword , const char * wgname , int num , const char * unit , float default_value) {
  smspec_node_type * smspec_node = ecl_sum_add_blank_var( ecl_sum , default_value );
  ecl_sum_init_var( ecl_sum , smspec_node , keyword , wgname , num , unit );
  return smspec_node;
}


smspec_node_type * ecl_sum_add_blank_var( ecl_sum_type * ecl_sum , float default_value) {
  smspec_node_type * smspec_node = smspec_node_alloc_new( -1 , default_value );
  ecl_smspec_add_node( ecl_sum->smspec , smspec_node );
  return smspec_node;
}



/*
  Observe the time argument in ecl_sum_add_tstep() and the bool flag
  time_in_days in ecl_sum_alloc_writer() can be misleading:

  - The time argument 'sim_seconds' to ecl_sum_add_tstep() should
    *ALWAYS* be in seconds.

  - The 'sim_in_days' argument to the ecl_sum_alloc_writer( ) is just
    a very very basic unit support in the output. If sim_in_days ==
    true the output time unit will be days, otherwise it will be hours.
*/

ecl_sum_tstep_type * ecl_sum_add_tstep( ecl_sum_type * ecl_sum , int report_step , double sim_seconds) {
  return ecl_sum_data_add_new_tstep( ecl_sum->data , report_step , sim_seconds );
}


ecl_sum_type * ecl_sum_alloc_restart_writer( const char * ecl_case , const char * restart_case , bool fmt_output , bool unified , const char * key_join_string , time_t sim_start , bool time_in_days , int nx , int ny , int nz) {
  
  ecl_sum_type * ecl_sum = ecl_sum_alloc__( ecl_case , key_join_string );
  ecl_sum_set_unified( ecl_sum , unified );
  ecl_sum_set_fmt_case( ecl_sum , fmt_output );

  ecl_sum->smspec = ecl_smspec_alloc_writer( key_join_string , restart_case, sim_start , time_in_days , nx , ny , nz );
  ecl_sum->data   = ecl_sum_data_alloc_writer( ecl_sum->smspec );

  return ecl_sum;
}

ecl_sum_type * ecl_sum_alloc_writer( const char * ecl_case , bool fmt_output , bool unified , const char * key_join_string , time_t sim_start , bool time_in_days , int nx , int ny , int nz) {
  return ecl_sum_alloc_restart_writer(ecl_case, NULL, fmt_output, unified, key_join_string, sim_start, time_in_days, nx, ny, nz);
}

void ecl_sum_fwrite( const ecl_sum_type * ecl_sum ) {
  ecl_sum_fwrite_smspec( ecl_sum );
  ecl_sum_data_fwrite( ecl_sum->data , ecl_sum->ecl_case , ecl_sum->fmt_case , ecl_sum->unified );
}


void ecl_sum_fwrite_smspec( const ecl_sum_type * ecl_sum ) {
  ecl_smspec_fwrite( ecl_sum->smspec , ecl_sum->ecl_case , ecl_sum->fmt_case );
}

/*****************************************************************/


/**
   This function frees the data from the ecl_sum instance and sets the
   data pointer to NULL. The SMSPEC data is still valid, and can be
   reused with calls to ecl_sum_fread_realloc_data().
*/

void ecl_sum_free_data( ecl_sum_type * ecl_sum ) {
  ecl_sum_data_free( ecl_sum->data );
  ecl_sum->data = NULL;
}


void ecl_sum_free( ecl_sum_type * ecl_sum ) {
  if (ecl_sum->data != NULL)
    ecl_sum_free_data( ecl_sum );

  if (ecl_sum->smspec != NULL)
    ecl_smspec_free( ecl_sum->smspec );

  util_safe_free( ecl_sum->path );
  util_safe_free( ecl_sum->ext );
  util_safe_free( ecl_sum->abs_path );

  free( ecl_sum->base );
  free( ecl_sum->ecl_case );

  free( ecl_sum->key_join_string );
  free( ecl_sum );
}



void ecl_sum_free__(void * __ecl_sum) {
  ecl_sum_type * ecl_sum = ecl_sum_safe_cast( __ecl_sum);
  ecl_sum_free( ecl_sum );
}




/**
   This function takes an input file, and loads the corresponding
   summary. The function extracts the path part, and the basename from
   the input file. The extension is not considered (the input need not
   even be a valid file). In principle a simulation directory with a
   given basename can contain four different simulation cases:

    * Formatted and unformatted.
    * Unified and not unified.

   The program will load the most recent dataset, by looking at the
   modification time stamps of the files; if no simulation case is
   found the function will return NULL.

   If the SMSPEC file contains the RESTART keyword the function will
   iterate backwards to load summary information from previous runs
   (this is goverened by the local variable include_restart).
*/


ecl_sum_type * ecl_sum_fread_alloc_case__(const char * input_file , const char * key_join_string , bool include_restart){
  ecl_sum_type * ecl_sum     = ecl_sum_alloc__(input_file , key_join_string);
  if (ecl_sum_fread_case( ecl_sum , include_restart))
    return ecl_sum;
  else {
    /*
      Loading a case failed - we discard the partly initialized
      ecl_sum structure and jump ship.
    */
    ecl_sum_free( ecl_sum );
    return NULL;
  }
}



ecl_sum_type * ecl_sum_fread_alloc_case(const char * input_file , const char * key_join_string){
  bool include_restart = true;
  return ecl_sum_fread_alloc_case__( input_file , key_join_string , include_restart );
}


bool ecl_sum_case_exists( const char * input_file ) {
  char * smspec_file = NULL;
  stringlist_type * data_files = stringlist_alloc_new();
  char * path;
  char * basename;
  char * extension;
  bool   case_exists;

  util_alloc_file_components( input_file , &path , &basename , &extension);
  case_exists = ecl_util_alloc_summary_files( path , basename , extension , &smspec_file , data_files );

  util_safe_free( path );
  util_safe_free( basename );
  util_safe_free( extension );
  util_safe_free( smspec_file );
  stringlist_free( data_files );

  return case_exists;
}


/*****************************************************************/

double ecl_sum_get_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const smspec_node_type * node) {
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , node );
}

double ecl_sum_get_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const smspec_node_type * node) {
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , node );
}

double ecl_sum_time2days( const ecl_sum_type * ecl_sum , time_t sim_time) {
  return ecl_sum_data_time2days( ecl_sum->data , sim_time );
}



/*****************************************************************/
/*
   Here comes lots of access functions - these are mostly thin
   wrapppers around ecl_smspec functions. See more 'extensive'
   documentation in ecl_smspec.c

   The functions returning an actual value,
   i.e. ecl_sum_get_well_var() will trustingly call ecl_sum_data_get()
   with whatever indices it gets. If the indices are invalid -
   ecl_sum_data_get() will abort. The abort is the 'correct'
   behaviour, but it is possible to abort in this scope as well, in
   that case more informative error message can be supplied (i.e. the
   well/variable B-33T2/WOPR does not exist, instead of just "invalid
   index" which is the best ecl_sum_data_get() can manage.).
*/

/*****************************************************************/
/* Well variables */

bool    ecl_sum_has_well_var(const ecl_sum_type * ecl_sum , const char * well , const char *var) {
  return ecl_smspec_has_well_var(ecl_sum->smspec , well , var);
}

double  ecl_sum_get_well_var(const ecl_sum_type * ecl_sum , int time_index , const char * well , const char *var) {
  int params_index = ecl_smspec_get_well_var_params_index( ecl_sum->smspec , well , var );
  return ecl_sum_data_iget( ecl_sum->data , time_index , params_index);
}

double ecl_sum_get_well_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * well , const char * var) {
  const smspec_node_type * node = ecl_smspec_get_well_var_node( ecl_sum->smspec , well , var );
  return ecl_sum_get_from_sim_time( ecl_sum , sim_time , node );
}

double ecl_sum_get_well_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * well , const char * var) {
  const smspec_node_type * node = ecl_smspec_get_well_var_node( ecl_sum->smspec , well , var );
  return ecl_sum_get_from_sim_days( ecl_sum , sim_days , node );
}


/*****************************************************************/
/* Group variables */

bool ecl_sum_has_group_var(const ecl_sum_type * ecl_sum , const char * group , const char *var) {
  return ecl_smspec_has_group_var( ecl_sum->smspec , group , var);
}

double  ecl_sum_get_group_var(const ecl_sum_type * ecl_sum , int time_index , const char * group , const char *var) {
  int params_index = ecl_smspec_get_group_var_params_index( ecl_sum->smspec , group , var );
  return ecl_sum_data_iget( ecl_sum->data , time_index , params_index);
}


double ecl_sum_get_group_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * group , const char * var) {
  const smspec_node_type * node = ecl_smspec_get_group_var_node( ecl_sum->smspec , group , var );
  return ecl_sum_get_from_sim_time( ecl_sum , sim_time , node );
}

double ecl_sum_get_group_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * group , const char * var) {
  const smspec_node_type * node = ecl_smspec_get_group_var_node( ecl_sum->smspec , group , var );
  return ecl_sum_get_from_sim_days( ecl_sum , sim_days , node );
}


/*****************************************************************/
/* Field variables */
bool ecl_sum_has_field_var(const ecl_sum_type * ecl_sum , const char *var) {
  return ecl_smspec_has_field_var( ecl_sum->smspec , var);
}

double ecl_sum_get_field_var(const ecl_sum_type * ecl_sum , int time_index , const char * var) {
  int params_index = ecl_smspec_get_field_var_params_index( ecl_sum->smspec ,  var );
  return ecl_sum_data_iget( ecl_sum->data , time_index , params_index);
}

double ecl_sum_get_field_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  const smspec_node_type * node = ecl_smspec_get_field_var_node( ecl_sum->smspec , var );
  return ecl_sum_get_from_sim_time( ecl_sum , sim_time , node );
}

double ecl_sum_get_field_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  const smspec_node_type * node = ecl_smspec_get_field_var_node( ecl_sum->smspec , var );
  return ecl_sum_get_from_sim_days( ecl_sum , sim_days , node );
}


/*****************************************************************/
/* Block variables */

bool ecl_sum_has_block_var(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr) {
  return ecl_smspec_has_block_var( ecl_sum->smspec , block_var , block_nr );
}

double ecl_sum_get_block_var(const ecl_sum_type * ecl_sum , int time_index , const char * block_var , int block_nr) {
  int params_index = ecl_smspec_get_block_var_params_index( ecl_sum->smspec ,  block_var , block_nr);
  return ecl_sum_data_iget( ecl_sum->data , time_index , params_index);
}

int  ecl_sum_get_block_var_index_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i, int j , int k ) {
  return ecl_smspec_get_block_var_params_index_ijk( ecl_sum->smspec , block_var , i , j , k);
}

bool ecl_sum_has_block_var_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i, int j , int k) {
  return ecl_smspec_has_block_var_ijk( ecl_sum->smspec , block_var , i ,j , k);
}

double ecl_sum_get_block_var_ijk(const ecl_sum_type * ecl_sum , int time_index , const char * block_var , int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum ,  block_var , i , j , k);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

double ecl_sum_get_block_var_ijk_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * block_var, int i , int j , int k) {
  const smspec_node_type * node = ecl_smspec_get_block_var_node_ijk( ecl_sum->smspec , block_var , i ,j  , k);
  return ecl_sum_get_from_sim_time( ecl_sum , sim_time , node );
}

double ecl_sum_get_block_var_ijk_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * block_var, int i , int j , int k) {
  const smspec_node_type * node = ecl_smspec_get_block_var_node_ijk( ecl_sum->smspec , block_var , i ,j  , k);
  return ecl_sum_get_from_sim_days( ecl_sum , sim_days , node );
}



/*****************************************************************/
/* Region variables */
/**
   region_nr: [1...num_regions] (NOT C-based indexing)
*/

bool ecl_sum_has_region_var(const ecl_sum_type * ecl_sum , const char *var , int region_nr ) {
  return ecl_smspec_has_region_var( ecl_sum->smspec , var , region_nr);
}

double ecl_sum_get_region_var(const ecl_sum_type * ecl_sum , int time_index , const char *var , int region_nr) {
  int params_index = ecl_smspec_get_region_var_params_index( ecl_sum->smspec ,  var , region_nr);
  return ecl_sum_data_iget( ecl_sum->data , time_index , params_index);
}

double ecl_sum_get_region_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var , int region_nr) {
  const smspec_node_type * node = ecl_smspec_get_region_var_node( ecl_sum->smspec , var , region_nr);
  return ecl_sum_get_from_sim_time( ecl_sum , sim_time , node );
}

double ecl_sum_get_region_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var , int region_nr) {
  const smspec_node_type * node = ecl_smspec_get_region_var_node( ecl_sum->smspec , var , region_nr);
  return ecl_sum_get_from_sim_days( ecl_sum , sim_days , node );
}



/*****************************************************************/
/* Misc variables */

int ecl_sum_get_misc_var_index(const ecl_sum_type * ecl_sum , const char *var) {
  return ecl_smspec_get_misc_var_params_index( ecl_sum->smspec , var );
}

bool ecl_sum_has_misc_var(const ecl_sum_type * ecl_sum , const char *var) {
  return ecl_smspec_has_misc_var( ecl_sum->smspec , var );
}

double  ecl_sum_get_misc_var(const ecl_sum_type * ecl_sum , int time_index , const char *var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum ,  var);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}


/*****************************************************************/
/* Well completion - not fully implemented ?? */

int ecl_sum_get_well_completion_var_index(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr) {
  return ecl_smspec_get_well_completion_var_params_index( ecl_sum->smspec , well , var , cell_nr);
}

bool ecl_sum_has_well_completion_var(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr)  {
  return ecl_smspec_has_well_completion_var( ecl_sum->smspec , well , var , cell_nr);
}

double ecl_sum_get_well_completion_var(const ecl_sum_type * ecl_sum , int time_index , const char * well , const char *var, int cell_nr)  {
  int index = ecl_sum_get_well_completion_var_index(ecl_sum , well , var , cell_nr);
  return ecl_sum_data_iget( ecl_sum->data , time_index , index);
}

/*****************************************************************/
/* General variables - this means WWCT:OP_1 - i.e. composite variables*/

const smspec_node_type * ecl_sum_get_general_var_node(const ecl_sum_type * ecl_sum , const char * lookup_kw) {
  const smspec_node_type * node = ecl_smspec_get_general_var_node( ecl_sum->smspec , lookup_kw );
  if (node != NULL)
    return node;
  else {
    util_abort("%s: summary case:%s does not contain key:%s\n",__func__ , ecl_sum_get_case( ecl_sum ) , lookup_kw );
    return NULL;
  }
}

int ecl_sum_get_general_var_params_index(const ecl_sum_type * ecl_sum , const char * lookup_kw) {
  return ecl_smspec_get_general_var_params_index( ecl_sum->smspec , lookup_kw );
}


bool ecl_sum_has_general_var(const ecl_sum_type * ecl_sum , const char * lookup_kw) {
  return ecl_smspec_has_general_var( ecl_sum->smspec , lookup_kw);
}

bool ecl_sum_has_key(const ecl_sum_type * ecl_sum , const char * lookup_kw) {
  return ecl_sum_has_general_var( ecl_sum , lookup_kw );
}


double ecl_sum_get_general_var(const ecl_sum_type * ecl_sum , int time_index , const char * lookup_kw) {
  int params_index = ecl_sum_get_general_var_params_index(ecl_sum , lookup_kw);
  return ecl_sum_data_iget( ecl_sum->data , time_index  , params_index);
}


void ecl_sum_fwrite_interp_csv_line(const ecl_sum_type * ecl_sum, time_t sim_time, const ecl_sum_vector_type * key_words, FILE *fp){
  ecl_sum_data_fwrite_interp_csv_line(ecl_sum->data, sim_time, key_words, fp);
}



double ecl_sum_get_general_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , var );
  return ecl_sum_get_from_sim_time( ecl_sum , sim_time , node );
}

double ecl_sum_get_general_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , node );
}


const char * ecl_sum_get_general_var_unit( const ecl_sum_type * ecl_sum , const char * var) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , var );
  return smspec_node_get_unit( node );
}

/*****************************************************************/

double ecl_sum_iget( const ecl_sum_type * ecl_sum , int time_index , int param_index) {
  return ecl_sum_data_iget(ecl_sum->data , time_index , param_index);
}

/*****************************************************************/
/* Simple get functions which take a general var key as input    */

bool ecl_sum_var_is_rate( const ecl_sum_type * ecl_sum , const char * gen_key) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_is_rate( node );
}

bool ecl_sum_var_is_total( const ecl_sum_type * ecl_sum , const char * gen_key) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_is_total( node );
}


ecl_smspec_var_type ecl_sum_identify_var_type( const char * var ) {
  return ecl_smspec_identify_var_type( var );
}

ecl_smspec_var_type ecl_sum_get_var_type( const ecl_sum_type * ecl_sum , const char * gen_key) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_get_var_type( node );
}

const char * ecl_sum_get_unit( const ecl_sum_type * ecl_sum , const char * gen_key) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_get_unit( node );
}

int ecl_sum_get_num( const ecl_sum_type * ecl_sum , const char * gen_key ) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_get_num( node );
}

const char * ecl_sum_get_wgname( const ecl_sum_type * ecl_sum , const char * gen_key ) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_get_wgname( node );
}

const char * ecl_sum_get_keyword( const ecl_sum_type * ecl_sum , const char * gen_key ) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key );
  return smspec_node_get_keyword( node );
}


/*****************************************************************/
/*
   Here comes a couple of functions relating to the time
   dimension. The functions here in this file are just thin wrappers
   of 'real' functions located in ecl_sum_data.c.
*/



bool  ecl_sum_has_report_step(const ecl_sum_type * ecl_sum , int report_step ) {
  return ecl_sum_data_has_report_step( ecl_sum->data , report_step );
}

int ecl_sum_get_last_report_step( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_last_report_step( ecl_sum->data );
}

int ecl_sum_get_first_report_step( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_first_report_step( ecl_sum->data );
}

int ecl_sum_iget_report_end( const ecl_sum_type * ecl_sum, int report_step) {
  return ecl_sum_data_iget_report_end(ecl_sum->data , report_step );
}

int ecl_sum_iget_report_start( const ecl_sum_type * ecl_sum, int report_step) {
  return ecl_sum_data_iget_report_start(ecl_sum->data , report_step );
}

int ecl_sum_iget_report_step( const ecl_sum_type * ecl_sum , int internal_index ){
  return ecl_sum_data_iget_report_step( ecl_sum->data , internal_index );
}


int ecl_sum_iget_mini_step( const ecl_sum_type * ecl_sum , int internal_index ){
  return ecl_sum_data_iget_mini_step( ecl_sum->data , internal_index );
}


void ecl_sum_init_time_vector( const ecl_sum_type * ecl_sum , time_t_vector_type * time_vector , bool report_only ) {
  ecl_sum_data_init_time_vector( ecl_sum->data , time_vector , report_only );
}


time_t_vector_type * ecl_sum_alloc_time_vector( const ecl_sum_type * ecl_sum  , bool report_only) {
  return ecl_sum_data_alloc_time_vector( ecl_sum->data , report_only );
}

void ecl_sum_init_data_vector( const ecl_sum_type * ecl_sum , double_vector_type * data_vector , int data_index , bool report_only ) {
  ecl_sum_data_init_data_vector( ecl_sum->data , data_vector , data_index , report_only );
}


double_vector_type * ecl_sum_alloc_data_vector( const ecl_sum_type * ecl_sum  , int data_index , bool report_only) {
  return ecl_sum_data_alloc_data_vector( ecl_sum->data , data_index , report_only );
}



void ecl_sum_summarize( const ecl_sum_type * ecl_sum , FILE * stream ) {
  ecl_sum_data_summarize( ecl_sum->data , stream );
}



/**
   Returns the first internal index where a limiting value is
   reached. If the limiting value is never reached, -1 is
   returned. The smspec_index should be calculated first with one of
   the

      ecl_sum_get_XXXX_index()

   functions. I.e. the following code will give the first index where
   the water wut in well PX exceeds 0.25:

   {
      int smspec_index   = ecl_sum_get_well_var( ecl_sum , "PX" , "WWCT" );
      int first_index    = ecl_sum_get_first_gt( ecl_sum , smspec_index , 0.25);
   }

*/


static int ecl_sum_get_limiting(const ecl_sum_type * ecl_sum , int smspec_index , double limit , bool gt) {
  const int length        = ecl_sum_data_get_length( ecl_sum->data );
  int internal_index      = 0;
  do {
    double value = ecl_sum_data_iget( ecl_sum->data , internal_index , smspec_index );
    if (gt) {
      if (value > limit)
        break;
    } else {
      if (value < limit)
        break;
    }
    internal_index++;
  } while (internal_index < length);

  if (internal_index == length)  /* Did not find it */
    internal_index = -1;

  return internal_index;
}

int ecl_sum_get_first_gt( const ecl_sum_type * ecl_sum , int param_index , double limit) {
  return ecl_sum_get_limiting( ecl_sum , param_index , limit , true );
}

int ecl_sum_get_first_lt( const ecl_sum_type * ecl_sum , int param_index , double limit) {
  return ecl_sum_get_limiting( ecl_sum , param_index , limit , false );
}



time_t ecl_sum_get_report_time( const ecl_sum_type * ecl_sum , int report_step ) {
  return ecl_sum_data_get_report_time( ecl_sum->data , report_step );
}

time_t ecl_sum_iget_sim_time( const ecl_sum_type * ecl_sum , int index ) {
  return ecl_sum_data_iget_sim_time( ecl_sum->data , index );
}

const time_interval_type * ecl_sum_get_sim_time( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_sim_time( ecl_sum->data );
}

time_t ecl_sum_get_data_start( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_data_start( ecl_sum->data );
}

time_t ecl_sum_get_start_time( const ecl_sum_type * ecl_sum) {
  return ecl_smspec_get_start_time( ecl_sum->smspec );
}

time_t ecl_sum_get_end_time( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_sim_end( ecl_sum->data );
}

double ecl_sum_iget_sim_days( const ecl_sum_type * ecl_sum , int index ) {
  return ecl_sum_data_iget_sim_days( ecl_sum->data , index );
}


/*****************************************************************/
/* This is essentially the summary.x program. */



void ecl_sum_fmt_init_summary_x( const ecl_sum_type * ecl_sum , ecl_sum_fmt_type * fmt ) {
  fmt->locale     = NULL;
  fmt->sep        = "";
  fmt->date_fmt   = "%d/%m/%Y   ";
  fmt->value_fmt  = " %15.6g ";

  if (util_string_equal( ecl_sum_get_unit( ecl_sum , "TIME") , "DAYS"))
    fmt->days_fmt   = "%7.2f   ";
  else
    fmt->days_fmt   = "%7.4f   ";

  fmt->header_fmt = " %15s ";

  fmt->newline = "\n";
  fmt->print_header= true;
  fmt->print_dash = true;
  fmt->date_dash  = "-----------------------";
  fmt->value_dash = "-----------------";
  fmt->date_header= "-- Days   dd/mm/yyyy   ";
}


/*#define DAYS_DATE_FORMAT    "%7.2f   %02d/%02d/%04d   "
#define FLOAT_FORMAT        " %15.6g "
#define HEADER_FORMAT       " %15s "
#define DATE_DASH           "-----------------------"
#define FLOAT_DASH          "-----------------"
*/

#define DATE_HEADER         "-- Days   dd/mm/yyyy   "
#define DATE_STRING_LENGTH 128

static void __ecl_sum_fprintf_line( const ecl_sum_type * ecl_sum , FILE * stream , int internal_index , const bool_vector_type * has_var , const int_vector_type * var_index , char * date_string , const ecl_sum_fmt_type * fmt) {
  fprintf(stream , fmt->days_fmt , ecl_sum_iget_sim_days(ecl_sum , internal_index));
  fprintf(stream , "%s", fmt->sep );

  {
    struct tm ts;
    time_t sim_time = ecl_sum_iget_sim_time(ecl_sum , internal_index );
    util_time_utc( &sim_time , &ts);
    strftime( date_string , DATE_STRING_LENGTH - 1 , fmt->date_fmt , &ts);
    fprintf(stream , "%s", date_string );
  }

  {
    int ivar;
    for (ivar = 0; ivar < int_vector_size( var_index ); ivar++) {
      if (bool_vector_iget( has_var , ivar )) {
        fprintf(stream , "%s", fmt->sep);
        fprintf(stream , fmt->value_fmt , ecl_sum_iget(ecl_sum , internal_index, int_vector_iget( var_index , ivar )));
      }
    }
  }

  fprintf(stream , "%s", fmt->newline);
}


static void ecl_sum_fprintf_header( const ecl_sum_type * ecl_sum , const stringlist_type * key_list , const bool_vector_type * has_var , FILE * stream, const ecl_sum_fmt_type * fmt) {
  fprintf(stream , "%s", fmt->date_header);
  {
    int i;
    for (i=0; i < stringlist_get_size( key_list ); i++)
      if (bool_vector_iget( has_var , i )) {
        fprintf(stream , "%s", fmt->sep);
        fprintf(stream , fmt->header_fmt , stringlist_iget( key_list , i ));
      }
  }

  fprintf( stream , "%s", fmt->newline);
  if (fmt->print_dash)   {
    fprintf(stream , "%s", fmt->date_dash);

    {
      int i;
      for (i=0; i < stringlist_get_size( key_list ); i++)
        if (bool_vector_iget( has_var , i ))
          fprintf(stream , "%s", fmt->value_dash);
    }
    fprintf( stream , "%s", fmt->newline);
  }
}



void ecl_sum_fprintf(const ecl_sum_type * ecl_sum , FILE * stream , const stringlist_type * var_list , bool report_only , const ecl_sum_fmt_type * fmt) {
  bool_vector_type  * has_var   = bool_vector_alloc( stringlist_get_size( var_list ), false );
  int_vector_type   * var_index = int_vector_alloc( stringlist_get_size( var_list ), -1 );
  char * date_string            = util_malloc( DATE_STRING_LENGTH * sizeof * date_string);

  char * current_locale = NULL;
  if (fmt->locale != NULL)
    current_locale = setlocale(LC_NUMERIC , fmt->locale);

  {
    int ivar;
    for (ivar = 0; ivar < stringlist_get_size( var_list ); ivar++) {
      if (ecl_sum_has_general_var( ecl_sum , stringlist_iget( var_list , ivar) )) {
        bool_vector_iset( has_var , ivar , true );
        int_vector_iset( var_index , ivar , ecl_sum_get_general_var_params_index( ecl_sum , stringlist_iget( var_list , ivar) ));
      } else {
        fprintf(stderr,"** Warning: could not find variable: \'%s\' in summary file \n", stringlist_iget( var_list , ivar));
        bool_vector_iset( has_var , ivar , false );
      }
    }
  }

  if (fmt->print_header)
    ecl_sum_fprintf_header( ecl_sum , var_list , has_var , stream , fmt);

  if (report_only) {
    int first_report = ecl_sum_get_first_report_step( ecl_sum );
    int last_report  = ecl_sum_get_last_report_step( ecl_sum );
    int report;

    for (report = first_report; report <= last_report; report++) {
      if (ecl_sum_data_has_report_step(ecl_sum->data , report)) {
        int time_index;
        time_index = ecl_sum_data_iget_report_end( ecl_sum->data , report );
        __ecl_sum_fprintf_line( ecl_sum , stream , time_index , has_var , var_index , date_string , fmt);
      }
    }
  } else {
    int time_index;
    for (time_index = 0; time_index < ecl_sum_get_data_length( ecl_sum ); time_index++)
      __ecl_sum_fprintf_line( ecl_sum , stream , time_index , has_var , var_index , date_string , fmt);
  }

  int_vector_free( var_index );
  bool_vector_free( has_var );
  if (current_locale != NULL)
    setlocale( LC_NUMERIC , current_locale);
  free( date_string );
}
#undef DATE_STRING_LENGTH




static void ecl_sum_fmt_init_csv( ecl_sum_fmt_type * fmt , const char * date_format , const char * date_header , const char * sep) {
  fmt->locale     = NULL; //"Norwegian";
  fmt->sep        = sep;
  fmt->date_fmt   = date_format;
  fmt->value_fmt  = "%g";
  fmt->days_fmt   = "%7.2f";
  fmt->header_fmt = "%s";

  fmt->newline     = "\r\n";
  fmt->date_header = date_header;
  fmt->print_header = true;
  fmt->print_dash   = false;
}


void ecl_sum_export_csv(const ecl_sum_type * ecl_sum , const char * filename  , const stringlist_type * var_list , const char * date_format , const char * sep) {
  FILE * stream = util_mkdir_fopen(filename , "w");
  char * date_header = util_alloc_sprintf("DAYS%sDATE" , sep);
  bool report_only = false;
  ecl_sum_fmt_type fmt;
  ecl_sum_fmt_init_csv( &fmt , date_format , date_header , sep );
  ecl_sum_fprintf( ecl_sum , stream , var_list , report_only , &fmt );
  fclose( stream );
  free( date_header );
}



const char * ecl_sum_get_case(const ecl_sum_type * ecl_sum) {
  return ecl_sum->ecl_case;
}


const char * ecl_sum_get_path(const ecl_sum_type * ecl_sum ) {
  return ecl_sum->path;
}


const char * ecl_sum_get_abs_path(const ecl_sum_type * ecl_sum ) {
  return ecl_sum->abs_path;
}


const char * ecl_sum_get_base(const ecl_sum_type * ecl_sum ) {
  return ecl_sum->base;
}


/**
   This function will check if the currently loaded case corresponds
   to the case specified by @input_file. The extension of @input file
   can be arbitrary (or nonexistent) and will be ignored (this can
   lead to errors with formatted/unformatted mixup if the simulation
   directory has been changed after the ecl_sum instance has been
   loaded).
*/


bool ecl_sum_same_case( const ecl_sum_type * ecl_sum , const char * input_file ) {
  bool   same_case = false;
  {
    char * path;
    char * base;

    util_alloc_file_components( input_file , &path , &base , NULL);
    {
      bool   fmt_file = ecl_smspec_get_formatted( ecl_sum->smspec );
      char * header_file = ecl_util_alloc_exfilename( path , base , ECL_SUMMARY_HEADER_FILE , fmt_file , -1 );
      if (header_file != NULL) {
        same_case = util_same_file( header_file , ecl_smspec_get_header_file( ecl_sum->smspec ));
        free( header_file );
      }
    }

    util_safe_free( path );
    util_safe_free( base );
  }
  return same_case;
}




/*****************************************************************/


stringlist_type * ecl_sum_alloc_matching_general_var_list(const ecl_sum_type * ecl_sum , const char * pattern) {
  return ecl_smspec_alloc_matching_general_var_list(ecl_sum->smspec , pattern );
}

void ecl_sum_select_matching_general_var_list( const ecl_sum_type * ecl_sum , const char * pattern , stringlist_type * keys) {
  ecl_smspec_select_matching_general_var_list( ecl_sum->smspec , pattern , keys );
}

stringlist_type * ecl_sum_alloc_well_list( const ecl_sum_type * ecl_sum , const char * pattern) {
  return ecl_smspec_alloc_well_list( ecl_sum->smspec , pattern );
}

stringlist_type * ecl_sum_alloc_group_list( const ecl_sum_type * ecl_sum , const char * pattern) {
  return ecl_smspec_alloc_group_list( ecl_sum->smspec , pattern );
}

stringlist_type * ecl_sum_alloc_well_var_list( const ecl_sum_type * ecl_sum ) {
  return ecl_smspec_alloc_well_var_list( ecl_sum->smspec );
}



/*****************************************************************/


void ecl_sum_resample_from_sim_time( const ecl_sum_type * ecl_sum , const time_t_vector_type * sim_time , double_vector_type * value , const char * gen_key) {
  const smspec_node_type * node = ecl_smspec_get_general_var_node( ecl_sum->smspec , gen_key);
  double_vector_reset( value );
  {
    int i;
    for (i=0; i < time_t_vector_size( sim_time ); i++)
      double_vector_iset( value , i , ecl_sum_data_get_from_sim_time( ecl_sum->data , time_t_vector_iget( sim_time , i ) , node));
  }
}


void ecl_sum_resample_from_sim_days( const ecl_sum_type * ecl_sum , const double_vector_type * sim_days , double_vector_type * value , const char * gen_key) {
  const smspec_node_type * node = ecl_smspec_get_general_var_node( ecl_sum->smspec , gen_key);
  double_vector_reset( value );
  {
    int i;
    for (i=0; i < double_vector_size( sim_days ); i++)
      double_vector_iset( value , i , ecl_sum_data_get_from_sim_days( ecl_sum->data , double_vector_iget( sim_days , i ) , node));
  }
}


time_t ecl_sum_time_from_days( const ecl_sum_type * ecl_sum , double sim_days ) {
  time_t t = ecl_smspec_get_start_time( ecl_sum->smspec );
  util_inplace_forward_days_utc( &t , sim_days );
  return t;
}


double ecl_sum_days_from_time( const ecl_sum_type * ecl_sum , time_t sim_time ) {
  double seconds_diff = util_difftime( ecl_smspec_get_start_time( ecl_sum->smspec ) , sim_time , NULL , NULL , NULL, NULL);
  return seconds_diff * 1.0 / (3600 * 24.0);
}


double ecl_sum_get_first_day( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_first_day( ecl_sum->data );
}

double ecl_sum_get_sim_length( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_sim_length( ecl_sum->data );
}

/**
   Will return the number of data blocks.
*/
int ecl_sum_get_data_length( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_length( ecl_sum->data );
}

void ecl_sum_scale_vector( ecl_sum_type * ecl_sum, int index, double scalar ) {
  ecl_sum_data_scale_vector( ecl_sum->data, index, scalar );
}

void ecl_sum_shift_vector( ecl_sum_type * ecl_sum, int index, double addend ) {
  ecl_sum_data_shift_vector( ecl_sum->data, index, addend );
}

bool ecl_sum_check_sim_time( const ecl_sum_type * sum , time_t sim_time) {
  return ecl_sum_data_check_sim_time( sum->data , sim_time );
}


bool ecl_sum_check_sim_days( const ecl_sum_type * sum , double sim_days) {
  return ecl_sum_data_check_sim_days( sum->data , sim_days );
}

int ecl_sum_get_report_step_from_time( const ecl_sum_type * sum , time_t sim_time) {
  return ecl_sum_data_get_report_step_from_time( sum->data , sim_time );
}


int ecl_sum_get_report_step_from_days( const ecl_sum_type * sum , double sim_days) {
  return ecl_sum_data_get_report_step_from_days( sum->data , sim_days );
}




/*****************************************************************/

const ecl_smspec_type * ecl_sum_get_smspec( const ecl_sum_type * ecl_sum ) {
  return ecl_sum->smspec;
}

void ecl_sum_update_wgname( ecl_sum_type * ecl_sum , smspec_node_type * node , const char * wgname ) {
  ecl_smspec_update_wgname( ecl_sum->smspec ,node , wgname );
}

/*****************************************************************/

/*
   The functions below are extremly simple functions which only serve
   as an easy access to the smspec_alloc_xxx_key() functions which
   know how to create the various composite keys.
*/

char * ecl_sum_alloc_well_key( const ecl_sum_type * ecl_sum , const char * keyword , const char * wgname) {
  return ecl_smspec_alloc_well_key( ecl_sum->smspec , keyword , wgname );
}



bool ecl_sum_is_oil_producer( const ecl_sum_type * ecl_sum , const char * well) {
  const char * WOPT_KEY = "WOPT";
  bool oil_producer = false;

  if (ecl_sum_has_well_var( ecl_sum , well , WOPT_KEY)) {
    int last_step = ecl_sum_get_data_length( ecl_sum ) - 1;
    double wopt = ecl_sum_get_well_var( ecl_sum , last_step , well , WOPT_KEY);

    if (wopt > 0)
      oil_producer = true;
  }

  return oil_producer;
}



bool ecl_sum_report_step_equal( const ecl_sum_type * ecl_sum1 , const ecl_sum_type * ecl_sum2) {
  if (ecl_sum1 == ecl_sum2)
    return true;
  else
    return ecl_sum_data_report_step_equal( ecl_sum1->data , ecl_sum2->data );
}


bool ecl_sum_report_step_compatible( const ecl_sum_type * ecl_sum1 , const ecl_sum_type * ecl_sum2) {
  if (ecl_sum1 == ecl_sum2)
    return true;
  else
    return ecl_sum_data_report_step_compatible( ecl_sum1->data , ecl_sum2->data );
}


double_vector_type * ecl_sum_alloc_seconds_solution( const ecl_sum_type * ecl_sum , const char * gen_key , double cmp_value , bool rates_clamp_lower) {
  const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum , gen_key);
  return ecl_sum_data_alloc_seconds_solution( ecl_sum->data , node , cmp_value , rates_clamp_lower);
}


double_vector_type * ecl_sum_alloc_days_solution( const ecl_sum_type * ecl_sum , const char * gen_key , double cmp_value , bool rates_clamp_lower) {
  double_vector_type * solution = ecl_sum_alloc_seconds_solution( ecl_sum , gen_key , cmp_value , rates_clamp_lower );
  double_vector_scale( solution , 1.0 / 86400 );
  return solution;
}


time_t_vector_type * ecl_sum_alloc_time_solution( const ecl_sum_type * ecl_sum , const char * gen_key , double cmp_value , bool rates_clamp_lower) {
  time_t_vector_type * solution = time_t_vector_alloc( 0 , 0);
  {
    double_vector_type * seconds = ecl_sum_alloc_seconds_solution( ecl_sum , gen_key , cmp_value , rates_clamp_lower );
    time_t start_time = ecl_sum_get_start_time(ecl_sum);
    for (int i=0; i < double_vector_size( seconds ); i++) {
      time_t t = start_time;
      util_inplace_forward_seconds_utc( &t , double_vector_iget( seconds , i ));
      time_t_vector_append( solution , t );
    }
    double_vector_free( seconds );
  }
  return solution;
}
