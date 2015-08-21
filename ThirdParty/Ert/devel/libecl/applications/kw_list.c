/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'kw_list.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>
#include <string.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_endian_flip.h>


void kw_list(const char *filename) {
  fortio_type *fortio;
  ecl_kw_type * ecl_kw = ecl_kw_alloc_empty();
  bool fmt_file;
  if (ecl_util_fmt_file(filename , &fmt_file)) {

    printf("-----------------------------------------------------------------\n");
    printf("%s: \n",filename);
    fortio = fortio_open_reader(filename , fmt_file , ECL_ENDIAN_FLIP);
    while(  ecl_kw_fread_realloc(ecl_kw , fortio) )
      ecl_kw_summarize(ecl_kw);
    printf("-----------------------------------------------------------------\n");

    ecl_kw_free(ecl_kw);
    fortio_fclose(fortio);
  } else
    fprintf(stderr,"Could not determine formatted/unformatted status of:%s - skipping \n",filename);
}


int main (int argc , char **argv) {
  int i;
  for (i = 1; i < argc; i++)
    kw_list(argv[i]);
  return 0;
}
