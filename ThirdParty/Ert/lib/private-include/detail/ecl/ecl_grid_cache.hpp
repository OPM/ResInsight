/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_grid_cache.h' is part of ERT - Ensemble based
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

#ifndef ERT_ECL_GRID_CACHE_H
#define ERT_ECL_GRID_CACHE_H

#include <vector>

#include <ert/ecl/ecl_grid.hpp>

namespace ecl {
  class ecl_grid_cache {
  public:
    ecl_grid_cache(const ecl_grid_type * grid);

    const std::vector<double>& volume() const;
    const std::vector<double>& xpos() const { return this->xp; }
    const std::vector<double>& ypos() const { return this->yp; }
    const std::vector<double>& zpos() const { return this->zp; }
    const std::vector<int>& global_index( ) const { return this->gi; }
    int size() const { return this->xp.size(); }

  private:
    const ecl_grid_type * grid;
    std::vector<int> gi;
    std::vector<double> xp;
    std::vector<double> yp;
    std::vector<double> zp;
    mutable std::vector<double> v;
  };
}

#endif
