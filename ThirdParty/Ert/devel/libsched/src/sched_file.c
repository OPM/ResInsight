/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_file.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/parser.h>
#include <ert/util/time_t_vector.h>

#include <ert/sched/sched_file.h>
#include <ert/sched/sched_util.h>
#include <ert/sched/sched_blob.h>
#include <ert/sched/sched_kw_dates.h>
#include <ert/sched/sched_kw_wconhist.h>
#include <ert/sched/sched_kw_wconinje.h>
#include <ert/sched/sched_kw_tstep.h>
#include <ert/sched/sched_kw.h>

/* This sched_file.c contains code for internalizing an ECLIPSE
   schedule file.

   Two structs are defined in this file:

    1. The sched_file_struct, which can be accessed externaly
       through various interface functions.

    2. The sched_block_struct, which is for internal use.

   The internalization function 'sched_file_parse' splits the ECLIPSE
   schedule file into a sequence of 'sched_block_type's, where a single 
   block contains one or more keywords. Except for the first block, which
   is empty per definition, the last keyword in a block will always be
   a timing keyword like DATES or TSTEP. Thus, the number of blocks in
   the sched_file_struct will always cooincide with the number of 
   restart files in the ECLIPSE simulation. In order to make this work,
   TSTEP and DATES keyword containing multiple data, are split into
   a sequence of keywords. 

   Note the following:

   1. This implies that scheduling data after the last timing
      keyword is irrelevant. This is similar to how ECLIPSE works.

   2. Scheduling data after keyword END is ignored, since this
      is interpreted as the end of the SCHEDULE section.

*/

#define SCHED_FILE_TYPE_ID 677198

struct sched_block_struct {
  vector_type     * kw_list;           /* A list of sched_kw's in the block.   */
  time_t            block_start_time;  
  time_t            block_end_time;
  hash_type       * kw_hash;           /* Hash table indexed with kw_name - containing vectors of kw instances . */
};



struct sched_file_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type         * fixed_length_table;    /* A hash table of keywords with a fixed length, i.e. not '/' terminated. */
  vector_type       * kw_list;
  vector_type       * kw_list_by_type;        
  vector_type       * blocks;                /* A list of chronologically sorted sched_block_type's. */
  stringlist_type   * files;                 /* The name of the files which have been parsed to generate this sched_file instance. */
  time_t              start_time;            /* The start of the simulation. */
  bool                hasEND;
};



/************************************************************************/



static sched_block_type * sched_block_alloc_empty()
{
  sched_block_type * block = util_malloc(sizeof * block);
  block->kw_list = vector_alloc_new();
  block->kw_hash = hash_alloc();
  return block;
}



static void sched_block_debug_fprintf( const sched_block_type * block ) {
  util_fprintf_date( block->block_start_time , stdout );
  printf(" -- ");
  util_fprintf_date( block->block_end_time , stdout );
  printf("\n");
  {
    int i;
    for (i=0; i < vector_get_size( block->kw_list ); i++) {
      const sched_kw_type * sched_kw = vector_iget_const( block->kw_list , i);
      printf("%s \n",sched_kw_get_type_name( sched_kw ));
    }
  }
}


static void sched_block_free(sched_block_type * block)
{
  vector_free( block->kw_list );
  hash_free( block->kw_hash );
  free(block);
}



static void sched_block_free__(void * block)
{    
  sched_block_free( (sched_block_type *) block);
}



static void sched_block_add_kw(sched_block_type * block, const sched_kw_type * kw)
{
  vector_append_ref(block->kw_list , kw );
  if (!hash_has_key( block->kw_hash , sched_kw_get_name( kw ))) 
    hash_insert_hash_owned_ref( block->kw_hash , sched_kw_get_name( kw ) , vector_alloc_new() , vector_free__);
  
  {
    vector_type * kw_vector = hash_get( block->kw_hash , sched_kw_get_name( kw ));
    vector_append_ref( kw_vector , kw );
  }
}


sched_kw_type * sched_block_iget_kw(sched_block_type * block, int i)
{
  return vector_iget( block->kw_list , i);
}





