/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grid_cache.c' is part of ERT - Ensemble based
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

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_grid.hpp>

#include "detail/ecl/ecl_grid_cache.hpp"




/**
   The ecl_grid_cache_struct data structure internalizes the world
   position of all the active cells. This is just a minor
   simplification to speed up repeated calls to get the true world
   coordinates of a cell.
*/

namespace ecl {
  ecl_grid_cache::ecl_grid_cache(const ecl_grid_type * grid) :
    grid(grid)
  {
    for (int active_index = 0; active_index < ecl_grid_get_active_size(this->grid); active_index++) {
      double x,y,z;
      int global_index = ecl_grid_get_global_index1A(this->grid, active_index);
      ecl_grid_get_xyz1(this->grid, global_index, &x, &y, &z);

      this->gi.push_back(global_index);
      this->xp.push_back(x);
      this->yp.push_back(y);
      this->zp.push_back(z);
    }
  }


  const std::vector<double>& ecl_grid_cache::volume() const {
    if (this->v.empty()) {
      for (int active_index = 0; active_index < this->size(); active_index++)
        this->v.push_back( ecl_grid_get_cell_volume1A(this->grid, active_index));
    }
    return this->v;
  }

}


