/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_subsidence.h' is part of ERT - Ensemble based
   Reservoir Tool.

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

#ifndef __ECL_SUBSIDENCE_H__
#define __ECL_SUBSICENCE_H__
#ifdef __plusplus
extern "C" {
#endif

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_region.h>

  typedef struct ecl_subsidence_struct            ecl_subsidence_type;
  typedef struct ecl_subsidence_survey_struct     ecl_subsidence_survey_type;
  

  void                         ecl_subsidence_free( ecl_subsidence_type * ecl_subsidence_config );
  ecl_subsidence_type        * ecl_subsidence_alloc( const ecl_grid_type * ecl_grid, const ecl_file_type * init_file );
  ecl_subsidence_survey_type * ecl_subsidence_add_survey_PRESSURE( ecl_subsidence_type * subsidence , 
                                                                   const char * name , const ecl_file_type * restart_file );
  double                       ecl_subsidence_eval( const ecl_subsidence_type * subsidence , 
                                                    const char * base, const char * monitor , 
                                                    ecl_region_type * region , 
                                                    double utm_x, double utm_y , double depth, double compressibility, double poisson_ratio);


#ifdef __plusplus
}
#endif
#endif