static void sched_block_fprintf(const sched_block_type * block, FILE * stream)
{
  int i;
  for (i=0; i < vector_get_size(block->kw_list); i++) {
    const sched_kw_type * sched_kw = vector_iget_const( block->kw_list , i);
    sched_kw_fprintf(sched_kw, stream);
  }
}



int sched_block_get_size(const sched_block_type * block)
{
  return vector_get_size(block->kw_list);
}





static sched_kw_type * sched_block_get_last_kw_ref(sched_block_type * block)
{
  int last_index = vector_get_size( block->kw_list ) - 1;
  return sched_block_iget_kw( block , last_index );
}



static void sched_file_add_block(sched_file_type * sched_file, sched_block_type * block)
{
  vector_append_owned_ref(sched_file->blocks , block , sched_block_free__);
}



sched_block_type * sched_file_iget_block(const sched_file_type * sched_file, int i)
{
  return vector_iget(sched_file->blocks , i);
}

/**
   This is a fucking mess:

   block[0]   start_time - start_time
   block[1]   start_time - time of first report, i.e. X0001
   block[2]   X0001      - X0002
   ....

   The reason for this funny convention is to ensure one-to-one index
   correspondance between the restart files and the sched_file blocks
   (I think ...).
*/



static void sched_file_build_block_dates(sched_file_type * sched_file)
{
  int num_restart_files = sched_file_get_num_restart_files(sched_file);
  time_t curr_time, new_time;

  if(num_restart_files < 1)
    util_abort("%s: Error - empty sched_file - aborting.\n", __func__);

  /* Special case for block 0. */
  sched_block_type * sched_block = sched_file_iget_block(sched_file, 0);
  sched_block->block_start_time  = sched_file->start_time ;
  sched_block->block_end_time    = sched_file->start_time ;

  curr_time = sched_file->start_time;
  for(int i=1; i<num_restart_files; i++)
  {
    sched_block = sched_file_iget_block(sched_file, i);
    sched_block->block_start_time = curr_time;
    
    sched_kw_type * timing_kw = sched_block_get_last_kw_ref(sched_block);
    new_time = sched_kw_get_new_time(timing_kw, curr_time);
    
    if(curr_time > new_time)
      util_abort("%s: Schedule file contains negative timesteps - aborting.\n",__func__);
    
    curr_time = new_time;
    sched_block->block_end_time = curr_time;
  }
}




/******************************************************************************/



static void sched_file_add_kw( sched_file_type * sched_file , const sched_kw_type * kw) {
  vector_append_owned_ref( sched_file->kw_list , kw , sched_kw_free__);
}


static void sched_file_update_index( sched_file_type * sched_file ) {
  int ikw;
  

  /* By type index */
  {
    if (sched_file->kw_list_by_type != NULL) 
      vector_free( sched_file->kw_list_by_type );
    sched_file->kw_list_by_type = vector_alloc_NULL_initialized( NUM_SCHED_KW_TYPES );
    for (ikw = 0; ikw < vector_get_size( sched_file->kw_list ); ikw++) {
      const sched_kw_type * kw = vector_iget_const( sched_file->kw_list , ikw );
      sched_kw_type_enum type  = sched_kw_get_type( kw );
      {
        vector_type * tmp      = vector_iget( sched_file->kw_list_by_type , type );
        
        if (tmp == NULL) {
          tmp = vector_alloc_new();
          vector_iset_owned_ref( sched_file->kw_list_by_type , type , tmp , vector_free__ );
        }
        
        vector_append_ref( tmp , kw );
      }
    }
  }

  
  
  /* Block based on restart number. */
  {
    time_t current_time;
    sched_block_type * current_block;
    vector_clear( sched_file->blocks );

    /* 
       Adding a pseudo block at the start which runs from the start of
       time (i.e. EPOCH start 01/01/1970) to simulation start.
    */
    current_block = sched_block_alloc_empty( 0 );
    current_block->block_start_time  = sched_file->start_time;//-1;     /* Need this funny node - hhmmmmmm */
    current_block->block_end_time    = sched_file->start_time;
    sched_file_add_block( sched_file , current_block );
    
    current_block = sched_block_alloc_empty( 0 );
    current_block->block_start_time  = sched_file->start_time;
    current_time = sched_file->start_time;
    
    for (ikw = 0; ikw < vector_get_size( sched_file->kw_list ); ikw++) {
      const sched_kw_type * kw = vector_iget_const( sched_file->kw_list , ikw );
      sched_kw_type_enum type  = sched_kw_get_type( kw );
      {
        sched_block_add_kw( current_block , kw );
        if(type == DATES || type == TSTEP || type == TIME) {
          /**
             Observe that when we enocunter a time-based keyword we do the following:
             
               1. Finish the the current block by setting the end_time
                  field and add this block to the sched_file
                  structure.

               2. Create a new block starting at current time.

              ------- 

              Blocks are not actually added to the sched_file instance
              before they are terminated with a DATES/TSTEP
              keyword. This implies that keywords which come after the
              last DATES/TSTEP keyword are lost.
          */
               
          current_time = sched_kw_get_new_time( kw , current_time );

          /* Finishing off the current block, and adding it to the sched_file. */
          current_block->block_end_time = current_time;
          sched_file_add_block( sched_file , current_block );
          
          /* Creating a new block - not yet added to the sched_file. */
          current_block = sched_block_alloc_empty( vector_get_size( sched_file->blocks ));
          current_block->block_start_time = current_time;
        }
      }
    }
    /*
      Free the last block, which has not been added to the sched_file
      object.
    */
    sched_block_free( current_block );
  }
}



