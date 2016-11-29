/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'block_fs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fnmatch.h>

#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/block_fs.h>
#include <ert/util/vector.h>
#include <ert/util/buffer.h>
#include <ert/util/long_vector.h>


#define MOUNT_MAP_MAGIC_INT  8861290
#define BLOCK_FS_TYPE_ID     7100652
#define INDEX_MAGIC_INT      1213775
#define INDEX_FORMAT_VERSION       1

// #define ENABLE_CACHE


/*
  During mounting a significant part of the time is spent on filling
  up the index hash table. By default a hash table is created with a
  quite small size, and when initializing a large block_fs structure
  it must be resized many times. By setting a default size with the
  DEFAULT_INDEX_SIZE variable the hash table will immediately be
  resized, avoiding some of the automatic calls to hash_resize. 

  When the file system is loaded from an index a good size estimate
  can be inferred directly from the index.  
*/

#define DEFAULT_INDEX_SIZE 2048



/**
   These should be bitwise "smart" - so it is possible
   to go on a wild chase through a binary stream and look for them.
*/

#define NODE_IN_USE_BYTE  85                /* Binary(85)  =  01010101 */
#define NODE_FREE_BYTE   170                /* Binary(170) =  10101010 */   
#define WRITE_START__    77162

static const int NODE_END_TAG            = 16711935;       /* Binary      =  00000000111111110000000011111111 */
static const int NODE_WRITE_ACTIVE_START = WRITE_START__;
static const int NODE_WRITE_ACTIVE_END   = 776512;


typedef enum {
  NODE_IN_USE       =  1431655765,    /* NODE_IN_USE_BYTE * ( 1 + 256 + 256**2 + 256**3) => Binary 01010101010101010101010101010101 */
  NODE_FREE         = -1431655766,    /* NODE_FREE_BYTE   * ( 1 + 256 + 256**2 + 256**3) => Binary 10101010101010101010101010101010 */
  NODE_WRITE_ACTIVE =  WRITE_START__, /* This */
  NODE_INVALID      = 13              /* This should __never__ be written to disk */
} node_status_type;


/**
   The free_node_struct is used to implement a doubly linked list of
   free nodes; i.e. holes in the file which are available for other use. 
*/
typedef struct file_node_struct file_node_type;
typedef struct free_node_struct free_node_type;

struct free_node_struct {
  free_node_type * next;
  free_node_type * prev;
  file_node_type * file_node;
};



/*
  Datastructure representing one 'block' in the datafile. The block
  can either refer to a file (status == NODE_IN_USE) or to an empty
  slot in the datafile (status == NODE_FREE).
*/

struct file_node_struct{
  long int           node_offset;   /* The offset into the data_file of this node. NEVER Changed. */
  int                data_offset;   /* The offset from the node start to the start of actual data - i.e. data starts at absolute position: node_offset + data_offset. */
  int                node_size;     /* The size in bytes of this node - must be >= data_size. NEVER Changed. */
  int                data_size;     /* The size of the data stored in this node - in addition the node might need to store header information. */
  node_status_type   status;        /* This should be: NODE_IN_USE | NODE_FREE; in addition the disk can have NODE_WRITE_ACTIVE for incomplete writes. */

#ifdef ENABLE_CACHE
  char             * cache;
  int                cache_size;
#endif
};


/**
   data_size   : manipulated in block_fs_fwrite__() and block_fs_insert_free_node().
   status      : manipulated in block_fs_fwrite__() and block_fs_unlink_file__();
   data_offset : manipulated in block_fs_fwrite__() and block_fs_insert_free_node().
*/






struct block_fs_struct {
  UTIL_TYPE_ID_DECLARATION;
  char           * mount_file;    /* The full path to a file with some mount information - input to the mount routine. */
  char           * path;          
  char           * base_name;
  
  int              version;       /* A version number which is incremented each time the filesystem is defragmented - not implemented yet. */
  
  char           * data_file;
  char           * lock_file;
  char           * index_file;
  
  int              data_fd;
  FILE           * data_stream;

  long int         data_file_size;  /* The total number of bytes in the data_file. */
  long int         free_size;       /* Size of 'holes' in the data file. */ 
  int              block_size;      /* The size of blocks in bytes. */
  int              lock_fd;         /* The file descriptor for the lock_file. Set to -1 if we do not have write access. */
  
  pthread_mutex_t  io_lock;         /* Lock held during fread of the data file. */
  pthread_rwlock_t rw_lock;         /* Read-write lock during all access to the fs. */
  
  int              num_free_nodes;   
  hash_type      * index;           /* THE HASH table of all the nodes/files which have been stored. */
  free_node_type * free_nodes;
  vector_type    * file_nodes;      /* This vector owns all the file_node instances - the index and free_nodes structures
                                       only contain pointers to the objects stored in this vector. */
  int              write_count;     /* This just counts the number of writes since the file system was mounted. */
  int              max_cache_size;
  size_t           total_cache_size;
  size_t           max_total_cache_size;
  float            fragmentation_limit;  /* If fragmentation (amount of wasted space) is above this limit - do a rotate.
                                            fragmentation_limit == 1.0 : Never rotate.
                                            fragmentation_limit == 0.0 : Rotate when one byte is wasted. */
  bool             data_owner;
  int              fsync_interval;  /* 0: never  n: every nth iteration. */
};

/*****************************************************************/

static void block_fs_rotate__( block_fs_type * block_fs );

UTIL_SAFE_CAST_FUNCTION( block_fs , BLOCK_FS_TYPE_ID )


static inline void fseek__(FILE * stream , long int arg , int whence) {
  if (fseek(stream , arg , whence) != 0) {
    fprintf(stderr,"** Warning - seek:%ld failed %s(%d) \n",arg,strerror(errno) , errno);
    util_abort("%S - aborting\n",__func__);
  }
}

static inline void block_fs_fseek(block_fs_type * block_fs , long offset) {
  fseek__( block_fs->data_stream , offset , SEEK_SET );
}


/*****************************************************************/
/* file_node functions */




/**
   Observe that the two input arguments to this function should NEVER
   change. They represent offset and size in the underlying data file,
   and that is for ever fixed.  
*/


static file_node_type * file_node_alloc( node_status_type status , long int offset , int node_size) {
  file_node_type * file_node = util_malloc( sizeof * file_node );
  
  file_node->node_offset = offset;    /* These should NEVER change. */
  file_node->node_size   = node_size; /* -------------------------  */

  file_node->data_size   = 0;
  file_node->data_offset = 0;
  file_node->status      = status; 
  
#ifdef ENABLE_CACHE
  file_node->cache      = NULL;
  file_node->cache_size = 0;
#endif

  return file_node;
}


/**
   This function is called from the functions exporting file_node
   instances; the file_node instance will then have a correct filename
   field immediately afterwards, but the normal block_fs functions
   (read/write/unlink) do NOT update this field, so it can quickly go
   out of sync.
*/


     



#ifdef ENABLE_CACHE
static void file_node_read_from_cache( const file_node_type * file_node , void * ptr , size_t read_bytes) {
  memcpy(ptr , file_node->cache , read_bytes);
  /*
    Could check: (ext_offset + file_node->cache_size <= read_bytes) - else
    we are reading beyond the end of the cache.
  */
}


