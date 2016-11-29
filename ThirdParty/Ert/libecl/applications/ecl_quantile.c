/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_quantile.c' is part of ERT - Ensemble based Reservoir Tool.

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

#define  _GNU_SOURCE   /* Must define this to get access to pthread_rwlock_t */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <glob.h>

#include <ert/util/util.h>
#include <ert/util/double_vector.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/statistics.h>
#include <ert/util/vector.h>
#include <ert/util/arg_pack.h>
#include <ert/util/thread_pool.h>

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>
#include <ert/config/config_error.h>
#include <ert/config/config_content_item.h>
#include <ert/config/config_content_node.h>

#include <ert/ecl/ecl_sum.h>

#define DEFAULT_NUM_INTERP  50
#define SUMMARY_JOIN       ":"
#define MIN_SIZE            10
#define LOAD_THREADS         4


typedef enum {
  S3GRAPH    = 1,
  HEADER     = 2,    /* Columns of numbers like summary.x - with a header */
  PLAIN      = 3     /* Columns of numbers like summary.x - no header */
} format_type;


typedef struct {
  ecl_sum_type             * ecl_sum;
  double_vector_type       * interp_data;
  const time_t_vector_type * interp_time;
  time_t                     start_time;
  time_t                     end_time;
} sum_case_type;


/**
   Microscopic data structure representing one column of data;
   i.e. one ECLIPSE summary key and one accompanying quantile value.
*/

typedef struct {
  char    * sum_key;
  double    quantile;
} quant_key_type ;



typedef struct  {
  vector_type     * keys;  /* Vector of quant_key_type instances. */
  char            * file;
  format_type       format;
} output_type;



typedef struct {
  vector_type         * data;
  time_t_vector_type  * interp_time;
  int                   num_interp;
  time_t                start_time;
  time_t                end_time;
  const ecl_sum_type  * refcase;     /* Pointer to an arbitrary ecl_sum instance in the ensemble - to have access to indexing functions. */
  pthread_rwlock_t      rwlock;
} ensemble_type;



#define S3GRAPH_STRING "S3GRAPH"
#define HEADER_STRING  "HEADER"
#define PLAIN_STRING   "PLAIN"



/*****************************************************************/

static quant_key_type * quant_key_alloc( const char * sum_key , double quantile ) {
  quant_key_type * qkey = util_malloc( sizeof * qkey );
  qkey->sum_key  = util_alloc_string_copy( sum_key );
  qkey->quantile = quantile;
  return qkey;
}

static void quant_key_free( quant_key_type * qkey) {
  free( qkey->sum_key );
  free( qkey );
}

static void quant_key_free__( void * qkey ) {
  quant_key_free( (quant_key_type *) qkey);
}


/*****************************************************************/

sum_case_type * sum_case_fread_alloc( const char * data_file , const time_t_vector_type * interp_time ) {
  sum_case_type * sum_case = util_malloc( sizeof * sum_case );

  sum_case->ecl_sum     = ecl_sum_fread_alloc_case( data_file , SUMMARY_JOIN );
  sum_case->interp_data = double_vector_alloc(0 , 0);
  sum_case->interp_time = interp_time;
  sum_case->start_time  = ecl_sum_get_start_time( sum_case->ecl_sum );
  sum_case->end_time    = ecl_sum_get_end_time( sum_case->ecl_sum );
  return sum_case;
}


void sum_case_free( sum_case_type * sum_case) {
  ecl_sum_free( sum_case->ecl_sum );
  double_vector_free( sum_case->interp_data );
  free( sum_case );
}


void sum_case_free__( void * sum_case) {
  sum_case_free( (sum_case_type *) sum_case);
}


/*****************************************************************/