void sched_file_add_fixed_length_kw( sched_file_type * sched_file , const char * kw , int length ) {
  hash_insert_int( sched_file->fixed_length_table, kw , length );
}



static void sched_file_init_fixed_length( sched_file_type * sched_file ) {
  sched_file_add_fixed_length_kw(sched_file , "NEXTSTEP" , 1);
  sched_file_add_fixed_length_kw(sched_file , "RPTSCHED" , 1);
  sched_file_add_fixed_length_kw(sched_file , "DRSDT"    , 1);
  sched_file_add_fixed_length_kw(sched_file , "SKIPREST" , 0);
  sched_file_add_fixed_length_kw(sched_file , "NOECHO"   , 0);
  sched_file_add_fixed_length_kw(sched_file , "ECHO"     , 0);
  sched_file_add_fixed_length_kw(sched_file , "RPTRST"   , 1);
  sched_file_add_fixed_length_kw(sched_file , "TUNING"   , 3);
  sched_file_add_fixed_length_kw(sched_file , "WHISTCTL" , 1);
  sched_file_add_fixed_length_kw(sched_file , "TIME"     , 1);
  sched_file_add_fixed_length_kw(sched_file , "VAPPARS"  , 1);
  sched_file_add_fixed_length_kw(sched_file , "NETBALAN" , 1);
  sched_file_add_fixed_length_kw(sched_file , "WPAVE"    , 1);
  sched_file_add_fixed_length_kw(sched_file , "VFPTABL"  , 1);
  sched_file_add_fixed_length_kw(sched_file , "GUIDERAT" , 1);
  sched_file_add_fixed_length_kw(sched_file , "MESSAGES" , 1);
  sched_file_add_fixed_length_kw(sched_file , "LIFTOPT"  , 1);
}


sched_file_type * sched_file_alloc(time_t start_time)
{
  sched_file_type * sched_file = util_malloc(sizeof * sched_file);
  UTIL_TYPE_ID_INIT( sched_file , SCHED_FILE_TYPE_ID);
  sched_file->kw_list            = vector_alloc_new();
  sched_file->kw_list_by_type    = NULL;
  sched_file->blocks             = vector_alloc_new();
  sched_file->files              = stringlist_alloc_new();
  sched_file->start_time         = start_time;
  sched_file->fixed_length_table = hash_alloc();
  sched_file->hasEND             = false;
  sched_file_init_fixed_length( sched_file );
  {
    char * fixed_length_file = getenv("SCHEDULE_FIXED_LENGTH");
    if ((fixed_length_file != NULL) && (util_entry_readable( fixed_length_file ))) {
      FILE * stream = util_fopen(fixed_length_file , "r");
      char kw[32];
      int  len;
      bool OK = true;

      do {
        if (fscanf(stream , "%s %d" , kw , &len) == 2)
          sched_file_add_fixed_length_kw( sched_file , kw , len);
        else
          OK = false;
      } while (OK);
      fclose( stream);
    }
  }
  return sched_file;
}