static void file_node_buffer_read_from_cache( const file_node_type * file_node , buffer_type * buffer ) {
  buffer_fwrite( buffer , file_node->cache , 1 , file_node->cache_size );
}


static void file_node_update_cache( file_node_type * file_node , int data_size , const void * data) {
  if (data_size != file_node->cache_size) {
    file_node->cache = util_realloc_copy( file_node->cache , data , data_size );
    file_node->cache_size = data_size;
  } else 
    memcpy( file_node->cache , data , data_size);
}


static void file_node_clear_cache( file_node_type * file_node ) {
  if (file_node->cache != NULL) {
    file_node->cache_size = 0;
    free(file_node->cache);
    file_node->cache = NULL;
  }
}
#endif


static void file_node_free( file_node_type * file_node ) {
  free( file_node );
#ifdef ENABLE_CACHE
  util_safe_free( file_node->cache );
#endif
}


static void file_node_free__( void * file_node ) {
  file_node_free( (file_node_type *) file_node );
}



static bool file_node_verify_end_tag( const file_node_type * file_node , FILE * stream ) {
  int end_tag;
  fseek__( stream , file_node->node_offset + file_node->node_size - sizeof NODE_END_TAG , SEEK_SET);      
  if (fread( &end_tag , sizeof end_tag , 1 , stream) == 1) {
    if (end_tag == NODE_END_TAG) 
      return true; /* All hunkadory. */
    else
      return false;
  } else 
    return false;
}



static file_node_type * file_node_fread_alloc( FILE * stream , char ** key) {
  file_node_type * file_node = NULL;
  node_status_type status;
  long int node_offset = ftell( stream );
  if (fread( &status , sizeof status , 1 , stream) == 1) {
    if ((status == NODE_IN_USE) || (status == NODE_FREE)) {
      int node_size;
      if (status == NODE_IN_USE) 
        *key = util_fread_realloc_string( *key , stream );
      else {
        util_safe_free( *key );  /* Explicitly set to NULL for free nodes. */
        *key = NULL;
      }
      
      node_size = util_fread_int( stream );
      if (node_size <= 0)
        status = NODE_INVALID;
      /*
        A case has occured with an invalid node with size 0. That
        resulted in a deadlock, because the reader never got beyond
        the broken node. We therefor explicitly check for this
        condition.
      */
      
      file_node = file_node_alloc( status , node_offset , node_size );
      if (status == NODE_IN_USE) {
        file_node->data_size = util_fread_int( stream );
        file_node->data_offset    = ftell( stream ) - file_node->node_offset;
      }
    } else {
      /* 
         We did not recognize the status identifier; the node will
         eventually be marked as free. 
      */
      if (status != NODE_WRITE_ACTIVE)
        status = NODE_INVALID;
      file_node = file_node_alloc( status , node_offset , 0 );
    }
  }
  return file_node;
}


/**
   Internal index layout:

   |<InUse: Bool><Key: String><node_size: Int><data_size: Int>|
   |<InUse: Bool><node_size: Int><data_size: Int>|
             
  /|\                                                        
   |                                                         
   |<-------------------------------------------------------->|                                                         
                                                   |           
node_offset                                      offset

  The node_offset and offset values are not stored on disk, but rather
  implicitly read with ftell() calls.
*/

/**
   This function will write the node information to file, this
   includes the NODE_END_TAG identifier which shoule be written to the
   end of the node.
*/

static void file_node_fwrite( const file_node_type * file_node , const char * key , FILE * stream ) {
  if (file_node->node_size == 0)
    util_abort("%s: trying to write node with z<ero size \n",__func__);
  {
    fseek__( stream , file_node->node_offset , SEEK_SET);
    util_fwrite_int( file_node->status , stream );
    if (file_node->status == NODE_IN_USE)
      util_fwrite_string( key , stream );
    util_fwrite_int( file_node->node_size , stream );
    util_fwrite_int( file_node->data_size , stream );
    fseek__( stream , file_node->node_offset + file_node->node_size - sizeof NODE_END_TAG , SEEK_SET);
    util_fwrite_int( NODE_END_TAG , stream );
  }
}


/**
   This marks the start and end of the node with the integer tags:
   NODE_WRITE_ACTIVE_START and NODE_WRITE_ACTIVE_END, signalling this
   section in the data file is 'work in progress', and should be
   discarded if the application aborts during the write.

   When the write is complete file_node_fwrite() should be called,
   which will replace the NODE_WRITE_ACTIVE_START and
   NODE_WRITE_ACTIVE_END tags with NODE_IN_USE and NODE_END_TAG
   identifiers.
*/

   
static void file_node_init_fwrite( const file_node_type * file_node , FILE * stream) {
  fseek__( stream , file_node->node_offset , SEEK_SET );
  util_fwrite_int( NODE_WRITE_ACTIVE_START , stream );
  fseek__( stream , file_node->node_offset + file_node->node_size - sizeof NODE_END_TAG , SEEK_SET);
  util_fwrite_int( NODE_WRITE_ACTIVE_END   , stream );
}


/**
   Observe that header in this context include the size of the tail
   marker NODE_END_TAG.
*/
   
static int file_node_header_size( const char * filename ) {
  file_node_type * file_node;
  return sizeof ( file_node->status    ) + 
         sizeof ( file_node->node_size ) + 
         sizeof ( file_node->data_size ) + 
         sizeof ( NODE_END_TAG )         + sizeof(int) /* embedded by the util_fwrite_string routine */ + strlen(filename) + 1 /* \0 */;
}


static void file_node_set_data_offset( file_node_type * file_node, const char * filename ) {
  file_node->data_offset = file_node_header_size( filename ) - sizeof( NODE_END_TAG );
}



static void file_node_dump_index( const file_node_type * file_node , FILE * index_stream) {
  util_fwrite_int( file_node->status , index_stream );
  util_fwrite_long( file_node->node_offset , index_stream );
  util_fwrite_int( file_node->node_size   , index_stream );
  util_fwrite_int( file_node->data_offset , index_stream );
  util_fwrite_int( file_node->data_size   , index_stream );
}



/*
static file_node_type * file_node_index_fread_alloc( FILE * stream ) {
  node_status_type status = util_fread_int( stream );
  long int node_offset    = util_fread_long( stream );
  int node_size           = util_fread_int( stream );
  {
    file_node_type * file_node = file_node_alloc( status , node_offset , node_size );
    file_node->data_offset = util_fread_int( stream );
    file_node->data_size   = util_fread_int( stream );
    
    return file_node;
  }
}
*/

static file_node_type * file_node_index_buffer_fread_alloc( buffer_type * buffer) {
  node_status_type status = buffer_fread_int( buffer );
  long int node_offset    = buffer_fread_long( buffer );
  int node_size           = buffer_fread_int( buffer );
  {
    file_node_type * file_node = file_node_alloc( status , node_offset , node_size );

    file_node->data_offset = buffer_fread_int( buffer );
    file_node->data_size   = buffer_fread_int( buffer );
    
    return file_node;
  }
}

/* file_node functions - end. */
/*****************************************************************/

static free_node_type * free_node_alloc( file_node_type * file_node ) {
  free_node_type * free_node = util_malloc( sizeof * free_node );

  free_node->file_node = file_node;
  free_node->next = NULL;
  free_node->prev = NULL;

  return free_node;
}


