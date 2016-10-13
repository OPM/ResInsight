/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'well_info.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw_magic.h>

#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_rseg_loader.h>

#include <ert/ecl/fortio.h>


struct well_rseg_loader_struct {
  ecl_file_type       * rst_file;
  int_vector_type     * relative_index_map;
  int_vector_type     * absolute_index_map;
  char                * buffer;
  char                * kw;
};


well_rseg_loader_type * well_rseg_loader_alloc(ecl_file_type * rst_file) {
    well_rseg_loader_type * loader = util_malloc(sizeof * loader);

    int element_count = 4;

    loader->rst_file = rst_file;
    loader->relative_index_map = int_vector_alloc(0, 0);
    loader->absolute_index_map = int_vector_alloc(0, 0);
    loader->buffer = util_malloc(element_count * sizeof(double));
    loader->kw = RSEG_KW;

    int_vector_append(loader->relative_index_map, RSEG_DEPTH_INDEX);
    int_vector_append(loader->relative_index_map, RSEG_LENGTH_INDEX);
    int_vector_append(loader->relative_index_map, RSEG_TOTAL_LENGTH_INDEX);
    int_vector_append(loader->relative_index_map, RSEG_DIAMETER_INDEX);

    return loader;
}

int well_rseg_loader_element_count(const well_rseg_loader_type * well_rseg_loader) {
    return int_vector_size(well_rseg_loader->relative_index_map);
}


void well_rseg_loader_free(well_rseg_loader_type * loader) {
    if(ecl_file_flags_set(loader->rst_file, ECL_FILE_CLOSE_STREAM)) {
        ecl_file_close_fortio_stream(loader->rst_file);
    }

    int_vector_free(loader->relative_index_map);
    int_vector_free(loader->absolute_index_map);
    free(loader->buffer);
    free(loader);
}

double * well_rseg_loader_load_values(const well_rseg_loader_type * loader, int rseg_offset) {
    int_vector_type * index_map = loader->absolute_index_map;

    int index = 0;
    for(index = 0; index < int_vector_size(loader->relative_index_map); index++) {
        int relative_index = int_vector_iget(loader->relative_index_map, index);
        int_vector_iset(index_map, index, relative_index + rseg_offset);
    }

    ecl_file_indexed_read(loader->rst_file, loader->kw, 0, index_map, loader->buffer);

    return (double*) loader->buffer;
}