UTIL_SAFE_CAST_FUNCTION(sched_file , SCHED_FILE_TYPE_ID);


void sched_file_free(sched_file_type * sched_file)
{
  vector_free( sched_file->blocks );
  vector_free( sched_file->kw_list );
  if (sched_file->kw_list_by_type != NULL)
    vector_free( sched_file->kw_list_by_type );

  stringlist_free( sched_file->files );
  hash_free( sched_file->fixed_length_table );
  free(sched_file);
}


/**
   This function will allocate a time_t_vector instance, which
   contains all the time_t values for this schedule_file - starting
   with the start_date.
*/

time_t_vector_type * sched_file_alloc_time_t_vector( const sched_file_type * sched_file ) {
  time_t_vector_type * vector = time_t_vector_alloc(0,0);
  int i;
  time_t_vector_append( vector , sched_file->start_time );
  for (i=1; i < vector_get_size( sched_file->blocks ); i++) {
    const sched_block_type * block = vector_iget_const( sched_file->blocks , i );
    time_t_vector_append( vector , block->block_end_time );
  }
  return vector;
}


static stringlist_type * sched_file_tokenize( const char * filename ) {
  stringlist_type  * token_list;
  parser_type     * parser    = parser_alloc(" \t"  ,      /* Splitters */
                                             "\'\"" ,      /* Quoters   */
                                             "\n"   ,      /* Specials - splitters which will be kept. */  
                                             "\r"   ,      /* Delete set - these are just deleted. */
                                             "--"   ,      /* Comment start */
                                             "\n");        /* Comment end */  
  bool strip_quote_marks = false;
  token_list             = parser_tokenize_file( parser , filename , strip_quote_marks  );
  parser_free( parser );
  
  return token_list;
}


/**
   This function parses 'further', i.e typically adding another
   schedule file to the sched_file instance.
*/

void sched_file_parse_append(sched_file_type * sched_file , const char * filename) {
  bool foundEND = false;
  stringlist_type * token_list = sched_file_tokenize( filename );
  sched_kw_type    * current_kw;
  int token_index = 0;
  do {
    sched_util_skip_newline( token_list , &token_index );
    current_kw = sched_kw_token_alloc(token_list , &token_index , sched_file->fixed_length_table, &foundEND);
    if (current_kw != NULL) {
      sched_kw_type_enum type = sched_kw_get_type(current_kw);
      if (type == DATES || type == TSTEP || type == TIME) {
        int i , num_steps;
        sched_kw_type ** sched_kw_dates = sched_kw_split_alloc_DATES(current_kw, &num_steps);
        sched_kw_free(current_kw);
        
        for(i=0; i<num_steps; i++)  
          sched_file_add_kw( sched_file , sched_kw_dates[i]);
        
        free(sched_kw_dates);   
      } else
        sched_file_add_kw( sched_file , current_kw);
    }
  } while ( current_kw != NULL );
  
  if (foundEND)
    sched_file->hasEND = true;
  
  stringlist_append_copy( sched_file->files , filename );
  sched_file_build_block_dates(sched_file);
  sched_file_update_index( sched_file );
  stringlist_free( token_list );
}


void sched_file_simple_parse( const char * filename , time_t start_time) {
  stringlist_type * token_list = sched_file_tokenize( filename );
  const int num_tokens         = stringlist_get_size( token_list );
  int token_index = 0;
  do {
    sched_kw_type_enum kw_type = sched_kw_type_from_string( stringlist_iget( token_list , token_index ));
    if ((kw_type == DATES) || (kw_type == TSTEP)) {
      
    }
  } while( token_index < num_tokens );
  
  stringlist_free( token_list );
}



void sched_file_parse(sched_file_type * sched_file, const char * filename)
{
  /* 
     Add the first empty pseudo block - this runs from time -infty:start_date.
  */
  sched_file_add_block(sched_file , sched_block_alloc_empty());
  sched_file_parse_append( sched_file , filename );
}