static void free_node_free( free_node_type * free_node ) {
  free( free_node );
}

static void free_node_free_list( free_node_type * head ) {
  free_node_type * current = head;
  free_node_type * next;
  while (current != NULL) {
    next = current->next;
    free_node_free( current );
    current = next;
  }
}



/*****************************************************************/
static inline void block_fs_aquire_wlock( block_fs_type * block_fs ) {
  if (block_fs->data_owner)
    pthread_rwlock_wrlock( &block_fs->rw_lock );
  else
    util_abort("%s: tried to write to read only filesystem mounted at: %s \n",__func__ , block_fs->mount_file );
}


static inline void block_fs_release_rwlock( block_fs_type * block_fs ) {
  pthread_rwlock_unlock( &block_fs->rw_lock );
}


static inline void block_fs_aquire_rlock( block_fs_type * block_fs ) {
  pthread_rwlock_rdlock( &block_fs->rw_lock );
  /*
    We just assume that the user does NOT write to the filesystem
    with another instance; in that case we will go out of sync;
    and things will probably fail badly.
  */
}



static void block_fs_insert_index_node( block_fs_type * block_fs , const char * filename , const file_node_type * file_node) {
  hash_insert_ref( block_fs->index , filename , file_node);
}


/**
   Looks through the list of free nodes - looking for a node with
   offset 'node_offset'. If no such node can be found, NULL will be
   returned.
*/

static file_node_type * block_fs_lookup_free_node( const block_fs_type * block_fs , long int node_offset) {
  free_node_type * current = block_fs->free_nodes;
  while (current != NULL && (current->file_node->node_offset != node_offset)) 
    current = current->next;
  
  if (current == NULL)
    return NULL;
  else
    return current->file_node;
}









/**
   Inserts a file_node instance in the linked list of free nodes. The
   list is sorted in order of increasing node size.
*/

static void block_fs_insert_free_node( block_fs_type * block_fs , file_node_type * file_node ) {
  free_node_type * new = free_node_alloc( file_node );
  
  /* Special case: starting with a empty list. */
  if (block_fs->free_nodes == NULL) {
    new->next = NULL;
    new->prev = NULL;
    block_fs->free_nodes = new;
  } else {
    free_node_type * current = block_fs->free_nodes;
    free_node_type * prev    = NULL;
    
    while ( current != NULL && (current->file_node->node_size < file_node->node_size)) {
      prev = current;
      current = current->next;
    }
    
    if (current == NULL) {
      /* 
         The new node should be added at the end of the list - i.e. it
         will not have a next node.
      */
      new->next = NULL;
      new->prev = prev;
      prev->next = new;
    } else {
      /*
        The new node should be placed BEFORE the current node.
      */
      if (prev == NULL) {
        /* The new node should become the new list head. */
        block_fs->free_nodes = new;
        new->prev = NULL;
      } else {
        prev->next = new;
        new->prev  = prev;
      }
      current->prev = new;
      new->next  = current;
    }
    if (new != NULL)     if (new->next == new) util_abort("%s: broken LIST1 \n",__func__);
    if (prev != NULL)    if (prev->next == prev) util_abort("%s: broken LIST2 \n",__func__);
    if (current != NULL) if (current->next == current) util_abort("%s: Broken LIST3 \n",__func__);
  }
  block_fs->num_free_nodes++;
  block_fs->free_size += new->file_node->node_size;
}


/**
   Installing the new node AND updating file tail. 
*/

static void block_fs_install_node(block_fs_type * block_fs , file_node_type * node) {
  block_fs->data_file_size = util_size_t_max( block_fs->data_file_size , node->node_offset + node->node_size);  /* Updating the total size of the file - i.e the next available offset. */
  vector_append_owned_ref( block_fs->file_nodes , node , file_node_free__ );
}


static void block_fs_set_filenames( block_fs_type * block_fs ) {
  char * data_ext  = util_alloc_sprintf("data_%d" , block_fs->version );
  char * lock_ext  = util_alloc_sprintf("lock_%d" , block_fs->version );
  const char * index_ext = "index";

  util_safe_free( block_fs->data_file );
  util_safe_free( block_fs->lock_file );
  util_safe_free( block_fs->index_file );
  
  block_fs->data_file  = util_alloc_filename( block_fs->path , block_fs->base_name , data_ext);
  block_fs->lock_file  = util_alloc_filename( block_fs->path , block_fs->base_name , lock_ext);
  block_fs->index_file = util_alloc_filename( block_fs->path , block_fs->base_name , index_ext);

  free( data_ext );
  free( lock_ext );
}



/**
   This function is called both when allocating a new block_fs
   instance, and when an existing block_fs instance is 'rotated'.
*/


static void block_fs_reinit( block_fs_type * block_fs ) {
  block_fs->index               = hash_alloc_unlocked();
  block_fs->file_nodes          = vector_alloc_new();
  block_fs->free_nodes          = NULL;
  block_fs->num_free_nodes      = 0;
  block_fs->write_count         = 0;
  block_fs->data_file_size      = 0;
  block_fs->free_size           = 0; 
  block_fs->total_cache_size    = 0;
  block_fs_set_filenames( block_fs );
}




static block_fs_type * block_fs_alloc_empty( const char * mount_file ,
                                             int block_size ,
                                             int max_cache_size,
                                             float fragmentation_limit,
                                             int fsync_interval ,
                                             bool read_only,
                                             bool use_lockfile) {
  block_fs_type * block_fs      = util_malloc( sizeof * block_fs );
  UTIL_TYPE_ID_INIT(block_fs , BLOCK_FS_TYPE_ID);
  
  block_fs->mount_file           = util_alloc_string_copy( mount_file );
  block_fs->fsync_interval       = fsync_interval;
  block_fs->block_size           = block_size;
  block_fs->max_cache_size       = max_cache_size;
  block_fs->total_cache_size     = 0;
  block_fs->max_total_cache_size = 512 * 1024 * 1024;  /* 512 MB */
  
  block_fs->fragmentation_limit = fragmentation_limit;   
  util_alloc_file_components( mount_file , &block_fs->path , &block_fs->base_name, NULL );
  pthread_mutex_init( &block_fs->io_lock  , NULL);
  pthread_rwlock_init( &block_fs->rw_lock , NULL);
  {
    FILE * stream            = util_fopen( mount_file , "r");
    int id                   = util_fread_int( stream );
    block_fs->version        = util_fread_int( stream );
    fclose( stream );
    
    if (id != MOUNT_MAP_MAGIC_INT) 
      util_abort("%s: The file:%s does not seem to be a valid block_fs mount map \n",__func__ , mount_file);
  }
  block_fs->data_file   = NULL;
  block_fs->lock_file   = NULL;
  block_fs->index_file  = NULL;
  block_fs_reinit( block_fs );


  {
    bool lock_aquired = true;

    if (use_lockfile) {
      lock_aquired = util_try_lockf( block_fs->lock_file , S_IWUSR + S_IWGRP , &block_fs->lock_fd);
    
      if (!lock_aquired) 
        fprintf(stderr," Another program has already opened filesystem read-write - this instance will be UNSYNCRONIZED read-only. Cross your fingers ....\n");
    }

    if (lock_aquired && (read_only == false))
      block_fs->data_owner = true; 
    else
      block_fs->data_owner = false;

  }
  return block_fs;
}
  

