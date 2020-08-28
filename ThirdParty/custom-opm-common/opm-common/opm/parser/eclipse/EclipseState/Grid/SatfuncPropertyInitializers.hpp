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

#include <string>
#include <vector>

namespace Opm {

    class Phases;
    class TableManager;

namespace satfunc {

    std::vector<double> SGLEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> ISGLEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> SGUEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> ISGUEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> SWLEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> ISWLEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> SWUEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> ISWUEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> SGCREndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> ISGCREndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> SOWCREndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> ISOWCREndpoint(const TableManager&,
                                       const Phases&,
                                       const std::vector<double>&,
                                       const std::vector<int>&,
                                       const std::vector<int>&);

    std::vector<double> SOGCREndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> ISOGCREndpoint(const TableManager&,
                                       const Phases&,
                                       const std::vector<double>&,
                                       const std::vector<int>&,
                                       const std::vector<int>&);

    std::vector<double> SWCREndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> ISWCREndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> PCWEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> IPCWEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> PCGEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> IPCGEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> KRWEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> IKRWEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> KRWREndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> IKRWREndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> KROEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> IKROEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> KRORWEndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> IKRORWEndpoint(const TableManager&,
                                       const Phases&,
                                       const std::vector<double>&,
                                       const std::vector<int>&,
                                       const std::vector<int>&);

    std::vector<double> KRORGEndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> IKRORGEndpoint(const TableManager&,
                                       const Phases&,
                                       const std::vector<double>&,
                                       const std::vector<int>&,
                                       const std::vector<int>&);

    std::vector<double> KRGEndpoint(const TableManager&,
                                    const Phases&,
                                    const std::vector<double>&,
                                    const std::vector<int>&,
                                    const std::vector<int>&);

    std::vector<double> IKRGEndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> KRGREndpoint(const TableManager&,
                                     const Phases&,
                                     const std::vector<double>&,
                                     const std::vector<int>&,
                                     const std::vector<int>&);

    std::vector<double> IKRGREndpoint(const TableManager&,
                                      const Phases&,
                                      const std::vector<double>&,
                                      const std::vector<int>&,
                                      const std::vector<int>&);

    std::vector<double> init(const std::string& kewyord,
                             const TableManager& tables,
                             const Phases& phases,
                             const std::vector<double>& cell_depth,
                             const std::vector<int>& num,
                             const std::vector<int>& endnum);
}
}

#endif // ECLIPSE_SATFUNCPROPERTY_INITIALIZERS_HPP