sched_file_type * sched_file_parse_alloc(const char * filename , time_t start_date) {
  sched_file_type * sched_file = sched_file_alloc( start_date );
  sched_file_parse(sched_file , filename);
  return sched_file;
}



int sched_file_get_num_restart_files(const sched_file_type * sched_file)
{
  return vector_get_size(sched_file->blocks);
}



static void sched_file_fprintf_i__(const sched_file_type * sched_file, int last_restart_file, const char * file , bool addEND)
{
  FILE * stream = util_fopen(file, "w");
  int num_restart_files = sched_file_get_num_restart_files(sched_file);
  

  last_restart_file = util_int_min( last_restart_file , num_restart_files - 1);
  
  if (last_restart_file > num_restart_files) {
    util_abort("%s: you asked for restart nr:%d - the last available restart nr is: %d \n",__func__ , last_restart_file , num_restart_files);
    /* Must abort here because the calling scope is expecting to find last_restart_file.  */
  }
  
  for(int i=0; i<= last_restart_file; i++)
  {
    const sched_block_type * sched_block = vector_iget_const( sched_file->blocks , i);
    sched_block_fprintf(sched_block, stream);
  }

  if (addEND)
    fprintf(stream, "END\n");
  
  fclose(stream);
}


void sched_file_fprintf_i(const sched_file_type * sched_file, int last_restart_file, const char * file) {
  sched_file_fprintf_i__( sched_file , last_restart_file , file , true);
}


/* Writes the complete schedule file. */
void sched_file_fprintf(const sched_file_type * sched_file, const char * file)
{
  int num_restart_files = sched_file_get_num_restart_files(sched_file);
  sched_file_fprintf_i__( sched_file , num_restart_files - 1 , file , sched_file->hasEND);
}





/*
  const char * sched_file_get_filename(const sched_file_type * sched_file) {
  return sched_file->filename;
  }
*/


int sched_file_get_restart_nr_from_time_t(const sched_file_type * sched_file, time_t time)
{
  int num_restart_files = sched_file_get_num_restart_files(sched_file);
  for( int i=0; i<num_restart_files; i++ ) {
    time_t block_end_time = sched_file_iget_block_end_time(sched_file, i);

    if (block_end_time > time) {
      int mday,year,month;
      util_set_date_values( time , &mday , &month , &year);
      util_abort("%s: Date: %02d/%02d/%04d  does not cooincide with any report time. Aborting.\n", __func__ , mday , month , year);
    } else if (block_end_time == time)
      return i; 
  }
  
  // If we are here, time did'nt correspond a restart file. Abort.
  {
    int mday,year,month;
    util_set_date_values( time , &mday , &month , &year);
    util_abort("%s: Date: %02d/%02d/%04d  does not cooincide with any report time. Aborting.\n", __func__ , mday , month , year);
  }
  return 0;
}


/**
   This function finds the restart_nr for the a number of days after
   simulation start.
*/

int sched_file_get_restart_nr_from_days(const sched_file_type * sched_file , double days) {
  time_t time = sched_file_iget_block_start_time(sched_file, 0);
  util_inplace_forward_days( &time , days);
  return sched_file_get_restart_nr_from_time_t(sched_file , time);
}



time_t sched_file_iget_block_start_time(const sched_file_type * sched_file, int i)
{
  sched_block_type * block = sched_file_iget_block(sched_file, i);
  return block->block_start_time;
}



time_t sched_file_iget_block_end_time(const sched_file_type * sched_file, int i)
{
  sched_block_type * block = sched_file_iget_block(sched_file, i);
  return block->block_end_time;
}


double sched_file_iget_block_start_days(const sched_file_type * sched_file, int i)
{
  sched_block_type * block = sched_file_iget_block(sched_file, i);
  return util_difftime_days( sched_file->start_time , block->block_start_time );
}


double sched_file_iget_block_end_days(const sched_file_type * sched_file, int i)
{
  sched_block_type * block = sched_file_iget_block(sched_file, i);
  return util_difftime_days( sched_file->start_time , block->block_end_time );
}