UTIL_IS_INSTANCE_FUNCTION(block_fs , BLOCK_FS_TYPE_ID);


static void block_fs_fwrite_mount_info__( const char * mount_file , int version) {
  FILE * stream = util_fopen( mount_file , "w");
  util_fwrite_int( MOUNT_MAP_MAGIC_INT , stream );
  util_fwrite_int( version , stream );
  fclose( stream );
 }


/**
   Will seek the datafile to the end of the current file_node. So that the next read will be "guaranteed" to
   start at a new node.
*/
static void block_fs_fseek_node_end(block_fs_type * block_fs , const file_node_type * file_node) {
  block_fs_fseek( block_fs , file_node->node_offset + file_node->node_size);
}

static void block_fs_fseek_node_data(block_fs_type * block_fs , const file_node_type * file_node) {
  block_fs_fseek( block_fs , file_node->node_offset + file_node->data_offset );
}




/**
   This function will read through the datafile seeking for one of the
   identifiers: NODE_IN_USE | NODE_FREE. If one of the valid status
   identifiers is found the stream is repositioned at the beginning of
   the valid node, so the calling scope can continue with a
   
      file_node = file_node_date_fread_alloc()

   call. If no valid status ID is found whatsover the data_stream
   indicator is left at the end of the file; and the calling scope
   will finish from there.
*/
static bool block_fs_fseek_valid_node( block_fs_type * block_fs ) {
  unsigned char byte;
  int  status;
  while (true) {
    if (fread(&byte , sizeof byte ,1 , block_fs->data_stream) == 1) {
      if (byte == NODE_IN_USE_BYTE || byte == NODE_FREE_BYTE) {
        long int pos = ftell( block_fs->data_stream );
        /* 
           OK - we found one interesting byte; let us try to read the
           whole integer and see if we have hit any of the valid status identifiers.
        */
        fseek__( block_fs->data_stream , -1 , SEEK_CUR);
        if (fread(&status , sizeof status , 1 , block_fs->data_stream) == 1) {
          if (status == NODE_IN_USE || status == NODE_FREE_BYTE) {
            /* 
               OK - we have found a valid identifier. We reposition to
               the start of this valid status id and return true.
            */
            fseek__( block_fs->data_stream , -sizeof status , SEEK_CUR);
            return true;
          } else 
            /* 
               OK - this was not a valid id; we go back and continue 
               reading single bytes.
            */
            block_fs_fseek( block_fs , pos );
        } else
          break; /* EOF */
      }
    } else
      break; /* EOF */
  }
  fseek__( block_fs->data_stream , 0 , SEEK_END);
  return false;
}





/**
   The read-only open mode is only for the mount section, where the
   data file is read in to load/verify the index.

   If the read_only open fails - the data_stream is set to NULL. If
   the open succeeds the calling scope should close the stream before
   calling this function again, with read_only == false.
*/

static void block_fs_open_data( block_fs_type * block_fs , bool read_write) {
  if (read_write) {
    /* Normal read-write open.- */
    if (util_file_exists( block_fs->data_file ))
      block_fs->data_stream = util_fopen( block_fs->data_file , "r+");
    else
      block_fs->data_stream = util_fopen( block_fs->data_file , "w+");
  } else {
    /* read-only open. */
    if (util_file_exists( block_fs->data_file ))
      block_fs->data_stream = util_fopen( block_fs->data_file , "r");
    else
      block_fs->data_stream = NULL;
    /* 
       If we ever try to dereference this pointer it will break
       hard; but it should be stopped in hash_get() calls before the
       data_stream is dereferenced anyway?
    */
  }
  if (block_fs->data_stream == NULL)
    block_fs->data_fd = -1;
  else
    block_fs->data_fd = fileno( block_fs->data_stream );
}

#ifdef ENABLE_CACHE

static void block_fs_clear_cache_node( block_fs_type * block_fs , file_node_type * node ) {
  if (node->cache_size > 0) {
    block_fs->total_cache_size -= node->cache_size;
    file_node_clear_cache( node );
  }
}

static void block_fs_update_cache_node( block_fs_type * block_fs, file_node_type * node, int data_size, const void * data) {
  int delta_size = data_size - node->cache_size;
  if (delta_size < 0) {
    file_node_update_cache( node , data_size , data );
    block_fs->total_cache_size += delta_size;
  } else {
    if (((block_fs->total_cache_size + delta_size) <= block_fs->max_total_cache_size) &&   /* Check total cache usage */
        (data_size <= block_fs->max_cache_size)) {                                         /* Chech cache size of this node */
      block_fs->total_cache_size += delta_size;
      file_node_update_cache( node , data_size , data );
    } else
      block_fs_clear_cache_node( block_fs , node );
  }
}

/**
   This function will load all the small (i.e. with size less than the
   maximum cache size) nodes, and fill the cache.
*/

static void block_fs_preload( block_fs_type * block_fs ) {
  if ((block_fs->max_cache_size > 0) && (block_fs->data_stream != NULL) && (block_fs->max_total_cache_size > 0)) {
    void * buffer = util_malloc( block_fs->max_cache_size );
    hash_iter_type * index_iter = hash_iter_alloc( block_fs->index );
    
    while (!hash_iter_is_complete( index_iter )) {
      file_node_type * node = hash_iter_get_next_value( index_iter );
      if ((node->data_size < block_fs->max_cache_size) &&                                         /* Check the size of this node */ 
          (block_fs->total_cache_size + node->data_size < block_fs->max_total_cache_size)) {      /* Check the total cache size */
        block_fs_fseek_node_data(block_fs , node);
        util_fread( buffer , 1 , node->data_size , block_fs->data_stream , __func__);
        block_fs_update_cache_node( block_fs , node , node->data_size , buffer );
      }
    }
    
    hash_iter_free( index_iter );
    free( buffer );
  }
}
#else
static void block_fs_clear_cache_node( block_fs_type * block_fs , file_node_type * node ) { return; }
static void block_fs_update_cache_node( block_fs_type * block_fs, file_node_type * node, int data_size, const void * data) { return ;}
static void block_fs_preload( block_fs_type * block_fs ) { return; }
#endif



/**
   This function will 'fix' the nodes with offset in offset_list.  The
   fixing in this case means the following:

     1. The node is updated in place on the file to become a free node.
     2. The node is added to the block_fs instance as a free node, which can
        be recycled at a later stage.
   
   If the instance is not data owner (i.e. read-only) the function
   will return immediately.
*/


