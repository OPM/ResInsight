/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'bls.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <signal.h>

#include <ert/util/util.h>
#include <ert/util/block_fs.h>
#include <ert/util/vector.h>


void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace. Killing with SIGKILL (-9) will not give a backtrace.*/
}


static int usage( void ) {
  fprintf(stderr,"\n");
  fprintf(stderr,"Usage:\n\n");
  fprintf(stderr,"   bash%% bls BLOCK_FILE.mnt <pattern>\n\n");
  fprintf(stderr,"Will list all elements in BLOCK_FILE matching pattern - remember to quote wildcards.\n");
  exit(1);
}


int main(int argc , char ** argv) {
  install_SIGNALS();
  if (argc == 1)
    usage();
  {
    const char * mount_file      = argv[1];
    if (block_fs_is_mount(mount_file)) {
      block_fs_sort_type sort_mode = OFFSET_SORT;
      const char * pattern         = NULL;
      int iarg;

      for (iarg = 2; iarg < argc; iarg++) {
        if (argv[iarg][0] == '-') {
          /** OK - this is an option .. */
        }
        else
          pattern = argv[iarg];
      }

      {
        block_fs_type * block_fs = block_fs_mount(mount_file , 1 , 0 , 1 , 0 , false , true , false);
        vector_type   * files    = block_fs_alloc_filelist( block_fs , pattern , sort_mode , false );
        {
          int i;
          for (i=0; i < vector_get_size( files ); i++) {
            const user_file_node_type * node = vector_iget_const( files , i );
            printf("%-40s   %10d %ld    \n",user_file_node_get_filename( node ), user_file_node_get_data_size( node ) , user_file_node_get_node_offset( node ));
          }
        }
        vector_free( files );
        block_fs_close( block_fs , false );
      }
    } else
      fprintf(stderr,"The file:%s does not seem to be a block_fs mount file.\n" , mount_file);
  }
}
