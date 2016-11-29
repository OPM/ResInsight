/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_plot_genvector.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ENKF_PLOT_GENVECTOR_H
#define ERT_ENKF_PLOT_GENVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_config_node.h>

typedef struct enkf_plot_genvector_struct enkf_plot_genvector_type;

enkf_plot_genvector_type *  enkf_plot_genvector_alloc( const enkf_config_node_type * enkf_config_node , int iens);
void                        enkf_plot_genvector_free( enkf_plot_genvector_type * vector );
int                         enkf_plot_genvector_get_size( const enkf_plot_genvector_type * vector );
void                        enkf_plot_genvector_reset( enkf_plot_genvector_type * vector );
  void                      * enkf_plot_genvector_load__( void * arg );
double                      enkf_plot_genvector_iget( const enkf_plot_genvector_type * vector , int index);

UTIL_IS_INSTANCE_HEADER( enkf_plot_genvector );


#ifdef __cplusplus
}
#endif
#endif