static void block_fs_fix_nodes( block_fs_type * block_fs , long_vector_type * offset_list ) {
  if (block_fs->data_owner) { 
    fsync( block_fs->data_fd );
    {
      char * key = NULL;
      for (int inode = 0; inode < long_vector_size( offset_list ); inode++) {
        bool new_node = false;
        long int node_offset = long_vector_iget( offset_list , inode );
        file_node_type * file_node;
        block_fs_fseek(block_fs , node_offset);
        file_node = file_node_fread_alloc( block_fs->data_stream , &key );
        
        if ((file_node->status == NODE_INVALID) || (file_node->status == NODE_WRITE_ACTIVE)) {
          /* This node is really quite broken. */
          long int node_end;
          block_fs_fseek_valid_node( block_fs );
          node_end             = ftell( block_fs->data_stream );
          file_node->node_size = node_end - node_offset;
        } 
        
        file_node->status      = NODE_FREE;
        file_node->data_size   = 0;
        file_node->data_offset = 0;
        if (block_fs_lookup_free_node( block_fs , node_offset) == NULL) {
          /* The node is already on the free list - we just change some metadata. */
          new_node = true;
          block_fs_install_node( block_fs , file_node );
          block_fs_insert_free_node( block_fs , file_node );
        }
        
        block_fs_fseek(block_fs , node_offset);
        file_node_fwrite( file_node , NULL , block_fs->data_stream );
        if (!new_node)
          file_node_free( file_node );
      }
      util_safe_free( key );
    }
    fsync( block_fs->data_fd );
  }
}



static void block_fs_build_index( block_fs_type * block_fs , long_vector_type * error_offset ) {
  char * filename = NULL;
  file_node_type * file_node;
  
  hash_resize( block_fs->index , DEFAULT_INDEX_SIZE );
  block_fs_fseek( block_fs , 0);
  do {
    file_node = file_node_fread_alloc( block_fs->data_stream , &filename );
    if (file_node != NULL) {
      if ((file_node->status == NODE_INVALID) || (file_node->status == NODE_WRITE_ACTIVE)) {
        /* Oh fuck */
        if (file_node->status == NODE_INVALID) 
          fprintf(stderr,"** Warning:: invalid node found at offset:%ld in datafile:%s - data will be lost, node_size:%d\n", file_node->node_offset , block_fs->data_file , file_node->node_size);
        else
          fprintf(stderr,"** Warning:: file system was prematurely shut down while writing node in %s/%ld - will be discarded.\n",block_fs->data_file , file_node->node_offset);
        
        long_vector_append( error_offset , file_node->node_offset );
        file_node_free( file_node );
        block_fs_fseek_valid_node( block_fs );
      } else {
        if (file_node_verify_end_tag(file_node, block_fs->data_stream)) {
          block_fs_fseek_node_end(block_fs , file_node);
          block_fs_install_node( block_fs , file_node );
          switch(file_node->status) {
          case(NODE_IN_USE):
            block_fs_insert_index_node(block_fs , filename , file_node);
            break;
          case(NODE_FREE):
            block_fs_insert_free_node( block_fs , file_node );
            break;
          default:
            util_abort("%s: node status flag:%d not recognized - error in data file \n",__func__ , file_node->status);
          }
        } else {
          /* 
             Could not find a valid END_TAG - indicating that
             the filesystem was shut down during the write of
             this node.  This node will NOT be added to the
             index.  The node will be updated to become a free node.
          */
          fprintf(stderr,"** Warning found node:%s at offset:%ld which was incomplete - discarded.\n",filename, file_node->node_offset);
          long_vector_append( error_offset , file_node->node_offset );
          file_node_free( file_node );
          block_fs_fseek_valid_node( block_fs );
        }
      }
    }
  } while (file_node != NULL);
  util_safe_free( filename );
}


/**
   Load an index for (slightly) faster mounting of the filesystem. The
   function starts be reading a header and check if the current index
   file is applicable.

   Will return true of the loading succedeed, and false if no index
   was loaded.  
*/


static bool block_fs_load_index( block_fs_type * block_fs ) {
  stat_type data_stat;
  if (fstat( block_fs->data_fd , &data_stat) == 0) {
    FILE * stream = fopen( block_fs->index_file , "r");
    if (stream != NULL) {
      int    id          = util_fread_int( stream );
      int    version     = util_fread_int( stream );
      time_t index_mtime = util_fread_time_t( stream );

      time_t data_mtime  = data_stat.st_mtime;
      fclose( stream );

      if ((id == INDEX_MAGIC_INT) &&               /* This is indeed an index file. */ 
          (version == INDEX_FORMAT_VERSION) &&     /* The version on disk agrees with this version. */
          (index_mtime == data_mtime)) {           /* The time stamp agrees with the time stamp of the data. */
        
        /* Read the whole index file in one single read operation. */
        buffer_type * buffer = buffer_fread_alloc( block_fs->index_file );
        
        buffer_fskip( buffer , sizeof( time_t ) + 2 * sizeof( int ));
        /*1: Loading all the active nodes. */
        {
          int num_active_nodes = buffer_fread_int( buffer );
          hash_resize( block_fs->index , num_active_nodes * 2 + 64);
          
          for (int i=0; i < num_active_nodes; i++) {
            const char * filename = buffer_fread_string( buffer );
            file_node_type * file_node = file_node_index_buffer_fread_alloc( buffer );
            block_fs_install_node( block_fs , file_node);
            block_fs_insert_index_node(block_fs , filename , file_node);
          }
        }
        
        /*2: Loading all the free nodes. */
        {
          int num_free_nodes = buffer_fread_int( buffer );
          for (int i=0; i < num_free_nodes; i++) {
            file_node_type * file_node = file_node_index_buffer_fread_alloc( buffer );
            block_fs_install_node( block_fs , file_node);
            block_fs_insert_free_node(block_fs , file_node);
          }
        }
        buffer_free( buffer );
        
        return true;
      }
    } 
  }
  /** No index was loaded - for whatever reason. */
  return false;
}


size_t block_fs_get_cache_usage( const block_fs_type * block_fs ) {
  return block_fs->total_cache_size;
}


bool block_fs_is_mount( const char * mount_file ) {
  FILE * stream            = util_fopen( mount_file , "r");
  int id                   = util_fread_int( stream );

  if (id == MOUNT_MAP_MAGIC_INT) 
    return true;
  else
    return false;
}


bool block_fs_is_readonly( const block_fs_type * bfs ) {
  if (bfs->data_owner)
    return false;
  else
    return true;
}



block_fs_type * block_fs_mount( const char * mount_file ,
                                int block_size ,
                                int max_cache_size ,
                                float fragmentation_limit ,
                                int fsync_interval ,
                                bool preload ,
                                bool read_only,
                                bool use_lockfile) {
  block_fs_type * block_fs;
  {

    if (!util_file_exists(mount_file)) 
      /* This is a brand new filesystem - create the mount map first. */
      block_fs_fwrite_mount_info__( mount_file , 0 );
    {
      long_vector_type * fix_nodes = long_vector_alloc(0 , 0);
      block_fs = block_fs_alloc_empty( mount_file , block_size , max_cache_size , fragmentation_limit , fsync_interval , read_only, use_lockfile);
      /* We build up the index & free_nodes_list based on the header/index information embedded in the datafile. */
      block_fs_open_data( block_fs , false );
      if (block_fs->data_stream != NULL) {
        if (!block_fs_load_index( block_fs ))
          block_fs_build_index( block_fs , fix_nodes );

        fclose(block_fs->data_stream);
      }
      
      block_fs_open_data( block_fs , block_fs->data_owner ); /* The data_stream is opened for reading AND writing (IFF we are data_owner - otherwise it is still read only) */
      block_fs_fix_nodes( block_fs , fix_nodes );  
      long_vector_free( fix_nodes );
    }
  }
  if (preload) block_fs_preload( block_fs );
  return block_fs;
}



