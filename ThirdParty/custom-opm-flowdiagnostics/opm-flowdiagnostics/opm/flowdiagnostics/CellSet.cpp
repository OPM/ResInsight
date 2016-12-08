/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/CellSet.hpp>

#include <utility>

namespace Opm
{
namespace FlowDiagnostics
{


CellSetID::CellSetID()
{}

CellSetID::CellSetID(Repr id)
    : id_(std::move(id))
{
}

std::string
CellSetID::to_string() const
{
    return id_;
}

bool
CellSetID::operator<(const CellSetID& other) const
{
    return id_ < other.id_;
}

// =====================================================================

CellSet::CellSet(CellSetID id)
    : id_(std::move(id))
{
}

CellSet::CellSet(CellSetID id, const std::vector<int>& cells)
    : id_(std::move(id)), iset_(cells.begin(), cells.end())
{
}

const CellSetID&
CellSet::id() const
{
    return id_;
}

void
CellSet::insert(const int i)
{
    iset_.insert(i);
}

CellSet::const_iterator
CellSet::begin() const
{
    return iset_.begin();
}

CellSet::const_iterator
CellSet::end() const
{
    return iset_.end();
}

} // namespace FlowDiagnostics
} // namespace Opm
