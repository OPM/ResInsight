/*
   Copyright (C) 2012 Statoil ASA, Norway.

   The file 'ecl_init_file.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_INIT_FILE_H
#define ERT_ECL_INIT_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_util.h>

  void ecl_init_file_fwrite_header( fortio_type * fortio , const ecl_grid_type * grid , const ecl_kw_type * poro , ert_ecl_unit_enum unit_system, int phases , time_t start_date);


#ifdef __cplusplus
}
#endif
#endif
