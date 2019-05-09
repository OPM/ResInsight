/*
   Copyright (C) 2011  Equinor ASA, Norway.

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

#ifndef ERT_WELL_RSEG_LOADER_H
#define ERT_WELL_RSEG_LOADER_H


#ifdef __cplusplus
extern "C" {
#endif

#include <ert/ecl/ecl_file_view.hpp>


  typedef struct well_rseg_loader_struct well_rseg_loader_type;

  well_rseg_loader_type *  well_rseg_loader_alloc(ecl_file_view_type * rst_view);
  void                     well_rseg_loader_free(well_rseg_loader_type * well_rseg_loader);

  int                      well_rseg_loader_element_count(const well_rseg_loader_type * well_rseg_loader);
  double *                 well_rseg_loader_load_values(const well_rseg_loader_type * well_rseg_loader, int rseg_offset);

#ifdef __cplusplus
}
#endif

#endif
