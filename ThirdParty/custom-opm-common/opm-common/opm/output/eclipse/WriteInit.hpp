/*
  Copyright (c) 2019 Equinor ASA

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

#ifndef OPM_WRITE_INIT_HPP
#define OPM_WRITE_INIT_HPP

#include <map>
#include <string>
#include <vector>

#include <opm/input/eclipse/EclipseState/Grid/NNC.hpp>

namespace Opm {

    class EclipseGrid;
    class EclipseState;
    class NNC;
    class Schedule;

} // namespace Opm

namespace Opm { namespace data {

    class Solution;

}} // namespace Opm::data

namespace Opm { namespace EclIO { namespace OutputStream {

    class Init;

}}} // namespace Opm::EclIO::OutputStream

namespace Opm { namespace InitIO {

    void write(const ::Opm::EclipseState&              es,
               const ::Opm::EclipseGrid&               grid,
               const ::Opm::Schedule&                  schedule,
               const ::Opm::data::Solution&            simProps,
               std::map<std::string, std::vector<int>> int_data,
               const std::vector<::Opm::NNCdata>&      nnc,
               ::Opm::EclIO::OutputStream::Init&       initFile);

}} // namespace Opm::InitIO

#endif // OPM_WRITE_INIT_HPP