void ensemble_add_case( ensemble_type * ensemble , const char * data_file ) {
  sum_case_type * sum_case = sum_case_fread_alloc( data_file , ensemble->interp_time );

  pthread_rwlock_wrlock( &ensemble->rwlock );
  {
    printf("Loading case: %s \n", data_file );
    vector_append_owned_ref( ensemble->data , sum_case , sum_case_free__ );
    if (ensemble->start_time > 0)
      ensemble->start_time = util_time_t_min( ensemble->start_time , sum_case->start_time);
    else
      ensemble->start_time = ecl_sum_get_start_time( sum_case->ecl_sum );

    ensemble->end_time   = util_time_t_max( ensemble->end_time   , sum_case->end_time);
  }
  pthread_rwlock_unlock( &ensemble->rwlock );

}

void * ensemble_add_case__( void * arg ) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  ensemble_type * ensemble = arg_pack_iget_ptr( arg , 0 );
  const char * data_file   = arg_pack_iget_ptr( arg , 1 );
  ensemble_add_case( ensemble , data_file );
  arg_pack_free( arg_pack );
  return NULL;
}




void ensemble_init_time_interp( ensemble_type * ensemble ) {
  int i;
  for (i = 0; i < ensemble->num_interp; i++)
    time_t_vector_append( ensemble->interp_time , ensemble->start_time + i * (ensemble->end_time - ensemble->start_time) / (ensemble->num_interp - 1));
}





void ensemble_load_from_glob( ensemble_type * ensemble , const char * pattern , thread_pool_type * tp) {
  glob_t pglob;
  int    i;
  glob( pattern , GLOB_NOSORT , NULL , &pglob );

  for (i=0; i < pglob.gl_pathc; i++) {
    arg_pack_type * arg_pack = arg_pack_alloc( );
    arg_pack_append_ptr( arg_pack , ensemble );
    arg_pack_append_owned_ptr( arg_pack , util_alloc_string_copy( pglob.gl_pathv[i] ) , free );
    thread_pool_add_job( tp , ensemble_add_case__ , arg_pack );
  }

  globfree( &pglob );
}



ensemble_type * ensemble_alloc( ) {
  ensemble_type * ensemble = util_malloc( sizeof * ensemble );

  ensemble->num_interp  = DEFAULT_NUM_INTERP;
  ensemble->start_time  = -1;
  ensemble->end_time    = -1;
  ensemble->data        = vector_alloc_new();
  ensemble->interp_time = time_t_vector_alloc( 0 , -1 );
  pthread_rwlock_init( &ensemble->rwlock , NULL );
  return ensemble;
}


void ensemble_init( ensemble_type * ensemble , config_content_type * config) {

  /*1 : Loading ensembles and settings from the config instance */
  /*1a: Loading the eclipse summary cases. */
  {
    thread_pool_type * tp = thread_pool_alloc( LOAD_THREADS , true );
    {
      int i,j;
      if (config_content_has_item( config , "CASE_LIST")) {
        const config_content_item_type * case_item = config_content_get_item( config , "CASE_LIST" );
        for (j=0; j < config_content_item_get_size( case_item ); j++) {
          const config_content_node_type * case_node = config_content_item_iget_node( case_item , j );
          for (i=0; i < config_content_node_get_size( case_node ); i++) {
            const char * case_glob = config_content_node_iget( case_node , i );
            ensemble_load_from_glob( ensemble , case_glob , tp);
          }
        }
      }

    }
    thread_pool_join( tp );
    thread_pool_free( tp );
  }

  {
    const sum_case_type * tmp = vector_iget_const( ensemble->data , 0 );
    ensemble->refcase = tmp->ecl_sum;
  }

  /*1b: Other config settings */
  if (config_content_has_item( config , "NUM_INTERP" ))
    ensemble->num_interp  = config_content_iget_as_int( config , "NUM_INTERP" , 0 , 0 );


  /*2: Remaining initialization */
  ensemble_init_time_interp( ensemble );
  if (vector_get_size( ensemble->data ) < MIN_SIZE )
    util_exit("Sorry - quantiles make no sense with with < %d realizations; should have ~> 100.\n" , MIN_SIZE);
}

