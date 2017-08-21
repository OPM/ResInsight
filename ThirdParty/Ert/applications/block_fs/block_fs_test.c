#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <util.h>
#include <block_fs.h>
#include <hash.h>


typedef enum {
  WRITE_FILE  = 1,
  DELETE_FILE = 2,
  CHECK       = 3,
  ROTATE      = 4
} test_action_enum;

#define WRITE_FILE_STRING  "WRITE_FILE"
#define DELETE_FILE_STRING "DELETE_FILE"
#define CHECK_STRING       "CHECK"
#define ROATATE_STRING     "ROTATE"


typedef struct {
  test_action_enum   action;
  char             * filename;
  int                length;
} action_node_type;


action_node_type * action_node_alloc_new() {
  action_node_type * node = util_malloc( sizeof * node );
  node->filename = NULL;
  return node;
}

void action_node_free(action_node_type * node) {
  util_safe_free( node->filename );
  free( node );
}

void action_node_update(action_node_type * node, test_action_enum action , char * filename) {
  util_safe_free( node->filename );
  node->filename = filename;  /* Node takes ownership of filename. */
  node->action   = action;
}


void apply_delete_file( const action_node_type * node , block_fs_type * fs , const char * file_path) {
  if (block_fs_has_file( fs , node->filename)) {
    char * filename = util_alloc_filename( file_path , node->filename , NULL );
    block_fs_unlink_file( fs , node->filename );
    unlink( filename );
    free( filename );
  }
}

void apply_check( block_fs_type * fs , const char * file_path ) {
  char * buffer1 = NULL;
  char * buffer2 = NULL;

  int current_size = 0;
  vector_type   * files = block_fs_alloc_filelist( fs , NULL  , NO_SORT , false );
  int i;
  for (i=0; i < vector_get_size( files ); i++) {
    const user_file_node_type * node = vector_iget_const( files , i );
    int size = user_file_node_get_data_size( node );
    if (size > current_size) {
      current_size = size;
      buffer1 = util_realloc( buffer1 , current_size );
      buffer2 = util_realloc( buffer2 , current_size );
    }
    block_fs_fread_file( fs , user_file_node_get_filename( node ) , buffer1 );
    {
      char * filename = util_alloc_filename( file_path , user_file_node_get_filename( node ) , NULL);
      FILE * stream   = util_fopen( filename , "r");
      util_fread( buffer2 , 1 , size , stream , __func__);
      fclose( stream );
      free( filename );
    }
    if (memcmp( buffer1 , buffer2 , size ) != 0)
      fprintf(stderr,"Fatal error ..\n");
  }
  vector_free( files );
  free( buffer1 );
  free( buffer2 );
  printf("CHeck OK \n");
}


void apply_write_file( const action_node_type * node , block_fs_type * fs , const char * file_path) {
  const int short_min = 128;
  const int short_max = 256;
  const int long_min  = 4096;
  const int long_max  = 32000;
  const double p_short = 0.75;
  int length;

  
  {
    int min,max;
    double R = rand() * 1.0 / RAND_MAX;
    if (R < p_short) {
      min = short_min;
      max = short_max;
    } else {
      min = long_min;
      max = long_max;
    }
    length = min + (rand() % (max - min + 1));
  }
  {
    char * buffer = util_malloc( length * sizeof * buffer );
    int i;
    for (i=0; i < length; i++)
      buffer[i] = rand() % 256;
    block_fs_fwrite_file( fs , node->filename , buffer , length);
    {
      char * filename = util_alloc_filename(  file_path , node->filename , NULL );
      FILE * stream = util_fopen( filename , "w");
      util_fwrite( buffer , 1 , length , stream , __func__ );
      fclose( stream );
      free(filename);
    }
    free( buffer );
  }
}



void apply( const action_node_type ** action_list , int action_length, block_fs_type * fs , const char * file_path) {
  int i;
  for (i=0; i < action_length; i++) {
    const action_node_type * node = action_list[i];
    switch( node->action) {
    case WRITE_FILE:
      apply_write_file( node , fs , file_path );
      break;
    case DELETE_FILE:
      apply_delete_file( node , fs , file_path );
      break;
    case CHECK:
      apply_check( fs , file_path );
      break;
    case ROTATE:
      block_fs_rotate( fs , 0.00 );
      break;
    default:
      util_abort("Unrecognized enum value:%d \n", node->action );
    }
  }
}


int main(int argc, char ** argv) {
  
  const char * test_path  = "/tmp/block_fs"; 
  const char * file_fmt   = "file_%04d";
  char * file_path        = util_alloc_filename( test_path , "files" , NULL );
  char * test_mnt         = util_alloc_filename( test_path , "FORECAST" , "mnt");
  char * test_log         = util_alloc_filename( test_path , "test" , "log");

  const int    test_run   =    10;
  const int    block_size =  1000;
  const int    max_files  =  1000;
  
  const int fs_block_size  = 32;
  const int cache_size     = 2048;
  const float frag_limit   = 0.25;
  const int fsync_interval = 100;
  const bool preload       = false;
  int run = 0;

  block_fs_type * block_fs = block_fs_mount( test_mnt , fs_block_size , cache_size , frag_limit , fsync_interval , preload , false );
  
  action_node_type ** action_list = util_malloc( block_size * sizeof * action_list );
  {
    int i;
    for (i=0; i < block_size; i++) {
      action_list[i] = action_node_alloc_new(  );
    }
  }

  while ( run < test_run) {
    int action_nr = 0;

    run++;
    while (action_nr < (block_size - 2)) {
      char * filename = NULL;
      int R = rand() % 3;
      if (R == 0) {
        /* Delete file */
        filename = util_alloc_sprintf( file_fmt , rand() % max_files );
        action_node_update( action_list[ action_nr ] , DELETE_FILE  , filename );
      } else {
        /* Create file */
        filename = util_alloc_sprintf( file_fmt , rand() % max_files );
        action_node_update( action_list[ action_nr ] , WRITE_FILE  , filename );
      }

      action_nr++;
    }
    action_node_update( action_list[ block_size - 2] , ROTATE  , NULL);
    action_node_update( action_list[ block_size - 1] , CHECK   , NULL);
    
    apply( (const action_node_type **) action_list , block_size , block_fs , file_path );
    //fprintf_log( log_stream , action_list , block_size );
    printf("*"); fflush( stdout );
  }
  
  {
    int i;
    for (i=0; i < block_size; i++) 
      action_node_free( action_list[i] );
    
  }
  free( action_list );
  block_fs_close( block_fs , true);
  
  free( file_path );
  free( test_mnt );
  free( test_log );
}
