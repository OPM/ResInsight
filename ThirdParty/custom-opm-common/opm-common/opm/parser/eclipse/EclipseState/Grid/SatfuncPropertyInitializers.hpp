/*
  Copyright 2014 Andreas Lauser

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
#ifndef ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP
#define ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP

#include <memory>
#include <string>
#include <vector>

namespace Opm {
    class Phases;
    class TableManager;
}

namespace Opm { namespace satfunc {

    struct RawTableEndPoints
    {
        struct {
            std::vector<double> gas;
            std::vector<double> water;
        } connate;

        struct {
            std::vector<double> oil_in_gas;
            std::vector<double> oil_in_water;
            std::vector<double> gas;
            std::vector<double> water;
        } critical;

        struct {
            std::vector<double> gas;
            std::vector<double> water;
        } maximum;
    };

    std::shared_ptr<RawTableEndPoints>
    getRawTableEndpoints(const Opm::TableManager& tm,
                         const Opm::Phases&       phases,
                         const double             tolcrit);

    std::vector<double> init(const std::string& kewyord,
                             const TableManager& tables,
                             const Phases& phases,
                             const RawTableEndPoints& ep,
                             const std::vector<double>& cell_depth,
                             const std::vector<int>& num,
                             const std::vector<int>& endnum);

}} // namespace Opm::satfunc

#endif // ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP
