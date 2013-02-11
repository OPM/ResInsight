/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'block_fs_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/path_fmt.h>
#include <ert/util/block_fs.h>
#include <ert/util/buffer.h>
#include <ert/util/timer.h>
#include <ert/util/thread_pool.h>

#include <ert/enkf/fs_types.h>
#include <ert/enkf/fs_driver.h>
#include <ert/enkf/block_fs_driver.h>
#include <ert/enkf/enkf_types.h>


typedef struct bfs_struct bfs_type;
typedef struct bfs_config_struct bfs_config_type;

struct bfs_config_struct {
  int             fsync_interval;
  double          fragmentation_limit;
  bool            read_only;
  bool            preload;
  int             block_size;
  int             max_cache_size;
};


#define  BFS_TYPE_ID  5510643

struct bfs_struct  {
  UTIL_TYPE_ID_DECLARATION;
  /*-----------------------------------------------------------------*/
  /* New variables */
  block_fs_type * block_fs;
  char          * mountfile;  // The full path to the file mounted by the block_fs layer - including extension. 

  const bfs_config_type * config;
};



struct block_fs_driver_struct {
  FS_DRIVER_FIELDS;
  int                __id;
  int                num_fs;
  bfs_config_type  * config;
  
  // New variables
  bfs_type        ** fs_list;
}; 

/*****************************************************************/

bfs_config_type * bfs_config_alloc( fs_driver_enum driver_type , bool read_only) {
  const int STATIC_blocksize       = 64;
  const int PARAMETER_blocksize    = 64;
  const int DYNAMIC_blocksize      = 64;
  const int DEFAULT_blocksize      = 64;

  const bool STATIC_preload        = false;
  const bool PARAMETER_preload     = false;
  const bool DYNAMIC_preload       = true;
  const bool DEFAULT_preload       = false;

  const int max_cache_size         = 512; 
  const int fsync_interval         =  10;     /* An fsync() call is issued for every 10'th write. */
  const double fragmentation_limit = 1.0;     /* 1.0 => NO defrag is run. */

  {
    bfs_config_type * config = util_malloc( sizeof * config );
    config->max_cache_size      = max_cache_size;
    config->fsync_interval      = fsync_interval;
    config->fragmentation_limit = fragmentation_limit;
    config->read_only           = read_only;
    
    switch (driver_type) {
    case( DRIVER_PARAMETER ):
      config->block_size = PARAMETER_blocksize;
      config->preload = PARAMETER_preload;
      break;
    case( DRIVER_STATIC ):
      config->block_size = STATIC_blocksize;
      config->preload = STATIC_preload;
      break;
    case(DRIVER_DYNAMIC_FORECAST):
    case(DRIVER_DYNAMIC_ANALYZED):
      config->block_size = DYNAMIC_blocksize;
      config->preload = DYNAMIC_preload;
      break;
    default:
      config->block_size = DEFAULT_blocksize;
      config->preload = DEFAULT_preload;
    }
    return config;
  }
}

void bfs_config_free( bfs_config_type * config ) {
  free( config );
}

/*****************************************************************/

static UTIL_SAFE_CAST_FUNCTION(bfs , BFS_TYPE_ID);

static void bfs_close( bfs_type * bfs ) {
  if (bfs->block_fs != NULL)
    block_fs_close( bfs->block_fs , false);
  free( bfs );
}


static void * bfs_close__( void * arg ) {
  bfs_type * bfs = ( bfs_type * ) arg;
  bfs_close( bfs );
  
  return NULL;
}

static bfs_type * bfs_alloc( const bfs_config_type * config ) {
  bfs_type * fs = util_malloc( sizeof * fs );
  UTIL_TYPE_ID_INIT( fs , BFS_TYPE_ID );
  fs->config         = config;
  
  // New init
  fs->mountfile = NULL;
  
  return fs;
}