static void block_fs_unlink_free_node( block_fs_type * block_fs , free_node_type * node) {
  free_node_type * prev = node->prev;
  free_node_type * next = node->next;
  
  if (prev == NULL)
    /* Special case: popping off the head of the list. */
    block_fs->free_nodes = next;
  else
    prev->next = next;
  
  if (next != NULL)
    next->prev = prev;

  block_fs->num_free_nodes--;
  block_fs->free_size -= node->file_node->node_size;
  free_node_free( node );
}



/**
   This function first checks the free nodes if any of them can be
   used, otherwise a new node is created.
*/

static file_node_type * block_fs_get_new_node( block_fs_type * block_fs , const char * filename , size_t min_size) {
  
  free_node_type * current = block_fs->free_nodes;
  
  while (current != NULL && (current->file_node->node_size < min_size)) {
    current = current->next;
  }
  if (current != NULL) {
    /* 
       Current points to a file_node which can be used. Before we return current we must:
       
       1. Remove current from the free_nodes list.
       2. Add current to the index hash.
       
    */
    file_node_type * file_node = current->file_node;
    block_fs_unlink_free_node( block_fs , current );

    return file_node;
  } else {
    /* No usable nodes in the free nodes list - must allocate a brand new one. */

    long int offset;
    int node_size;
    file_node_type * new_node;
    
    {
      div_t d   = div( min_size , block_fs->block_size );
      node_size = d.quot * block_fs->block_size;
      if (d.rem)
        node_size += block_fs->block_size;
    }

    /* Must lock the total size here ... */
    offset = block_fs->data_file_size;
    new_node = file_node_alloc(NODE_IN_USE , offset , node_size);  
    block_fs_install_node( block_fs , new_node );                   /* <- This will update the total file size. */
    
    return new_node;
  }
}





bool block_fs_has_file__( const block_fs_type * block_fs , const char * filename) {
  return hash_has_key( block_fs->index , filename );
}



bool block_fs_has_file( block_fs_type * block_fs , const char * filename) {
  bool has_file;
  block_fs_aquire_rlock( block_fs );
  {
    has_file = block_fs_has_file__( block_fs , filename );
  }
  block_fs_release_rwlock( block_fs );
  return has_file;
}




static void block_fs_unlink_file__( block_fs_type * block_fs , const char * filename ) {
  file_node_type * node = hash_pop( block_fs->index , filename );
  block_fs_clear_cache_node( block_fs , node );

  node->status      = NODE_FREE;
  node->data_offset = 0;
  node->data_size   = 0;
  if (block_fs->data_stream != NULL) {  
    fsync( block_fs->data_fd );
    block_fs_fseek(block_fs , node->node_offset);
    file_node_fwrite( node , NULL , block_fs->data_stream );
    fsync( block_fs->data_fd );
  }
  block_fs_insert_free_node( block_fs , node );
}

/**
   Returns the fraction of unused space in the block_fs instance. 
*/
double block_fs_get_fragmentation( const block_fs_type * block_fs ) {
  return block_fs->free_size * 1.0 / block_fs->data_file_size;
}


void block_fs_unlink_file( block_fs_type * block_fs , const char * filename) {
  block_fs_aquire_wlock( block_fs );

  block_fs_unlink_file__( block_fs , filename );
  if (block_fs_get_fragmentation( block_fs ) > block_fs->fragmentation_limit) 
    block_fs_rotate__( block_fs );
  
  block_fs_release_rwlock( block_fs );
}


/**
   This function can be used to initiate explicit rotate of the file
   system, observe the following.

   1. This function is called with an explicit fragmentation_limit
   which might differ from the fragmentation limit held by the
   filesystem

   2. This function will take the write lock, it is therefor
   essential that this function is NOT called by another function
   which has already taken the write lock (i.e. the
   block_fs_unlink_file() or block_fs_fwrite_file() functions),
   that will deadlock.

   The return value is whether a rotation actually has taken place.
*/

bool block_fs_rotate( block_fs_type * block_fs , double fragmentation_limit) {
  if (block_fs_get_fragmentation( block_fs ) > fragmentation_limit) {
    block_fs_aquire_wlock( block_fs );
    block_fs_rotate__( block_fs );
    block_fs_release_rwlock( block_fs );
    return true;
  } else
    return false;
}

    
/* 
   It seems it is not enough to call fsync(); must also issue this
   funny fseek + ftell combination to ensure that all data is on
   disk after an uncontrolled shutdown.

   Could possibly use fdatasync() to improve speed slightly?
*/

void block_fs_fsync( block_fs_type * block_fs ) {
  if (block_fs->data_owner) {
    //fdatasync( block_fs->data_fd );
    fsync( block_fs->data_fd );
    block_fs_fseek( block_fs , block_fs->data_file_size );
    ftell( block_fs->data_stream );
  }
}




/**
   The single lowest-level write function:
   
   3. seek to correct position.
   4. Write the data with util_fwrite()

   7. increase the write_count
   8. set the data_size field of the node.

   Observe that when 'designing' this file-system the priority has
   been on read-spead, one consequence of this is that all write
   operations are sandwiched between two fsync() calls; that
   guarantees that the read access (which should be the fast path) can
   be without any calls to fsync().

   Not necessary to lock - since all writes are protected by the
   'global' rwlock anyway.
*/


static void block_fs_fwrite__(block_fs_type * block_fs , const char * filename , file_node_type * node , const void * ptr , int data_size) {

#ifdef ENABLE_CACHE
  if ((node->cache_size == data_size) && (memcmp( ptr , node->cache , data_size ) == 0)) 
    /* The current cache is identical to the data we are attempting to write - can leave immediately. */
    return;
#else 
  if (false) 
    return;
#endif

  else {
    block_fs_fseek(block_fs , node->node_offset);
    node->status      = NODE_IN_USE;
    node->data_size   = data_size; 
    file_node_set_data_offset( node , filename );
    
    /* This marks the node section in the datafile as write in progress with: NODE_WRITE_ACTIVE_START ... NODE_WRITE_ACTIVE_END */
    file_node_init_fwrite( node , block_fs->data_stream );                
    
    /* Writes the actual data content. */
    block_fs_fseek_node_data(block_fs , node);
    util_fwrite( ptr , 1 , data_size , block_fs->data_stream , __func__);
    
    /* Writes the file node header data, including the NODE_END_TAG. */
    file_node_fwrite( node , filename , block_fs->data_stream );

    block_fs_update_cache_node( block_fs , node , data_size , ptr);
    block_fs->write_count++;
    if (block_fs->fsync_interval && ((block_fs->write_count % block_fs->fsync_interval) == 0)) 
      block_fs_fsync( block_fs );
    
  }
}