double sched_file_get_sim_days(const sched_file_type * sched_file , int report_step) {
  return sched_file_iget_block_end_days( sched_file , report_step );
}


time_t sched_file_get_sim_time(const sched_file_type * sched_file , int report_step) {
  return sched_file_iget_block_end_time( sched_file , report_step );
}


const char * sched_file_iget_filename( const sched_file_type * sched_file , int file_nr ) {
  return stringlist_iget( sched_file->files , file_nr );
}




int sched_file_iget_block_size(const sched_file_type * sched_file, int block_nr)
{
  sched_block_type * block = sched_file_iget_block(sched_file, block_nr);
  return sched_block_get_size(block);
}



sched_kw_type * sched_file_ijget_block_kw_ref(const sched_file_type * sched_file, int block_nr, int kw_nr)
{
  sched_block_type * block = sched_file_iget_block(sched_file, block_nr);
  sched_kw_type * sched_kw = sched_block_iget_kw(block, kw_nr);
  return sched_kw;
}



static void __sched_file_summarize_line(int restart_nr , time_t start_time , time_t t , FILE * stream) {
  double days    = util_difftime( start_time , t , NULL , NULL , NULL , NULL) / (24 * 3600);
  int mday , month , year;
  
  util_set_date_values(t , &mday , &month , &year);
  fprintf(stream , "%02d/%02d/%04d   %7.1f days     %04d \n", mday , month , year , days , restart_nr);
}




void sched_file_summarize(const sched_file_type * sched_file , FILE * stream) {
  int len            = sched_file_get_num_restart_files(sched_file);
  time_t  start_time = sched_file_iget_block_start_time(sched_file , 0);
  for(int i=1; i<len; i++) {
    time_t t = sched_file_iget_block_start_time(sched_file , i);
    __sched_file_summarize_line(i - 1 , start_time , t , stream);
  }
  {
    time_t t = sched_file_iget_block_end_time(sched_file , len - 1);
    __sched_file_summarize_line(len - 1 , start_time , t , stream);
  }
}


/** 
    deep_copy is NOT implemented. With shallow_copy you get a new
    container (i.e. vector) , but the node content is unchanged.
*/


sched_file_type * sched_file_alloc_copy(const sched_file_type * src , bool deep_copy) {
  int ikw;
  sched_file_type * target = sched_file_alloc(src->start_time);
  
  for (ikw = 0; ikw < vector_get_size( src->kw_list ); ikw++) {
    sched_kw_type * kw = vector_iget( src->kw_list , ikw );
    sched_file_add_kw( target , kw );
  }
                                                                
  
  {
    int i;
    for (i = 0; i < stringlist_get_size( src->files ); i++) {
      if (deep_copy)
        stringlist_append_copy( target->files , stringlist_iget(src->files , i));
      else
        stringlist_append_ref( target->files , stringlist_iget(src->files , i));
    }
  }

  sched_file_update_index( target );
  return target;
}


/*****************************************************************/


static void sched_file_update_block(sched_block_type * block , 
                                    int restart_nr, 
                                    sched_kw_type_enum          kw_type , 
                                    sched_file_callback_ftype * callback,
                                    void * arg) {
  int ikw;
  for (ikw = 0; ikw < vector_get_size(block->kw_list); ikw++) {
    sched_kw_type * sched_kw = sched_block_iget_kw( block , ikw);
    if (sched_kw_get_type( sched_kw ) == kw_type)
      callback( sched_kw_get_data( sched_kw) , restart_nr , arg);  /* Calling back to 'user-space' to actually do the update. */
  }
}



/**
   This function is designed to facilitate 'user-space' update of the
   keywords in the schedule file based on callbacks. The function is
   called with two report steps, a type ID of the sched_kw type which
   should be updated, and a function pointer which will be invoked on
   all the relevant keywords. 
*/



