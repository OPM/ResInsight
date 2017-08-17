/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_unpack.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/msg.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_kw_magic.h>

void unpack_file(const char * filename) {
  ecl_file_enum target_type = ECL_OTHER_FILE;
  ecl_file_enum file_type;
  bool fmt_file;
  file_type = ecl_util_get_file_type(filename , &fmt_file , NULL);
  if (file_type == ECL_UNIFIED_SUMMARY_FILE)
    target_type = ECL_SUMMARY_FILE;
  else if (file_type == ECL_UNIFIED_RESTART_FILE)
    target_type = ECL_RESTART_FILE;
  else 
    util_exit("Can only unpack unified ECLIPSE summary and restart files\n");
  
  if (target_type == ECL_SUMMARY_FILE) {
    printf("** Warning: when unpacking unified summary files it as ambigous - starting with 0001  -> \n");
  }
  {
    ecl_file_type * src_file = ecl_file_open( filename , 0 );
    int    size;
    int    offset;
    int    report_step = 0;
    int    block_index = 0;
    char * path; 
    char * base;
    msg_type * msg;
    util_alloc_file_components( filename , &path , &base , NULL);
    {
      char * label  = util_alloc_sprintf("Unpacking %s => ", filename);
      msg = msg_alloc( label , false);
      free( label );
    }
    msg_show(msg);

    if (target_type == ECL_SUMMARY_FILE) 
      size = ecl_file_get_num_named_kw( src_file , "SEQHDR" );
    else
      size = ecl_file_get_num_named_kw( src_file , "SEQNUM" );
    
    
    while (true) {
      ecl_file_view_type * active_view;

      if (block_index == size)
        break;

      if (target_type == ECL_SUMMARY_FILE) {
        active_view = ecl_file_get_global_blockview(src_file, SEQHDR_KW, block_index);
        report_step += 1;
        offset = 0;
      } else {
        ecl_kw_type * seqnum_kw;
        active_view = ecl_file_get_global_blockview(src_file, SEQNUM_KW, block_index);
        seqnum_kw = ecl_file_view_iget_named_kw( active_view , SEQNUM_KW , 0);
        report_step = ecl_kw_iget_int( seqnum_kw , 0);
        offset = 1;
      }

      /**
         Will unpack to cwd, even though the source files might be
         somewhere else. To unpack to the same directory as the source
         files, just send in @path as first argument when creating the
         target_file.
      */
      
      {
        char * target_file = ecl_util_alloc_filename( NULL , base , target_type , fmt_file , report_step);
        fortio_type * fortio_target = fortio_open_writer( target_file , fmt_file , ECL_ENDIAN_FLIP );
        msg_update(msg , target_file);
        ecl_file_view_fwrite( active_view , fortio_target , offset);
        
        fortio_fclose(fortio_target);
        free(target_file);
      }
      block_index++;
    } 
    ecl_file_close( src_file );
    util_safe_free(path);
    free(base);
    msg_free(msg , true);
  }
}


int main(int argc , char ** argv) {
  if (argc == 1)
    util_exit("ecl_unpack UNIFIED_FILE1   UNIFIED_FILE2   ...\n");
  {
    int iarg;
    for (iarg = 1; iarg < argc; iarg++)
      unpack_file( argv[iarg] );
  }
}
