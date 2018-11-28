/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'kw_extract.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_endian_flip.h>


/**
   This file will extract all occurences of kw1,kw2,...,kwn from the
   source file and copy them over to the target file. Ordering in the
   target file will be according to the ordering in the source file,
   and not by the ordering given on the command line.
*/

int main(int argc, char ** argv) {
  if (argc < 4) {
    fprintf(stderr,"%s  src_file target_file kw1 kw2 kw3 \n",argv[0]);
    exit(0);
  }
  {
    const char *  src_file   = argv[1];
    const char * target_file = argv[2];
    fortio_type * fortio_src;
    fortio_type * fortio_target;
    bool fmt_src , fmt_target;
    stringlist_type * kw_set = stringlist_alloc_new();


    for (int iarg=3; iarg < argc; iarg++)
      stringlist_append_copy(kw_set, argv[iarg]);

    if (!ecl_util_fmt_file(src_file, &fmt_src))
      util_exit("Hmm - could not determine formatted/unformatted status for:%s \n",src_file);

    fmt_target        = fmt_src;                         /* Can in principle be different */
    fortio_src        = fortio_open_reader(src_file     , fmt_src , ECL_ENDIAN_FLIP);
    fortio_target     = fortio_open_writer(target_file , fmt_target , ECL_ENDIAN_FLIP);

    {
      ecl_kw_type * ecl_kw = ecl_kw_alloc_empty();
      while (true) {
        if (ecl_kw_fread_header( ecl_kw , fortio_src ) == ECL_KW_READ_OK) {
          const char * header = ecl_kw_get_header( ecl_kw );
          if (stringlist_contains( kw_set , header )) {
            ecl_kw_fread_realloc_data(ecl_kw , fortio_src );
            ecl_kw_fwrite( ecl_kw , fortio_target );
          } else
            ecl_kw_fskip_data( ecl_kw , fortio_src );
        } else
          break; /* We have reached EOF */
      }
      ecl_kw_free( ecl_kw );
    }

    fortio_fclose(fortio_src);
    fortio_fclose(fortio_target);
    stringlist_free( kw_set );
  }
}