static bfs_type * bfs_alloc_new( const bfs_config_type * config , char * mountfile) {
  bfs_type * bfs      = bfs_alloc( config );

  bfs->mountfile = mountfile;   // Warning pattern break: This is allocated in external scope; and the bfs takes ownership.
  return bfs;
}


static void bfs_mount( bfs_type * bfs ) {
  const bfs_config_type * config = bfs->config;
  bfs->block_fs = block_fs_mount( bfs->mountfile , 
                                  config->block_size , 
                                  config->max_cache_size , 
                                  config->fragmentation_limit , 
                                  config->fsync_interval , 
                                  config->preload , 
                                  config->read_only );
}


static void * bfs_mount__( void * arg ) {
  bfs_type * bfs = bfs_safe_cast( arg );
  bfs_mount( bfs );
  //printf("."); 
  //fflush( stdout );
  return NULL;
}




static void bfs_fsync( bfs_type * bfs ) {
  block_fs_fsync( bfs->block_fs );
}



/*****************************************************************/


static void block_fs_driver_assert_cast(block_fs_driver_type * block_fs_driver) {
  if (block_fs_driver->__id != BLOCK_FS_DRIVER_ID) 
    util_abort("%s: internal error - cast failed - aborting \n",__func__);
}


static block_fs_driver_type * block_fs_driver_safe_cast( void * __driver) {
  block_fs_driver_type * driver = (block_fs_driver_type *) __driver;
  block_fs_driver_assert_cast(driver);
  return driver;
}

static char * block_fs_driver_alloc_node_key( const block_fs_driver_type * driver , const char * node_key , int report_step , int iens) {
  char * key = util_alloc_sprintf("%s.%d.%d" , node_key , report_step , iens);
  return key;
}


static char * block_fs_driver_alloc_vector_key( const block_fs_driver_type * driver , const char * node_key , int iens) {
  char * key = util_alloc_sprintf("%s.%d" , node_key , iens);
  return key;
}

/**
   This function will take an input string, and try to to parse it as
   string.int.int, where string is the normal enkf key, and the two
   integers are report_step and ensemble number respectively. The
   storage for the enkf_key is allocated here in this function, and
   must be freed by the calling scope.  

   If the parsing fails the function will return false, and *config_key
   will be set to NULL; in this case the report_step and iens poinyers
   will not be touched.
*/

bool block_fs_sscanf_key(const char * key , char ** config_key , int * __report_step , int * __iens) {
  char ** tmp;
  int num_items;

  *config_key = NULL;
  util_split_string(key , "." , &num_items , &tmp);  /* The key can contain additional '.' - can not use sscanf(). */
  if (num_items >= 3) {
    int report_step , iens;
    if (util_sscanf_int(tmp[num_items - 2] , &report_step) && util_sscanf_int(tmp[num_items - 1] , &iens)) {
      /* OK - all is hunkadory */
      *__report_step = report_step;
      *__iens        = iens;
      *config_key    = util_alloc_joined_string((const char **) tmp , num_items - 2 , ".");  /* This must bee freed by the calling scope */
      util_free_stringlist( tmp , num_items );
      return true;
    } else  
      /* Failed to parse the two last items as integers. */
      return false;
  } else
    /* Did not have at least three items. */
    return false;
}




static bfs_type * block_fs_driver_get_fs( block_fs_driver_type * driver , int iens ) {
  int phase                = (iens % driver->num_fs);
  
  return driver->fs_list[phase];
}



static void block_fs_driver_load_node(void * _driver , const char * node_key , int report_step , int iens ,  buffer_type * buffer) {
  block_fs_driver_type * driver = block_fs_driver_safe_cast( _driver );
  {
    char * key          = block_fs_driver_alloc_node_key( driver , node_key , report_step , iens );
    bfs_type      * bfs = block_fs_driver_get_fs( driver , iens );
    
    block_fs_fread_realloc_buffer( bfs->block_fs , key , buffer);
    
    free( key );
  }
}


