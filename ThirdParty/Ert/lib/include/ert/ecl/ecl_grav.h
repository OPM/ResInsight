/*
   Copyright (C) 2011  Statoil ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_GRAV_H
#define ERT_ECL_GRAV_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_file_view.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_region.h>

typedef struct ecl_grav_struct            ecl_grav_type;
typedef struct ecl_grav_survey_struct     ecl_grav_survey_type;


void                   ecl_grav_free( ecl_grav_type * ecl_grav_config );
ecl_grav_type        * ecl_grav_alloc( const ecl_grid_type * ecl_grid, const ecl_file_type * init_file );
ecl_grav_survey_type * ecl_grav_add_survey_FIP( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file );
ecl_grav_survey_type * ecl_grav_add_survey_PORMOD( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file );
ecl_grav_survey_type * ecl_grav_add_survey_RPORV( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file );
ecl_grav_survey_type * ecl_grav_add_survey_RFIP( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file );
double                 ecl_grav_eval( const ecl_grav_type * grav , const char * base, const char * monitor , ecl_region_type * region , double utm_x, double utm_y , double depth, int phase_mask);
void                   ecl_grav_new_std_density( ecl_grav_type * grav , ecl_phase_enum phase , double default_density);
void                   ecl_grav_add_std_density( ecl_grav_type * grav , ecl_phase_enum phase , int pvtnum , double density);

#ifdef __cplusplus
}
#endif
#endif