const ecl_sum_type * ensemble_get_refcase( const ensemble_type * ensemble ) {
  return ensemble->refcase;
}


void ensemble_free( ensemble_type * ensemble ) {
  vector_free( ensemble->data );
  time_t_vector_free( ensemble->interp_time );
  free( ensemble );
}

/*****************************************************************/

static output_type * output_alloc( const char * file , const char * format_string) {
  output_type * output = util_malloc( sizeof * output );
  output->keys = vector_alloc_new();
  output->file = util_alloc_string_copy( file );
  {
    format_type  format;

    if ( util_string_equal(format_string , S3GRAPH_STRING))
      format = S3GRAPH;
    else if ( util_string_equal( format_string , HEADER_STRING))
      format = HEADER;
    else if ( util_string_equal( format_string , PLAIN_STRING) )
      format = PLAIN;
    else {
      format = PLAIN;  /* Compiler shut up. */
      util_abort("%s: unrecognized format string:%s \n",__func__ , format_string);
    }
    output->format = format;
  }

  return output;
}



static void output_free( output_type * output ) {
  vector_free( output->keys );
  free( output->file );
  free( output );
}

static void output_free__( void * arg) {
  output_free( (output_type *) arg);
}


/**
  The @qkey input argument can contain wildcards in the summary key;
  i.e. to get 75% quantile of all wells starting with 'B' you can use
  qkey == 'WOPR:B*:0.75' - the expansion is done based on the refcase
  provided.
*/

static void output_add_key( const ecl_sum_type * refcase , output_type * output , const char * qkey) {
  int tokens;
  double  quantile;
  char ** tmp;
  char  * sum_key;

  util_split_string( qkey , SUMMARY_JOIN , &tokens , &tmp);
  if (tokens == 1)
    util_exit("Hmmm - the key:%s is malformed - must be of the form SUMMARY_KEY:QUANTILE.\n",qkey);

  if (!util_sscanf_double( tmp[tokens - 1] , &quantile))
    util_exit("Hmmmm - failed to interpret:%s as a quantile - must be a number (0,1).\n",tmp[tokens-1]);

  if (quantile <= 0 || quantile >= 1.0)
    util_exit("Invalid quantile value:%g - must be in interval (0,1)\n", quantile);

  sum_key = util_alloc_joined_string( (const char **) tmp , tokens - 1 , SUMMARY_JOIN);
  {
    stringlist_type * matching_keys = stringlist_alloc_new();
    int i;
    ecl_sum_select_matching_general_var_list( refcase , sum_key , matching_keys );
    for (i=0; i < stringlist_get_size( matching_keys ); i++)
      vector_append_owned_ref( output->keys , quant_key_alloc( stringlist_iget( matching_keys , i ) , quantile) , quant_key_free__ );

    if (stringlist_get_size( matching_keys ) == 0)
      fprintf(stderr,"** Warning: No summary vectors matching:\'%s\' found?? \n", sum_key);
    stringlist_free( matching_keys );
  }

  util_free_stringlist( tmp, tokens );
}


/*****************************************************************/

/**
   Each output line should be of the format:

   OUTPUT  output_file    key.q    key.q    key.q    key.q    ...
*/

void output_table_init( const ecl_sum_type * refcase, hash_type * output_table , const config_content_type * config ) {
  int i,j;
  if (config_content_has_item( config , "OUTPUT")) {
    const config_content_item_type * output_item = config_content_get_item( config , "OUTPUT");
    for (i = 0; i < config_content_item_get_size( output_item ); i++) {
      const config_content_node_type * output_node = config_content_item_iget_node( output_item , i );

      const char * file              = config_content_node_iget( output_node , 0 );
      const char * format_string     = config_content_node_iget( output_node , 1 );
      output_type * output           = output_alloc( file , format_string );

      /* All the keys are just added - without any check. */
      for (j = 2; j < config_content_node_get_size( output_node ); j++)
        output_add_key( refcase , output , config_content_node_iget( output_node , j));

      hash_insert_hash_owned_ref( output_table , file , output , output_free__ );
    }
  }
}


