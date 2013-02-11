/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ens_tar.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <util.h>
#include <string.h>
#include <errno.h>
#include <msg.h>

bool tar_and_remove_dir( const char * root_path , const char * current_dir , int current_depth , void * arg) {
  if (strncmp(current_dir , "mem" , 3) == 0) {
    msg_type * msg = msg_safe_cast( arg );
    {
      char * tar_file    = util_alloc_filename(NULL , current_dir , "tar.gz");
      char * target_file = util_alloc_filename(root_path , current_dir , "tar.gz");
      char * text        = util_alloc_sprintf("%s: Creating %s" , root_path , tar_file );
      msg_update( msg , text );
      util_fork_exec("tar" , 3 , (const char *[3]) {"-czf" , tar_file , current_dir} , true , target_file , root_path , NULL , NULL , NULL);
      free( text );
      free( tar_file );
      free( target_file );
    }
    
    
    {
      char * full_path = util_alloc_filename( root_path , current_dir , NULL);
      char * text     = util_alloc_sprintf("%s: Removing %s" , root_path , current_dir);
      msg_update( msg , text );
      util_clear_directory( full_path , true , true );
      free( text );
      free( full_path );
    }
  }
  return true;
}





int main( int argc , char ** argv ) {
  const char * mount_file = "enkf_mount_info";
  if (util_file_exists( mount_file )) {
    msg_type * msg = msg_alloc("" , false);
    char * cwd    = util_alloc_cwd();
    
    msg_show( msg );

    util_walk_directory( cwd , NULL , NULL , tar_and_remove_dir , msg);
    msg_free( msg , true );
    free( cwd );
  } else 
    fprintf(stderr,"Hmmm - could not find file: \'%s\' - this does not seem to be a ENSPATH directory.\n",mount_file);
}




/*
/d/proj/bg/enkf/abuf/Gullfaks/Gullfaks_sector_model/Experiments_Real_Ens_100/cgg_seismic_prod_local_start_2_new/Ensemble_old/cgg_4d_prod_local_start_2_new

 */



