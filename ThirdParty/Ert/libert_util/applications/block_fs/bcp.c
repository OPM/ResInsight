/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'bcp.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


static void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the program with SIGTERM (the default kill signal) you will get a backtrace. Killing with SIGKILL (-9) will not give a backtrace.*/
}


static void usage() {
  printf("Usage: \n\n");
  printf("bcp src.mnt   target.mnt  file \n");
  exit(1);
}



int main(int argc , char ** argv) {
  install_SIGNALS();
  if (argc < 4)
    usage();
  {
    const char * src_mount      = argv[1];
    const char * target_mount   = argv[2];
    if (block_fs_is_mount(src_mount)) {
      const char * pattern  = NULL;
      int iarg;
      
      for (iarg = 3; iarg < argc; iarg++) {
        if (argv[iarg][0] == '-') {
          /** OK - this is an option .. */
        }
        else pattern = argv[iarg];
      }
      
      {
        block_fs_type * src_fs    = block_fs_mount(src_mount , 1 , 0 , 1 , 0 , false , true );
        block_fs_type * target_fs = block_fs_mount(target_mount , 1 , 0 , 1 , 0 , false , false );
        vector_type   * files     = block_fs_alloc_filelist( src_fs , pattern , NO_SORT , false );
        buffer_type   * buffer    = buffer_alloc( 1024 );
        {
          int i;
          msg_type   * msg      = msg_alloc("Copying :" , false);
          msg_show( msg );
          for (i=0; i < vector_get_size( files ); i++) {
            const user_file_node_type * node = vector_iget_const( files , i );
            const char * filename       = user_file_node_get_filename( node );
            msg_update( msg , filename ); 
            block_fs_fread_realloc_buffer( src_fs , filename , buffer );
            block_fs_fwrite_buffer( target_fs , filename , buffer );
          }
          msg_free( msg , true );
        }
        buffer_free( buffer );
        vector_free( files );
        block_fs_close( target_fs , true);
        block_fs_close( src_fs    , false );
      }
    } else 
      fprintf(stderr,"The files:%s/%s does not seem to be a block_fs mount files.\n" , src_mount , target_mount);
  }
}