/**
   Will print the string variable @var and the numerical variable @q
   padded to a total width of @w:

        'var:0.001     '
*/

static void print_var( FILE * stream , const char * var , double q , const char * kw_fmt) {
  char * qvar = util_alloc_sprintf( "%s:%3.1f" , var , q );
  fprintf( stream , kw_fmt , qvar );
  free( qvar );
}



/*
   ** The columns are <TAB> separated! **

   An ECLIPSE summary variable is generally characterized by three
   variable values from SMSPEC vectors; the three vectors are
   KEYWORDS, WGNAMES and NUMS.

   The main variable is the KEYWORDS variable which says what type of
   variable it is. Examples of KEYWORDS variable values are 'WWCT',
   'WOPR' and 'GWPT'. To make the variable unique we then need
   additional information from one, or both of WGNAMES and NUMS. For
   instance for a well variable or group variable we will need the
   well name from WGNAMES and for a block property we will need the
   block number (as i + j*nx + k*nx*ny) from the NUMS vector.

   When writing the S3Graph header I don't understand how to enter the
   different parts of header information. The current implementation,
   which seems to work reasonably well[*] , does the following:

     1. Write a line like this:

             DATE    TIME        KEYWORD1:xxx    KEYWORD2:xxx     KEYWORD3:xxxx

        Here KEYWORD is an eclipse variable memnonic from the KEYWORDS
        array, i.e. FOPT or WWCT. The :xxx part is the quantile we are
        looking at, i.e. 0.10 or 0.90. It seems adding the quantile
        does not confuse S3Graph.

     2. Write a line with units:

             DATE        TIME    KEYWORD1:xxx    KEYWORD2:xxx     KEYWORD3:xxxx
                         DAYS    UNIT1           UNIT2            UNIT2             <---- New line


     3. Write a line with keyword qualifiers, i.e. extra information:

             DATE    TIME        WOPR:xxx        FOPT:xxxx        BPR
                     DAYS        UNIT1           UNIT2            UNIT2
                                 OP1                              1000              <---- New line

        Now - the totally confusing part is that it is not clear what
        S3Graph expects on this third line, in the case of well/group
        variables it is a well/group name from the WGNAMES array,
        whereas for e.g. a region or block varaiable it wants an
        element from the NUMS array, and for e.g. a field variable it
        wants nothing extra. When it comes to variables which need
        both NUMS and WGNAMES to become unique (e.g completion
        variables) it is not clear how - if at all possible - to
        support it. In the current implementation a string
        concatenation of WGNAMES and NUMS is used.


   [*] : I do not really understand why it seems to work.

*/




