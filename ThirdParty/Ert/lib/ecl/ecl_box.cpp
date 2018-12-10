/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_box.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>

#include <algorithm>

#include <ert/ecl/ecl_box.hpp>
#include <ert/ecl/ecl_grid.hpp>


namespace ecl {
  ecl_box::ecl_box(const ecl_grid_type * grid, int i1, int i2, int j1, int j2, int k1, int k2) :
    grid(grid),
    i1(std::min(i1,i2)),
    i2(std::max(i1,i2)),
    j1(std::min(j1,j2)),
    j2(std::max(j1,j2)),
    k1(std::min(k1,k2)),
    k2(std::max(k1,k2))
  {
    for (int k=this->k1; k <= this->k2; k++)
      for (int j=this->j1; j <= this->j2; j++)
        for (int i=this->i1; i <= this->i2; i++) {
          int active_index = ecl_grid_get_active_index3(this->grid, i, j , k);
          if (active_index >= 0)
            this->active_index_list.push_back(active_index);
          this->global_index_list.push_back(ecl_grid_get_global_index3(this->grid, i, j, k));
        }
  }



  const std::vector<int>& ecl_box::active_list() const {
    return this->active_index_list;
  }

}
