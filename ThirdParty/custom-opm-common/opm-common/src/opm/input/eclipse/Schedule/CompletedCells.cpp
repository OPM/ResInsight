/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/input/eclipse/Schedule/CompletedCells.hpp>


Opm::CompletedCells::CompletedCells(std::size_t nx, std::size_t ny, std::size_t nz)
    : dims(nx,ny,nz)
{}


Opm::CompletedCells::CompletedCells(const Opm::GridDims& dims_)
    :dims(dims_)
{}


const Opm::CompletedCells::Cell& Opm::CompletedCells::get(std::size_t i, std::size_t j, std::size_t k) const {
    auto g = this->dims.getGlobalIndex(i,j,k);
    return this->cells.at(g);
}


std::pair<bool, Opm::CompletedCells::Cell&> Opm::CompletedCells::try_get(std::size_t i, std::size_t j, std::size_t k) {
    auto g = this->dims.getGlobalIndex(i,j,k);
    auto iter = this->cells.find(g);
    if (iter != this->cells.end())
        return {true, iter->second};

    this->cells.emplace(g, Cell{g,i,j,k});
    return {false, this->cells.at(g)};
}


bool Opm::CompletedCells::operator==(const Opm::CompletedCells& other) const {
    return this->dims == other.dims &&
           this->cells == other.cells;
}


Opm::CompletedCells Opm::CompletedCells::serializeObject() {
    Opm::CompletedCells cells(2,3,4);
    cells.cells.emplace(7, Opm::CompletedCells::Cell::serializeObject());
    return cells;
}

std::size_t Opm::CompletedCells::Cell::active_index() const{
    return this->props.value().active_index;
}

bool Opm::CompletedCells::Cell::is_active() const{
    return this->props.has_value();
}