void output_save_S3Graph( const output_type * output, ensemble_type * ensemble , const double ** data ) {
  FILE * stream = util_mkdir_fopen( output->file , "w");
  const char * kw_fmt       = "\t%s";
  const char * unit_fmt     = "\t%s";
  const char * wgname_fmt   = "\t%s";
  const char * num_fmt      = "\t%d";
  const char * float_fmt    = "\t%0.4f";
  const char * days_fmt     = "\t%0.2f";

  const char * empty_fmt    = "\t";
  const char * date_fmt     = "%d/%d/%d";
  const char * time_header  = "DATE\tTIME";
  const char * time_unit    = "\tDAYS";
  const char * time_blank   = "\t";
  const int    data_columns = vector_get_size( output->keys );
  const int    data_rows    = time_t_vector_size( ensemble->interp_time );
  int row_nr,column_nr;

  {
    char       * origin;
    util_alloc_file_components( output->file , NULL ,&origin , NULL);
    fprintf(stream , "ORIGIN %s\n", origin );
    free( origin );
  }

  /* 1: Writing first header line with variables. */
  fprintf(stream , time_header );
  for (column_nr = 0; column_nr < data_columns; column_nr++) {
    const quant_key_type * qkey = vector_iget( output->keys , column_nr );
    print_var( stream , ecl_sum_get_keyword( ensemble->refcase , qkey->sum_key ) , qkey->quantile , kw_fmt);
  }
  fprintf(stream , "\n");

  /* 2: Writing second header line with units. */
  fprintf(stream , time_unit );
  for (column_nr = 0; column_nr < data_columns; column_nr++) {
    const quant_key_type * qkey = vector_iget( output->keys , column_nr );
    fprintf(stream , unit_fmt , ecl_sum_get_unit( ensemble->refcase , qkey->sum_key ) );
  }
  fprintf(stream , "\n");

  /*3: Writing third header line with WGNAMES / NUMS - extra information -
       breaks completely down with LGR information. */
  fprintf(stream , time_blank );
  {
    for (column_nr = 0; column_nr < data_columns; column_nr++) {
      const quant_key_type * qkey  = vector_iget( output->keys , column_nr );
      const char * ecl_key         = qkey->sum_key;
      const char * wgname          = ecl_sum_get_wgname( ensemble->refcase , ecl_key );
      int          num             = ecl_sum_get_num( ensemble->refcase , ecl_key );
      ecl_smspec_var_type var_type = ecl_sum_get_var_type( ensemble->refcase , ecl_key);
      bool need_num                = ecl_smspec_needs_num( var_type );
      bool need_wgname             = ecl_smspec_needs_wgname( var_type );

      if (need_num && need_wgname) {
        /** Do not know how to include both - will just create a
            mangled name as a combination. */
        char * wgname_num = util_alloc_sprintf("%s:%d" , wgname , num);
        fprintf(stream , wgname_fmt , wgname_num);
        free( wgname_num );
      } else if (need_num)
        fprintf(stream , num_fmt , num);
      else if (need_wgname)
        fprintf(stream , wgname_fmt , wgname);
      else
        fprintf(stream , empty_fmt );
    }
    fprintf(stream , "\n");
  }

  /*4: Writing the actual data. */
  for (row_nr = 0; row_nr < data_rows; row_nr++) {
    time_t interp_time = time_t_vector_iget( ensemble->interp_time , row_nr);
    {
      int mday,month,year;
      ecl_util_set_date_values(interp_time , &mday , &month , &year);
      fprintf(stream , date_fmt , mday , month , year);
    }
    fprintf(stream , days_fmt , 1.0*(interp_time - ensemble->start_time) / 86400);

    for (column_nr = 0; column_nr < data_columns; column_nr++) {
      fprintf(stream , float_fmt , data[row_nr][column_nr]);
    }
    fprintf( stream , "\n");
  }
}



void output_save_plain__( const output_type * output , ensemble_type * ensemble , const double ** data , bool add_header) {
  FILE * stream = util_mkdir_fopen( output->file , "w");
  const char * key_fmt      = " %18s:%4.2f ";
  const char * time_header  = "--    DAYS      DATE    ";
  const char * time_dash    = "------------------------";
  const char * key_dash     = "-------------------------";
  const char * float_fmt    = "%24.5f ";
  const char * days_fmt     = "%10.2f ";
  const char * date_fmt     = "  %02d/%02d/%04d ";
  const int    data_columns = vector_get_size( output->keys );
  const int    data_rows    = time_t_vector_size( ensemble->interp_time );
  int row_nr,column_nr;

  if (add_header) {
    fprintf( stream ,time_header);
    for (int i=0; i < vector_get_size( output->keys ); i++) {
      const quant_key_type * qkey = vector_iget( output->keys , i );
      fprintf( stream , key_fmt , qkey->sum_key , qkey->quantile );
    }
    fprintf(stream , "\n");

    fprintf( stream , time_dash );
    for (int i=0; i < vector_get_size( output->keys ); i++)
      fprintf(stream , key_dash );
    fprintf(stream , "\n");
  }

  /*4: Writing the actual data. */
  for (row_nr = 0; row_nr < data_rows; row_nr++) {
    time_t interp_time = time_t_vector_iget( ensemble->interp_time , row_nr);
    fprintf(stream , days_fmt , 1.0*(interp_time - ensemble->start_time) / 86400);
    {
      int mday,month,year;
      ecl_util_set_date_values(interp_time , &mday , &month , &year);
      fprintf(stream , date_fmt , mday , month , year);
    }

    for (column_nr = 0; column_nr < data_columns; column_nr++) {
      fprintf(stream , float_fmt , data[row_nr][column_nr]);
    }
    fprintf( stream , "\n");
  }
}




