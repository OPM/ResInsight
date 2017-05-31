/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'brot.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
  const char * mount_file = argv[1];
  if (block_fs_is_mount(mount_file)) {
    block_fs_type * block_fs = block_fs_mount(mount_file , 1 , 0 , 1 , 0 , false , false );
    block_fs_rotate( block_fs , 0.00 ); 
    block_fs_close( block_fs , false );
  }
}
