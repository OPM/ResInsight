/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_plot_gendata.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ENKF_PLOT_GENDATA_H
#define ERT_ENKF_PLOT_GENDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/util/double_vector.h>

#include <ert/enkf/obs_vector.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_plot_genvector.h>



typedef struct enkf_plot_gendata_struct enkf_plot_gendata_type;

enkf_plot_gendata_type *    enkf_plot_gendata_alloc( const enkf_config_node_type * enkf_config_node);
enkf_plot_gendata_type *    enkf_plot_gendata_alloc_from_obs_vector( const obs_vector_type * obs_vector );
void                        enkf_plot_gendata_free( enkf_plot_gendata_type * data );
int                         enkf_plot_gendata_get_size( const enkf_plot_gendata_type * data );
enkf_plot_genvector_type *  enkf_plot_gendata_iget( const enkf_plot_gendata_type * plot_data , int index);
void                        enkf_plot_gendata_load( enkf_plot_gendata_type * plot_data ,
                                                    enkf_fs_type * fs ,
                                                    int report_step ,
                                                    const bool_vector_type * input_mask);

double_vector_type * enkf_plot_gendata_get_min_values(enkf_plot_gendata_type * plot_data);
double_vector_type * enkf_plot_gendata_get_max_values(enkf_plot_gendata_type * plot_data);


UTIL_IS_INSTANCE_HEADER( enkf_plot_gendata );

#ifdef __cplusplus
}
#endif
#endif