void sched_file_update_blocks(sched_file_type * sched_file, 
                              int restart1 , 
                              int restart2 , 
                              sched_kw_type_enum kw_type,
                              sched_file_callback_ftype * callback,
                              void * callback_arg) {

  int restart_nr;
  if (restart2 > sched_file_get_num_restart_files(sched_file))
    restart2 = sched_file_get_num_restart_files(sched_file) - 1;
  
  for (restart_nr = restart1; restart_nr <= restart2; restart_nr++) {
    sched_block_type * sched_block = sched_file_iget_block( sched_file , restart_nr );
    sched_file_update_block( sched_block , restart_nr , kw_type , callback , callback_arg);
  }
}



/** 
    Update a complete schedule file by using callbacks to
    'user-space'. Say for instance you want to scale up the oilrate in
    well P1. This could be achieved with the following code:

       -- This function is written by the user of the library - in a remote scope.

       void increase_orat_callback(void * void_kw , int restart_nr , void * arg) {
          double scale_factor  = *(( double * ) arg);
          sched_kw_wconhist_type * kw = sched_kw_wconhist_safe_cast( void_kw );
          sched_kw_wconhist_scale_orat( wconhist_kw , "P1" , scale_factor);
       }

       ....
       ....
       
       sched_file_update(sched_file , WCONHIST , increase_orat_callback , &scale_factor);

    Observe the following about the callback:
  
      * The sched_kw input argument comes as a void pointer, and an
        sched_kw_xxx_safe_cast() function should be used on input to
        check.

      * The user-space level does *NOT* have access to the internals
        of the sched_kw_xxxx type, so the library must provide
        functions for the relevant state modifications.

      * The last argumnt (void * arg) - can of course be anything and
        his brother.

*/


void sched_file_update(sched_file_type * sched_file, 
                       sched_kw_type_enum kw_type,
                       sched_file_callback_ftype * callback,
                       void * callback_arg) {

  sched_file_update_blocks(sched_file , 1 , sched_file_get_num_restart_files(sched_file) - 1 , kw_type , callback , callback_arg);

}



/*****************************************************************/
/**
   This function will count the number of TSTEP / DATES keywords in a
   schedule file, without actually internalizing the file. This
   function 'should' be used for schedule files which we do not manage
   to parse.
*/

int sched_file_step_count( const char * filename ) {
  stringlist_type * token_list = sched_file_tokenize( filename );
  int token_index = 0;
  int step_count  = 0;
  do {
    const char * current_token = stringlist_iget( token_list , token_index );
    sched_kw_type_enum kw_type = sched_kw_type_from_string( current_token );

    if (kw_type == DATES) {
      sched_kw_type * sched_kw             = sched_kw_token_alloc( token_list , &token_index , NULL , NULL);
      const sched_kw_dates_type * dates_kw = sched_kw_get_data( sched_kw );
      step_count += sched_kw_dates_get_size( dates_kw );
      sched_kw_free( sched_kw );
    } else if (kw_type == TSTEP ) {
      sched_kw_type * sched_kw             = sched_kw_token_alloc( token_list , &token_index , NULL , NULL);
      const sched_kw_tstep_type * tstep_kw = sched_kw_get_data( sched_kw );
      step_count += sched_kw_tstep_get_length( tstep_kw );
      sched_kw_free( sched_kw );
    } else 
      token_index++;
    
  } while ( token_index < stringlist_get_size( token_list ));
  stringlist_free( token_list );
  return step_count;
}


/*****************************************************************/


//void sched_file_merge( const char * filename , time_t start_date , time_t insert_date , bool append_string , const char * insert_string) {
//  stringlist_type * token_list = sched_file_tokenize( filename );
//  int  token_index = 0;
//  int  step_count  = 0;
//  bool has_date    = false;
//
//  do {
//    time_t       current_time  = start_date;
//    const char * current_token = stringlist_iget( token_list , token_index );
//    sched_kw_type_enum kw_type = sched_kw_type_from_string( current_token );
//
//    if ((kw_type == DATES) || (kw_type == TSTEP)) {
//      int istep , num_step;
//      sched_kw_type *  sched_kw = sched_kw_token_alloc( token_list , &token_index , NULL);
//      sched_kw_type ** split_kw = sched_kw_split_alloc_DATES( sched_kw , &num_step );
//      
//      for (istep = 0; istep  < num_step; istep++) {
//        if (kw_type == DATES) {
//          const sched_kw_dates_type * dates_kw = sched_kw_get_data( sched_kw );
//          current_time = sched_kw_dates_iget_date( dates_kw , 0 );
//        }
//        if (current_time == insert_date) {
//          /* 
//             We are currently looking at a DATES keyword EXACTLY at
//             the date we should insert at. If append_string == false
//             we let the new string go in immediately, otherwise we set
//             has_date to true and continue one more time_step (then
//             the current_time > insert_date test will kick in).
//          */
//          has_date = true;
//          if (!append_string) {
//            
//          }
//        }
//      }
//      sched_kw_free( sched_kw );
//      
//    } else 
//      token_index++;
//    
//  } while ( token_index < stringlist_get_size( token_list ));
//
//}
//


