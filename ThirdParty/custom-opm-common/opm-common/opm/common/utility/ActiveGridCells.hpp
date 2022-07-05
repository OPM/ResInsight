/*
  Copyright 2019 Equinor ASA

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
#ifndef ACTIVEGRIDCELLS_HPP
#define ACTIVEGRIDCELLS_HPP

#include <opm/input/eclipse/EclipseState/Grid/GridDims.hpp>

#include <array>

namespace Opm
{

/**
 * \brief Simple class capturing active cells of a grid
 *
 */
class ActiveGridCells
    : public GridDims
{
public:
    /// \brief Constructs mapping of active cells.
    /// \param xyz The cartesian dimensions of the grid
    /// \param globalCell Pointer to first entry of contiguous
    ///        array mapping local index to cartesian one.
    /// \param nc The number of cells of a grid.
    ActiveGridCells(std::array<int, 3> xyz,
                    const int* globalCell, std::size_t nc);

    /// \brief Constructs mapping of active cells.
    /// \param nx Number of cells in x
    /// \param ny Number of cells in y
    /// \param nz Number of cells in z
    /// \param globalCell Pointer to first entry of contiguous
    ///        array mapping local index to cartesian one.
    /// \param nc The number of cells of a grid.
    ActiveGridCells(std::size_t nx, std::size_t ny, std::size_t nz,
                    const int* globalCell, std::size_t nc);

    bool cellActive(std::size_t i, std::size_t j, std::size_t k) const;

    bool cellActive(std::size_t cartesianIndex) const;

    std::vector<int> actNum() const;

    /// \brief Get the local index of a cell
    /// \param cartesianIndex The cartesian index of the cell
    /// \return The local index or -1 if the cell is inactive
    int localCell(std::size_t cartesianIndex) const;
    
    /// \brief Get the local index of a cell
    /// \param i The index in the i direction
    /// \param j The index in the j direction
    /// \param k The index in the k direction
    /// \return The local index or -1 if the cell is inactive
    int localCell(std::size_t i, std::size_t j, std::size_t k) const;
protected:
    /// \brief Maps the cartesian index to a compressed local index.
    ///
    /// nonactive cells are marked with -1.
    std::vector<int> localCell_;
};
} // end namespace Opm
#endif //  ACTIVEGRIDCELLS_HPP