void output_save( const output_type * output , ensemble_type * ensemble , const double ** data ) {
  switch( output->format ) {
  case(S3GRAPH):
    output_save_S3Graph( output , ensemble , data );
    break;
  case(PLAIN):
    output_save_plain__( output , ensemble , data , false);
    break;
  case(HEADER):
    output_save_plain__( output , ensemble , data , true);
    break;
  default:
    util_exit("Sorry: output_format:%d not supported \n", output->format );
  }
}





void output_run_line( const output_type * output , ensemble_type * ensemble) {

  const int    data_columns = vector_get_size( output->keys );
  const int    data_rows    = time_t_vector_size( ensemble->interp_time );
  double     ** data;
  int row_nr, column_nr;

  data = util_calloc( data_rows , sizeof * data );
  /*
    time-direction, i.e. the row index is the first index and the
    column number (i.e. the different keys) is the second index.
  */
  for (row_nr=0; row_nr < data_rows; row_nr++)
    data[row_nr] = util_calloc( data_columns , sizeof * data[row_nr] );

  printf("Creating output file: %s \n",output->file );


  /*
     Go through all the cases and check that they have this key;
     exit if missing. Could also ignore the missing keys and just
     continue; and even defer the checking to the inner loop.
  */
  for (column_nr = 0; column_nr < vector_get_size( output->keys ); column_nr++) {
    const quant_key_type * qkey = vector_iget( output->keys , column_nr );
    {
      bool OK = true;

      for (int iens = 0; iens < vector_get_size( ensemble->data ); iens++) {
        const sum_case_type * sum_case = vector_iget_const( ensemble->data , iens );

        if (!ecl_sum_has_general_var(sum_case->ecl_sum , qkey->sum_key)) {
          OK = false;
          fprintf(stderr,"** Sorry: the case:%s does not have the summary key:%s \n", ecl_sum_get_case( sum_case->ecl_sum ), qkey->sum_key);
        }
      }

      if (!OK)
        util_exit("Exiting due to missing summary vector(s).\n");
    }
  }


  /* The main loop - outer loop is running over time. */
  {
    /**
       In the quite typical case that we are asking for several
       quantiles of the quantity, i.e.

       WWCT:OP_1:0.10  WWCT:OP_1:0.50  WWCT:OP_1:0.90

       the interp_data_cache construction will ensure that the
       underlying ecl_sum object is only queried once; and also the
       sorting will be performed once.
    */

    hash_type * interp_data_cache = hash_alloc();

    for (row_nr = 0; row_nr < data_rows; row_nr++) {
      time_t interp_time = time_t_vector_iget( ensemble->interp_time , row_nr);
      for (column_nr = 0; column_nr < vector_get_size( output->keys ); column_nr++) {
        const quant_key_type * qkey = vector_iget( output->keys , column_nr );
        double_vector_type * interp_data;

        /* Check if we have the vector in the cache table - if not create it. */
        if (!hash_has_key( interp_data_cache , qkey->sum_key)) {
          interp_data = double_vector_alloc(0 , 0);
          hash_insert_hash_owned_ref( interp_data_cache , qkey->sum_key , interp_data , double_vector_free__);
        }
        interp_data = hash_get( interp_data_cache , qkey->sum_key );

        /* Check if the vector has data - if not initialize it. */
        if (double_vector_size( interp_data ) == 0) {
          for (int iens = 0; iens < vector_get_size( ensemble->data ); iens++) {
            const sum_case_type * sum_case = vector_iget_const( ensemble->data , iens );

            if ((interp_time >= sum_case->start_time) && (interp_time <= sum_case->end_time))  /* We allow the different simulations to have differing length */
              double_vector_append( interp_data , ecl_sum_get_general_var_from_sim_time( sum_case->ecl_sum , interp_time , qkey->sum_key)) ;

            double_vector_sort( interp_data );
          }
        }
        data[row_nr][column_nr] = statistics_empirical_quantile__( interp_data , qkey->quantile );
      }
      hash_apply( interp_data_cache , double_vector_reset__ );
    }
    hash_free( interp_data_cache );
  }

  output_save( output , ensemble , (const double **) data);
  for (row_nr=0; row_nr < data_rows; row_nr++)
    free( data[row_nr] );
  free( data );
}



