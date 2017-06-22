/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'bfs_extract.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <util.h>
#include <block_fs.h>
#include <vector.h>
#include <signal.h>
#include <msg.h>

/*****************************************************************/
/* 
   This program is used to extract individual files from a block_fs
   file.
*/
   



void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace. Killing with SIGKILL (-9) will not give a backtrace.*/
}


static void usage() {
  fprintf(stderr,"\nThis program is used to extract individual files from a block_fs\n");
  fprintf(stderr,"file. The arguments to the program are:\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"  1. The block_fs mount file.\n");
  fprintf(stderr,"  2. The name of directory (need not exist) where the extracted files will be put.\n");
  fprintf(stderr,"  3. A list of files to extract - this can contain wildcards, but they MUST be \n");
  fprintf(stderr,"     quoted to avoid expansion by the shell.\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"Example:\n\n");
  fprintf(stderr,"  bash%% bfs_extract  block_fs_file.mnt  DOGFolder \'DOG*\'\n\n");
  fprintf(stderr,"This will extract all files starting with \'DOG\' to folder \'DOGFolder\'.\n\n");
  exit(1);
}


int main(int argc , char ** argv) {
  install_SIGNALS();
  if (argc < 3)
    usage();
  {
    
    const char * mount_file      = argv[1];
    if (block_fs_is_mount(mount_file)) {
      const char * target_path     = argv[2];
      int iarg;
      if (!util_is_directory( target_path )) {
        if (util_file_exists( target_path ))
          util_exit("The target:%s already exists - but it is not a directory.\n");
        else
          util_make_path( target_path );
      }
      {
        block_fs_type * block_fs = block_fs_mount(mount_file , 1 , 0 , 1 , 0 , false , true );
        buffer_type * buffer = buffer_alloc(1024);
        msg_type * msg = msg_alloc("Extracting: " , false);
        msg_show( msg );

        for (iarg = 3; iarg < argc; iarg++) {
          vector_type   * files  = block_fs_alloc_filelist( block_fs , argv[iarg] , NO_SORT , false );
          {
            int i;
            for (i=0; i < vector_get_size( files ); i++) {
              const user_file_node_type * node = vector_iget_const( files , i );
              const char * filename    = user_file_node_get_filename( node );
              const char * target_file = util_alloc_filename( target_path , filename , NULL );
              msg_update( msg , filename );
              block_fs_fread_realloc_buffer( block_fs , filename , buffer );
              buffer_store( buffer , target_file );
            }
          }
          vector_free( files );
        }
        block_fs_close( block_fs , false );
        msg_free( msg , true );
      }
    } else 
      fprintf(stderr,"The file:%s does not seem to be a block_fs mount file.\n" , mount_file);
  }
}