/*****************************************************************/



/**
   Currently ONLY applicable to WCONHIST producers.

   
*/

bool sched_file_well_open( const sched_file_type * sched_file , 
                           int restart_nr , 
                           const char * well_name) {

  bool well_found = false;
  bool well_open  = false;
  int block_nr    = restart_nr;
  while (!well_found && (block_nr >= 0)) {
    sched_block_type * block = sched_file_iget_block( sched_file , block_nr );
    
    if (hash_has_key( block->kw_hash , "WCONHIST")) {
      const vector_type * wconhist_vector = hash_get( block->kw_hash , "WCONHIST");
      int i;
      for (i=0; i < vector_get_size( wconhist_vector ); i++) {
        const sched_kw_type * kw = vector_iget_const( wconhist_vector , i );
        if (sched_kw_has_well( kw , well_name )) {
          well_found = true;
          well_open = sched_kw_well_open( kw , well_name );
        }
      }
    }

    
    if (hash_has_key( block->kw_hash , "WCONINJE")) {
      const vector_type * wconinje_vector = hash_get( block->kw_hash , "WCONINJE");
      int i;
      for (i=0; i < vector_get_size( wconinje_vector ); i++) {
        const sched_kw_type * kw = vector_iget_const( wconinje_vector , i );
        if (sched_kw_has_well( kw , well_name )) {
          well_found = true;
          well_open  = sched_kw_well_open( kw , well_name );
        }
      }
    }
    


    block_nr--;
  } 
  return well_open;
}





double sched_file_well_wconhist_rate( const sched_file_type * sched_file , 
                                      int restart_nr , 
                                      const char * well_name) {
  double rate = -1;
  bool well_found = false;
  int block_nr    = restart_nr;

  while (!well_found && (block_nr >= 0)) {
    sched_block_type * block = sched_file_iget_block( sched_file , block_nr );
    
    if (hash_has_key( block->kw_hash , "WCONHIST")) {
      const vector_type * wconhist_vector = hash_get( block->kw_hash , "WCONHIST");
      int i;
      for (i=0; i < vector_get_size( wconhist_vector ); i++) {
        sched_kw_type * kw = vector_iget( wconhist_vector , i );
        if (sched_kw_has_well( kw , well_name )) {
          well_found = true;
          rate = sched_kw_wconhist_get_orat( sched_kw_get_data( kw ) , well_name );
        }
      }
    }
    block_nr--;
  } 
  return rate;
}



double sched_file_well_wconinje_rate( const sched_file_type * sched_file , 
                                      int restart_nr , 
                                      const char * well_name) {
  double rate = -1;
  bool well_found = false;
  int block_nr    = restart_nr;

  while (!well_found && (block_nr >= 0)) {
    sched_block_type * block = sched_file_iget_block( sched_file , block_nr );
    
    if (hash_has_key( block->kw_hash , "WCONINJE")) {
      const vector_type * wconhist_vector = hash_get( block->kw_hash , "WCONINJE");
      int i;
      for (i=0; i < vector_get_size( wconhist_vector ); i++) {
        sched_kw_type * kw = vector_iget( wconhist_vector , i );
        if (sched_kw_has_well( kw , well_name )) {
          well_found = true;
          rate = sched_kw_wconinje_get_surface_flow( sched_kw_get_data( kw ) , well_name );
        }
      }
    }
    
    block_nr--;
  } 
  return rate;
}