static void block_fs_driver_load_vector(void * _driver , const char * node_key , int iens ,  buffer_type * buffer) {
  block_fs_driver_type * driver = block_fs_driver_safe_cast( _driver );
  {
    char * key          = block_fs_driver_alloc_vector_key( driver , node_key , iens );
    bfs_type      * bfs = block_fs_driver_get_fs( driver , iens );
    
    block_fs_fread_realloc_buffer( bfs->block_fs , key , buffer);
    free( key );
  }
}

/*****************************************************************/

static void block_fs_driver_save_node(void * _driver , const char * node_key , int report_step , int iens ,  buffer_type * buffer) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  {
    char * key     = block_fs_driver_alloc_node_key( driver , node_key , report_step , iens );
    bfs_type * bfs = block_fs_driver_get_fs( driver , iens );
    block_fs_fwrite_buffer( bfs->block_fs , key , buffer);
    free( key );
  }
}


static void block_fs_driver_save_vector(void * _driver , const char * node_key , int iens ,  buffer_type * buffer) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  {
    char * key     = block_fs_driver_alloc_vector_key( driver , node_key , iens );
    bfs_type * bfs = block_fs_driver_get_fs( driver , iens );
    block_fs_fwrite_buffer( bfs->block_fs , key , buffer);
    free( key );
  }
}

/*****************************************************************/

void block_fs_driver_unlink_node(void * _driver , const char * node_key , int report_step , int iens ) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  {
    char * key     = block_fs_driver_alloc_node_key( driver , node_key , report_step , iens );
    bfs_type * bfs = block_fs_driver_get_fs( driver , iens );
    block_fs_unlink_file( bfs->block_fs , key );
    free( key );
  }
}

void block_fs_driver_unlink_vector(void * _driver , const char * node_key , int iens ) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  {
    char * key     = block_fs_driver_alloc_vector_key( driver , node_key , iens );
    bfs_type * bfs = block_fs_driver_get_fs( driver , iens );
    block_fs_unlink_file( bfs->block_fs , key );
    free( key );
  }
}


/*****************************************************************/

bool block_fs_driver_has_node(void * _driver , const char * node_key , int report_step , int iens ) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  {
    char * key      = block_fs_driver_alloc_node_key( driver , node_key , report_step , iens );
    bfs_type  * bfs = block_fs_driver_get_fs( driver , iens );
    bool has_node   = block_fs_has_file( bfs->block_fs , key );
    free( key );
    return has_node;
  }
}


bool block_fs_driver_has_vector(void * _driver , const char * node_key , int iens ) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  {
    char * key      = block_fs_driver_alloc_vector_key( driver , node_key , iens );
    bfs_type  * bfs = block_fs_driver_get_fs( driver , iens );
    bool has_node   = block_fs_has_file( bfs->block_fs , key );
    free( key );
    return has_node;
  }
}

/*****************************************************************/









void block_fs_driver_free(void *_driver) {
  block_fs_driver_type * driver = block_fs_driver_safe_cast( _driver );
  {
    int driver_nr;
    thread_pool_type * tp         = thread_pool_alloc( 4 , true);
    for (driver_nr = 0; driver_nr < driver->num_fs; driver_nr++) 
      thread_pool_add_job( tp , bfs_close__ , driver->fs_list[driver_nr] );

    thread_pool_join( tp );
    thread_pool_free( tp );
  }
  bfs_config_free( driver->config );
  free( driver->fs_list );
  free(driver);
}



static void block_fs_driver_fsync( void * _driver ) {
  block_fs_driver_type * driver = (block_fs_driver_type *) _driver;
  block_fs_driver_assert_cast(driver);
  
  {
    int driver_nr;
    block_fs_driver_type * driver = block_fs_driver_safe_cast(_driver);
    for (driver_nr = 0; driver_nr < driver->num_fs; driver_nr++)  
      bfs_fsync( driver->fs_list[driver_nr] );
  }
}


