/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_pack.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_type.h>



int main(int argc, char ** argv) {
  int num_files = argc - 1;
  if (num_files >= 1) {
    /* File type and formatted / unformatted is determined from the first argument on the command line. */
    char * ecl_base;
    char * path;
    ecl_file_enum file_type , target_type;
    bool fmt_file;

    /** Look at the first command line argument to determine type and formatted/unformatted status. */
    file_type = ecl_util_get_file_type( argv[1] , &fmt_file , NULL);
    if (file_type == ECL_SUMMARY_FILE)
      target_type = ECL_UNIFIED_SUMMARY_FILE;
    else if (file_type == ECL_RESTART_FILE)
      target_type = ECL_UNIFIED_RESTART_FILE;
    else {
      util_exit("The ecl_pack program can only be used with ECLIPSE restart files or summary files.\n");
      target_type = -1;
    }
    util_alloc_file_components( argv[1] , &path , &ecl_base , NULL);


    /**
       Will pack to cwd, even though the source files might be
       somewhere else. To unpack to the same directory as the source
       files, just send in @path as first argument when creating the
       target_file.
    */

    {
      int i , report_step , prev_report_step;
      char *  target_file_name   = ecl_util_alloc_filename( NULL , ecl_base , target_type , fmt_file , -1);
      stringlist_type * filelist = stringlist_alloc_argv_copy( (const char **) &argv[1] , num_files );
      ecl_kw_type * seqnum_kw    = NULL;
      fortio_type * target       = fortio_open_writer( target_file_name , fmt_file , ECL_ENDIAN_FLIP);

      if (target_type == ECL_UNIFIED_RESTART_FILE) {
        int dummy;
        seqnum_kw = ecl_kw_alloc_new("SEQNUM" , 1 , ECL_INT , &dummy);
      }

      stringlist_sort( filelist , ecl_util_fname_report_cmp);
      prev_report_step = -1;
      for (i=0; i < num_files; i++) {
        ecl_file_enum this_file_type;
        this_file_type = ecl_util_get_file_type( stringlist_iget(filelist , i)  , NULL , &report_step);
        if (this_file_type == file_type) {
          if (report_step == prev_report_step)
            util_exit("Tried to write same report step twice: %s / %s \n",
                      stringlist_iget(filelist , i-1) ,
                      stringlist_iget(filelist , i));

          prev_report_step = report_step;
          {
            ecl_file_type * src_file = ecl_file_open( stringlist_iget( filelist , i) , 0 );
            if (target_type == ECL_UNIFIED_RESTART_FILE) {
              /* Must insert the SEQNUM keyword first. */
              ecl_kw_iset_int(seqnum_kw , 0 , report_step);
              ecl_kw_fwrite( seqnum_kw , target );
            }
            ecl_file_fwrite_fortio( src_file , target , 0);
            ecl_file_close( src_file );
          }
        }  /* Else skipping file of incorrect type. */
      }
      fortio_fclose( target );
      free(target_file_name);
      stringlist_free( filelist );
      if (seqnum_kw != NULL) ecl_kw_free(seqnum_kw);
    }
    free(ecl_base);
    free(path);
  }
}