void output_table_run( hash_type * output_table , ensemble_type * ensemble ) {
  hash_iter_type * iter = hash_iter_alloc( output_table);

  while (!hash_iter_is_complete( iter )) {
    const char * output_file     = hash_iter_get_next_key( iter );
    const output_type * output   = hash_get( output_table , output_file );
    output_run_line( output, ensemble );
  }
}




/*****************************************************************/

void config_init( config_parser_type * config ) {


  config_add_schema_item( config , "CASE_LIST"      , true );
  config_add_key_value( config , "NUM_INTERP" , false , CONFIG_INT);

  {
    config_schema_item_type * item;
    item = config_add_schema_item( config , "OUTPUT" , true );
    config_schema_item_set_argc_minmax( item , 2 , CONFIG_DEFAULT_ARG_MAX );
    config_schema_item_set_indexed_selection_set( item , 1 , 3 , (const char *[3]) { S3GRAPH_STRING , HEADER_STRING , PLAIN_STRING });
  }

}


/*****************************************************************/

void usage() {
  fprintf(stderr, "\nUse:\n\n    ecl_quantile config_file\n\n");

  printf("Help\n");
  printf("----\n");
  printf("\n");
  printf("The ecl_quantile program will load an ensemble of ECLIPSE summary\n");
  printf("files, it can then output quantiles of summary vectors over the time\n");
  printf("span of the simulation. The program is based on a simple configuration\n");
  printf("file which must be given as a commandline argument. The configuration\n");
  printf("file only has three keywords:\n");
  printf("\n");
  printf("\n");
  printf("   CASE_LIST   simulation*X/run*X/CASE*.DATA\n");
  printf("   CASE_LIST   extra_simulation.DATA    even/more/simulations*GG/run*.DATA\n");
  printf("   OUTPUT      FILE1   S3GRAPH WWCT:OP_1:0.10  WWCT:OP_1:0.50   WOPR:OP_3\n");
  printf("   OUTPUT      FILE2   PLAIN   FOPT:0.10  FOPT:0.90  FGPT:0.10  FGPT:0.90   FWPT:0.10  FWPT:0.90\n");
  printf("   NUM_INTERP  100\n");
  printf("\n");
  printf("\n");
  printf("CASE_LIST: This keyword is used to give the path to ECLIPSE data files\n");
  printf("  corresponding to summaries which you want to load, observe that the\n");
  printf("  argument given to the CASE_LIST keyword can contain unix-style\n");
  printf("  wildcards like '*'. One CASE_LIST keyword can point to several cases, \n");
  printf("  and in addition you can have several CASE_LIST keywords.\n");
  printf("\n");
  printf("\n");
  printf("OUTPUT: This keyword is used to denote what output you want from the\n");
  printf("  program. The first argument to the OUTPUT keyword is the name output\n");
  printf("  file you want to produce, in the example above we will create two\n");
  printf("  output files (FILE1 and FILE2 respectively). The second argument is \n");
  printf("  the wanted type of the output file, the three types currently supported\n");
  printf("  are: \n\n");
  printf("     S3GRAPH: S3GRAPH user format - at least quite close...\n");
  printf("     PLAIN: Columns of data without any header information\n");
  printf("     HEADER: Like plain, but with a header at the top\n\n");
  printf("  The remaining arguments on the output line corresponds to the \n");
  printf("  summary vector & quantile you are interested in. Each of these values\n");
  printf("  is a \":\" separated string consting of:\n");
  printf("  \n");
  printf("     VAR: The ECLIPSE summary variable we are interested in, (nearly)\n");
  printf("          all variables found in the summary file are available,\n");
  printf("          e.g. RPR, WWCT or GOPT. \n");
  printf("\n");
  printf("     WG?: This is extra information added to the variable to make it\n");
  printf("          unique, e.g. the name of a well or group for rate variables\n");
  printf("          and the region number for a region. Not all variables, in\n");
  printf("          particalar the field rates, Fxxx, have this string.\n");
  printf("\n");
  printf("     Q:   The quantile we are interested in, e.g 0.10 to get the P10\n");
  printf("          quantile and 0.90 to get the P90 quantile.\n");
  printf("\n");
  printf("  For the 'VAR' and 'WG?' parts of the keys you can use shell-style\n");
  printf("  wildcards to get all summary vectors matching a criteria, i.e. \n");
  printf("  'WOPR:A-*:0.50' will give the P50 quantile of WOPR for all wells \n");
  printf("  starting with 'A-'\n");
  printf("\n");
  printf("  Examples are:\n");
  printf("\n");
  printf("     WWCT:OPX:0.75     The P75 quantile of the watercut in well OPX.\n");
  printf("     BPR:10,10,5:0.50  The P50 quantile of the Block Pressure in block 10,10,5\n");
  printf("     FOPT:0.90         The P90 quantile of the field oil production total.\n");
  printf("     RPR:*:0.50        The P50 quantile of all regions.\n");
  printf("\n");
  printf("\n");
  printf("NUM_INTERP: Before the program can calculate quantiles it must\n");
  printf("  interpolate all the simulated data down on the same time axis. This\n");
  printf("  keyword regulates how many points should be used when interpolating\n");
  printf("  the time axis; the default is 50 which is probably quite OK. Observe\n");
  printf("  that for rate variable the program will not do linear interpolation\n");
  printf("  between ECLIPSE report steps, the might therefore look a bit jagged\n");
  printf("  if NUM_INTERP is set too high. This keyword is optional.\n");
  printf("\n");
  printf("All filenames in the configuration file will be interpreted relative to\n");
  printf("the location of the configuration file, i.e. irrespective of the current\n");
  printf("working directory when invoking the ecl_quantile program.\n\n");
  printf("ecl_quantile is written by Joakim Hove / joaho@statoil.com / 92 68 57 04.\n");
  exit(0);
}



int main( int argc , char ** argv ) {
  if (argc != 2)
    usage();
  else {
    hash_type     * output_table   = hash_alloc();
    ensemble_type * ensemble       = ensemble_alloc();
    {
      config_parser_type   * config      = config_alloc( );
      config_content_type   * content;
      const char    * config_arg  = argv[1];

      config_init( config );
      content = config_parse( config , config_arg , "--" , NULL , NULL , NULL , CONFIG_UNRECOGNIZED_WARN, true );

      if (config_content_is_valid( content )) {
        char * config_path;
        util_alloc_file_components( config_arg , &config_path , NULL , NULL);
        if (config_path != NULL) {
          util_chdir( config_path );
          free( config_path );
        }
      } else {
        config_error_type * error = config_content_get_errors( content );
        config_error_fprintf( error , true , stderr );
        exit(1);
      }



      ensemble_init( ensemble , content );
      output_table_init( ensemble_get_refcase( ensemble ) , output_table , content );
      config_content_free( content );
      config_free( config );
    }
    output_table_run( output_table , ensemble );
    ensemble_free( ensemble );
    hash_free( output_table );
  }
}