static void block_fs_fwrite_file_unlocked(block_fs_type * block_fs , const char * filename , const void * ptr , size_t data_size) {
  file_node_type * file_node;
  bool   new_node = true;   
  size_t min_size = data_size + file_node_header_size( filename );
  
  if (block_fs_has_file__( block_fs , filename )) {
    file_node = hash_get( block_fs->index , filename );
    if (file_node->node_size < min_size) {
      /* 
         The current node is too small for the new content:
         
         1. Remove the existing node, from the index and insert it
         into the free_nodes list.

         2. Get a new node.
        
      */
      block_fs_unlink_file__( block_fs , filename );
      file_node = block_fs_get_new_node( block_fs , filename , min_size );
    } else
      new_node = false;  /* We are reusing the existing node. */
  } else 
    file_node = block_fs_get_new_node( block_fs , filename , min_size );
  
  
  /* The actual writing ... */
  block_fs_fwrite__( block_fs , filename , file_node , ptr , data_size);
  if (new_node)
    block_fs_insert_index_node(block_fs , filename , file_node);
}



void block_fs_fwrite_file(block_fs_type * block_fs , const char * filename , const void * ptr , size_t data_size) {
  block_fs_aquire_wlock( block_fs );
  {
    block_fs_fwrite_file_unlocked( block_fs , filename , ptr , data_size );
    
    /* OKAY - this is going to take some time ... */
    if ((block_fs->free_size * 1.0 / block_fs->data_file_size) > block_fs->fragmentation_limit)
      block_fs_rotate__( block_fs );

  }
  block_fs_release_rwlock( block_fs );
}


void block_fs_defrag( block_fs_type * block_fs ) {
  block_fs_aquire_wlock( block_fs );
  block_fs_rotate__( block_fs );
  block_fs_release_rwlock( block_fs );
}


void block_fs_fwrite_buffer(block_fs_type * block_fs , const char * filename , const buffer_type * buffer) {
  block_fs_fwrite_file( block_fs , filename , buffer_get_data( buffer ) , buffer_get_size( buffer ));
}


/**
   Need extra locking here - because the global rwlock allows many
   concurrent readers.
*/
static void block_fs_fread__(block_fs_type * block_fs , const file_node_type * file_node , void * ptr , size_t read_bytes) {

#ifdef ENABLE_CACHE  
  if (file_node->cache != NULL) 
    file_node_read_from_cache( file_node , ptr , read_bytes);
  else
#else
    if (true) 
#endif

  {
    pthread_mutex_lock( &block_fs->io_lock );
    block_fs_fseek_node_data( block_fs , file_node );
    util_fread( ptr , 1 , read_bytes , block_fs->data_stream , __func__);
    //file_node_verify_end_tag( file_node , block_fs->data_stream );
    pthread_mutex_unlock( &block_fs->io_lock );
  }
}


/**
   Reads the full content of 'filename' into the buffer. 
*/

void block_fs_fread_realloc_buffer( block_fs_type * block_fs , const char * filename , buffer_type * buffer) {
  block_fs_aquire_rlock( block_fs );
  {
    file_node_type * node = hash_get( block_fs->index , filename);
    
    buffer_clear( buffer );   /* Setting: content_size = 0; pos = 0;  */
    {
      /* 
         Going low-level - essentially a second implementation of
         block_fs_fread__():
      */

#ifdef ENABLE_CACHE
      if (node->cache != NULL) 
        file_node_buffer_read_from_cache( node , buffer );
      else 
#else
      if (true)  
#endif

      {
        pthread_mutex_lock( &block_fs->io_lock );
        block_fs_fseek_node_data(block_fs , node );
        buffer_stream_fread( buffer , node->data_size , block_fs->data_stream );
        //file_node_verify_end_tag( node , block_fs->data_stream );
        pthread_mutex_unlock( &block_fs->io_lock );
      }
      
    }
    buffer_rewind( buffer );  /* Setting: pos = 0; */
  }
  block_fs_release_rwlock( block_fs );
}







/*
  This function will read all the data stored in 'filename' - it is
  the responsability of the calling scope that ptr is sufficiently
  large to hold it. You can use block_fs_get_filesize() first to
  check.
*/


void block_fs_fread_file( block_fs_type * block_fs , const char * filename , void * ptr) {
  block_fs_aquire_rlock( block_fs );
  {
    file_node_type * node = hash_get( block_fs->index , filename);
    block_fs_fread__( block_fs , node , ptr , node->data_size);
  }
  block_fs_release_rwlock( block_fs );
}



int block_fs_get_filesize( block_fs_type * block_fs , const char * filename) {
  int data_size;
  block_fs_aquire_rlock( block_fs );
  {
    file_node_type * node = hash_get( block_fs->index , filename );
    data_size = node->data_size;
  }
  block_fs_release_rwlock( block_fs );
  return data_size;
}


static void block_fs_dump_index( block_fs_type * block_fs ) {
  if (block_fs->data_owner) {
    struct stat stat_buffer;
    int stat_return = stat(block_fs->data_file , &stat_buffer);
    if (stat_return != 0)
      return;
    {
      time_t data_mtime = stat_buffer.st_mtime;
      FILE * index_stream = util_fopen( block_fs->index_file , "w");
      util_fwrite_int( INDEX_MAGIC_INT , index_stream );
      util_fwrite_int( INDEX_FORMAT_VERSION , index_stream );
      util_fwrite_time_t( data_mtime , index_stream );

      /* 1: Dumping the hash table of active nodes. */
      {
        hash_iter_type * index_iter = hash_iter_alloc( block_fs->index );
        
        util_fwrite_int( hash_get_size( block_fs->index ) , index_stream);
        while (!hash_iter_is_complete( index_iter )) {
          const char * key = hash_iter_get_next_key( index_iter );
          const file_node_type * file_node = hash_get( block_fs->index , key );
          
          util_fwrite_string( key , index_stream);
          file_node_dump_index( file_node , index_stream );
        }
        hash_iter_free( index_iter );
      }
      
      /* 2: Dumping information about empty slots in the datafile. */
      util_fwrite_int( block_fs->num_free_nodes , index_stream );
      {
        free_node_type * current = block_fs->free_nodes;
        while ( current != NULL) {
          file_node_dump_index( current->file_node , index_stream );
          current = current->next;
        }
      }
      
      fclose( index_stream );
    }
  }
}


/**
   Close/synchronize the open file descriptors and free all memory
   related to the block_fs instance.

   If the boolean unlink_empty is set to true all the files will be
   unlinked if the filesystem is empty.
*/

void block_fs_close( block_fs_type * block_fs , bool unlink_empty) {
  block_fs_fsync( block_fs );
  
  if (block_fs->data_owner) 
    block_fs_aquire_wlock( block_fs );

  if (block_fs->data_stream != NULL) 
    fclose( block_fs->data_stream );

  if (block_fs->data_owner) 
    block_fs_dump_index( block_fs );
      
  if (block_fs->lock_fd > 0) {
    close( block_fs->lock_fd );     /* Closing the lock_file file descriptor - and releasing the lock. */
    util_unlink_existing( block_fs->lock_file );
  }

  if (block_fs->data_owner) {
    if ( unlink_empty && (hash_get_size( block_fs->index) == 0)) {
      util_unlink_existing( block_fs->data_file );
      util_unlink_existing( block_fs->index_file );
      util_unlink_existing( block_fs->mount_file );
    }
    block_fs_release_rwlock( block_fs );
  }

  free( block_fs->index_file );
  free( block_fs->lock_file );
  free( block_fs->base_name );
  free( block_fs->data_file );
  free( block_fs->path );
  free( block_fs->mount_file );
  
  free_node_free_list( block_fs->free_nodes );
  hash_free( block_fs->index );
  vector_free( block_fs->file_nodes );
  free( block_fs );
}





