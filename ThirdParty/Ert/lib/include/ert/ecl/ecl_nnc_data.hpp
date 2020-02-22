/*
   Copyright (C) 2017  Equinor ASA, Norway.

   The file 'ecl_nnc_geometry.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ECL_NNC_DATA_H
#define ECL_NNC_DATA_H

#include <ert/util/type_macros.hpp>

#include <ert/ecl/ecl_nnc_geometry.hpp>
#include <ert/ecl/ecl_nnc_export.hpp>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecl_nnc_data_struct ecl_nnc_data_type;

ecl_nnc_data_type * ecl_nnc_data_alloc_tran(const ecl_grid_type * grid, const ecl_nnc_geometry_type * nnc_geo, const ecl_file_view_type * init_file);
ecl_nnc_data_type * ecl_nnc_data_alloc_wat_flux(const ecl_grid_type * grid, const ecl_nnc_geometry_type * nnc_geo, const ecl_file_view_type * init_file);
ecl_nnc_data_type * ecl_nnc_data_alloc_oil_flux(const ecl_grid_type * grid, const ecl_nnc_geometry_type * nnc_geo, const ecl_file_view_type * init_file);
ecl_nnc_data_type * ecl_nnc_data_alloc_gas_flux(const ecl_grid_type * grid, const ecl_nnc_geometry_type * nnc_geo, const ecl_file_view_type * init_file);
void                       ecl_nnc_data_free(ecl_nnc_data_type * data);

int                        ecl_nnc_data_get_size(ecl_nnc_data_type * data);
const double            *  ecl_nnc_data_get_values( const ecl_nnc_data_type * data );

double                     ecl_nnc_data_iget_value(const ecl_nnc_data_type * data, int index);

#ifdef __cplusplus
}
#endif
#endif
