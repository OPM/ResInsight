/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_box.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_BOX_H
#define ERT_ECL_BOX_H

#include <vector>
#include <ert/ecl/ecl_grid.hpp>


namespace ecl {

  class ecl_box {
  public:
    ecl_box(const ecl_grid_type * grid, int i1, int i2, int j1, int j2, int k1, int k2);
    const std::vector<int>& active_list() const;
  private:
    const ecl_grid_type * grid;

    int     i1,i2,j1,j2,k1,k2;
    std::vector<int> active_index_list;
    std::vector<int> global_index_list;
  };

}
#endif
