/*
  Copyright 2016 Statoil ASA.
  Copyright 2016 SINTEF ICT, Applied Mathematics.

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

#ifndef OPM_CELLSET_HEADER_INCLUDED
#define OPM_CELLSET_HEADER_INCLUDED

#include <string>
#include <unordered_set>

namespace Opm
{
namespace FlowDiagnostics
{


    class CellSetID
    {
    public:
        using Repr = std::string;

        CellSetID();

        explicit CellSetID(Repr id);

        std::string to_string() const;

    private:
        Repr id_;
    };

    class CellSet
    {
    private:
        using IndexSet = std::unordered_set<int>;

    public:
        using const_iterator = IndexSet::const_iterator;

        void identify(CellSetID id);

        const CellSetID& id() const;

        void insert(const int cell);

        const_iterator begin() const;
        const_iterator end()   const;

    private:
        CellSetID id_;
        IndexSet  iset_;
    };


} // namespace FlowDiagnostics
} // namespace Opm

#endif  // OPM_CELLSET_HEADER_INCLUDED