/**
   This function will 'rotate' the datafile to a new version which has
   been defragmented, i.e. with no 'holes' in it. In the process the
   datafile version number is increased with one. The function works
   by using the regular block_fs read and write functions.
   
   Observe that the block_fs instance should hold the write lock when
   entering this function.
*/

static void block_fs_rotate__( block_fs_type * block_fs ) {
  /* 
     Write a updated mount map where the version info has been bumped
     up with one; the new_fs will mount based on this mount_file.
  */
  block_fs->version++;
  block_fs_fwrite_mount_info__( block_fs->mount_file , block_fs->version ); 
  {
    vector_type    * old_nodes         = block_fs->file_nodes;
    hash_type      * old_index         = block_fs->index;
    FILE           * old_data_stream   = block_fs->data_stream;
    free_node_type * old_free_nodes    = block_fs->free_nodes;
    char           * old_data_file     = util_alloc_string_copy( block_fs->data_file );
    char           * old_lock_file     = util_alloc_string_copy( block_fs->lock_file );

    block_fs_reinit( block_fs );
    /** 
        Now the block_fs pointers point to the new copy. Must use the
        old_xxx pointers to access the existing.
    */
    block_fs_open_data( block_fs , block_fs->data_owner );
    {
      hash_iter_type * iter = hash_iter_alloc( old_index );
      buffer_type * buffer  = buffer_alloc(1024);
      
      while (!hash_iter_is_complete( iter )) {
        const char * key          = hash_iter_get_next_key( iter );
        file_node_type * old_node = hash_get( old_index , key );
        buffer_clear( buffer );

        /* Low level read of the old file. */
        fseek__( old_data_stream , old_node->node_offset + old_node->data_offset , SEEK_SET );
        buffer_stream_fread( buffer , old_node->data_size , old_data_stream );
        
        block_fs_fwrite_file_unlocked( block_fs , key , buffer_get_data( buffer ) , buffer_get_size( buffer ));  /* Normal write to the new file. */
      }
      
      buffer_free( buffer );
      hash_iter_free( iter );
    }
    /*
      OK - everything has been played over, and we should clean up the old fs:

        1. Close the old data stream.
        2. Unlink the old lockfile.
        3. Delete the old data file.
        4. Delete the old list of free nodes.
        5. free()

    */
    fclose( old_data_stream );
    unlink( old_data_file );
    unlink( old_lock_file );
    free( old_lock_file );
    free( old_data_file );
    
    free_node_free_list( old_free_nodes );
    hash_free( old_index );
    vector_free( old_nodes );
  }
}


/*****************************************************************/
/* Functions related to 'ls' like functionality.                 */
/*****************************************************************/


/* 
   Help structure used for 'ls' like funtionality. 
*/

struct user_file_node_struct {
  const file_node_type  * file_node;
  char                  * filename; 
};


static user_file_node_type * user_file_node_alloc( const char * name , const file_node_type * file_node) {
  user_file_node_type * user_node = util_malloc( sizeof * user_node );

  user_node->filename = util_alloc_string_copy( name );   /* name can be NULL */
  user_node->file_node = file_node;
  
  return user_node;
}


static void user_file_node_free( user_file_node_type * node ) {
  util_safe_free( node->filename );
  free( node );
}

static void user_file_node_free__( void * node ) {
  user_file_node_free( (user_file_node_type *) node );
}

long int user_file_node_get_node_offset( const user_file_node_type * user_file_node ) {
  return user_file_node->file_node->node_offset;
}

long int user_file_node_get_data_offset( const user_file_node_type * user_file_node ) {
  return user_file_node->file_node->data_offset + user_file_node->file_node->node_offset;
}

int user_file_node_get_node_size( const user_file_node_type * user_file_node ) {
  return user_file_node->file_node->node_size;
}

int user_file_node_get_data_size( const user_file_node_type * user_file_node ) {
  return user_file_node->file_node->data_size;
}

bool user_file_node_in_use( const user_file_node_type * user_file_node ) {
  if (user_file_node->file_node->status == NODE_IN_USE)
    return true;
  else
    return false;
}

const char * user_file_node_get_filename( const user_file_node_type * user_file_node ) {
  return user_file_node->filename;
}

static int offset_cmp( const void * arg1 , const void * arg2 ) {
  const user_file_node_type * node1 = (user_file_node_type *) arg1;
  const user_file_node_type * node2 = (user_file_node_type *) arg2;
  
  if (node1->file_node->node_offset > node2->file_node->node_offset)
    return 1;
  else
    return -1;
}


static int string_cmp( const void * arg1 , const void * arg2 ) {
  const user_file_node_type * node1 = (user_file_node_type *) arg1;
  const user_file_node_type * node2 = (user_file_node_type *) arg2;
  
  if (node1->filename == NULL)
    return 1;
  else if (node2->filename == NULL)
    return -1;
  else
    return strcmp( node1->filename , node2->filename );
}


static bool pattern_match( const char * pattern , const char * string ) {
  if (pattern == NULL)
    return true;
  else {
    if (fnmatch( pattern , string , 0 ) == 0)
      return true;
    else
      return false;
  }
}

/**
   If pattern == NULL all files will be selected. Observe that the
   returned vector contains pointers to the "real" file_node instances
   - this has two consequences:

    1. The calling scope should N O T change the elements in the
       vector - that will lead to internal corruption of the block_fs
       instance.

    2. If normal read/write/unlink operations are performed on the
       block_fs instance while the vector is held, the content of the
       vector will go out of sync.

*/

vector_type * block_fs_alloc_filelist( block_fs_type * block_fs  , const char * pattern , block_fs_sort_type sort_mode , bool include_free_nodes ) {
  vector_type    * sort_vector = vector_alloc_new();

  /* Inserting the nodes from the index. */
  block_fs_aquire_rlock( block_fs );
  {
    hash_iter_type * iter        = hash_iter_alloc( block_fs->index );
    while ( !hash_iter_is_complete( iter )) {
      const char * key            = hash_iter_get_next_key( iter );
      file_node_type * node = hash_get( block_fs->index , key );
      if (pattern_match( pattern , key )) {
        user_file_node_type * unode = user_file_node_alloc( key , node );
        vector_append_owned_ref( sort_vector , unode , user_file_node_free__ );
      }
    }
    hash_iter_free( iter );
  }
  block_fs_release_rwlock( block_fs );

  if (pattern != NULL)
    include_free_nodes = false;  /* Doing fnmatch on free nodes makes no sense */
  
  /* Inserting the free nodes - the holes. */
  if (include_free_nodes) {
    free_node_type * current = block_fs->free_nodes;
    while (current != NULL) {
      user_file_node_type * unode = user_file_node_alloc( NULL , current->file_node );
      vector_append_owned_ref( sort_vector , unode , user_file_node_free__ );
      current = current->next;
    }
  }

  switch( sort_mode ) {
  case(STRING_SORT):
    vector_sort(sort_vector , string_cmp);
    break;
  case(OFFSET_SORT):
    vector_sort(sort_vector , offset_cmp);
    break;
  case(NO_SORT):
    break;
  }

  return sort_vector;
}