static block_fs_driver_type * block_fs_driver_alloc(int num_fs) {
  block_fs_driver_type * driver = util_malloc(sizeof * driver );
  {
    fs_driver_type * fs_driver = (fs_driver_type *) driver;
    fs_driver_init(fs_driver);
  }
  driver->load_node     = block_fs_driver_load_node;
  driver->save_node     = block_fs_driver_save_node;
  driver->unlink_node   = block_fs_driver_unlink_node;
  driver->has_node      = block_fs_driver_has_node;

  driver->load_vector   = block_fs_driver_load_vector;
  driver->save_vector   = block_fs_driver_save_vector;
  driver->unlink_vector = block_fs_driver_unlink_vector;
  driver->has_vector    = block_fs_driver_has_vector;

  driver->free_driver   = block_fs_driver_free;
  driver->fsync_driver  = block_fs_driver_fsync;
  driver->__id          = BLOCK_FS_DRIVER_ID;
  driver->num_fs        = num_fs;

  driver->fs_list       = util_calloc( driver->num_fs , sizeof * driver->fs_list ); 
  return driver;
}




static void * block_fs_driver_alloc_new( fs_driver_enum driver_type , bool read_only , int num_fs , const char * mountfile_fmt ) {
  block_fs_driver_type * driver = block_fs_driver_alloc( num_fs );
  driver->config = bfs_config_alloc( driver_type , read_only );
  {
    for (int ifs = 0; ifs < driver->num_fs; ifs++) 
      driver->fs_list[ifs] = bfs_alloc_new( driver->config , util_alloc_sprintf( mountfile_fmt , ifs) );
  }
  return driver;
}


static void block_fs_driver_mount( block_fs_driver_type * driver ) {
  thread_pool_type * tp = thread_pool_alloc( 4 , true ); 

  for (int ifs = 0; ifs < driver->num_fs; ifs++) 
    thread_pool_add_job( tp , bfs_mount__ , driver->fs_list[ ifs ]);

  thread_pool_join( tp );
  thread_pool_free( tp );
}





/*****************************************************************/

void block_fs_driver_create_fs( FILE * stream , 
                                const char * mount_point , 
                                fs_driver_enum driver_type , 
                                int num_fs , 
                                const char * ens_path_fmt, 
                                const char * filename ) {
  
  util_fwrite_int(driver_type , stream );
  util_fwrite_int(num_fs , stream );
  {
    char * mountfile_fmt = util_alloc_sprintf("%s%c%s.mnt" , ens_path_fmt , UTIL_PATH_SEP_CHAR , filename );
    util_fwrite_string( mountfile_fmt , stream );
    free( mountfile_fmt );
  }
  
  for (int ifs = 0; ifs < num_fs; ifs++) {
    char * path_fmt = util_alloc_sprintf("%s%c%s" , mount_point , UTIL_PATH_SEP_CHAR , ens_path_fmt);
    char * ens_path = util_alloc_sprintf(path_fmt , ifs);
    
    util_make_path( ens_path );
    
    free( path_fmt );
    free( ens_path );
  }

}


/*
  @path should contain both elements called root_path and case_path in
  the block_fs_driver_create() function.  
*/

void * block_fs_driver_open(FILE * fstab_stream , const char * mount_point , fs_driver_enum driver_type , bool read_only) {
  int num_fs           = util_fread_int( fstab_stream ); 
  char * tmp_fmt       = util_fread_alloc_string( fstab_stream );
  char * mountfile_fmt = util_alloc_sprintf("%s%c%s" , mount_point , UTIL_PATH_SEP_CHAR , tmp_fmt );
  
  block_fs_driver_type * driver = block_fs_driver_alloc_new( driver_type , read_only , num_fs , mountfile_fmt );
  
  block_fs_driver_mount( driver );
  
  free( tmp_fmt );
  free( mountfile_fmt );
  return driver;
}

