/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'brm.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace. Killing with SIGKILL (-9) will not give a backtrace.*/
}


int main(int argc , char ** argv) {
  install_SIGNALS();
  const char * mount_file      = argv[1];
  if (block_fs_is_mount(mount_file)) {
    block_fs_sort_type sort_mode = OFFSET_SORT;
    const char * pattern         = NULL;
    int iarg;

    for (iarg = 2; iarg < argc; iarg++) {
      if (argv[iarg][0] == '-') {
        /** OK - this is an option .. */
      }
      else pattern = argv[iarg];
    }
    
    {
      block_fs_type * block_fs = block_fs_mount(mount_file , 1 , 0 , 1 , 0 , false , false );
      vector_type   * files    = block_fs_alloc_filelist( block_fs , pattern , sort_mode , false );
      {
        int i;
        msg_type * msg = msg_alloc("Deleting file: " , false);
        msg_show( msg );
        //for (i=0; i < vector_get_size( files ); i++) {
        //  const user_file_node_type * node = vector_iget_const( files , i );
        //  printf("%-40s   %10d %ld    \n",user_file_node_get_filename( node ), user_file_node_get_data_size( node ) , user_file_node_get_node_offset( node ));
        //}

        for (i=0; i < vector_get_size( files ); i++) {
          const user_file_node_type * node = vector_iget_const( files , i );
          msg_update( msg , user_file_node_get_filename( node ) );
          block_fs_unlink_file( block_fs , user_file_node_get_filename( node ));
        }
        msg_free( msg , true );
        printf("Final fragmentation: %5.2f \n", block_fs_get_fragmentation( block_fs ));
      }
      vector_free( files );
      block_fs_close( block_fs , false );
    }
  } else 
    fprintf(stderr,"The file:%s does not seem to be a block_fs mount file.\n" , mount_file);
}
