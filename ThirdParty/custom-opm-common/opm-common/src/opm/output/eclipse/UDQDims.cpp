/*
  Copyright (c) 2021 Equinor ASA

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

#include <opm/output/eclipse/UDQDims.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>


namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace Opm {

std::size_t UDQDims::entriesPerIUDQ()
{
    std::size_t no_entries = 3;
    return no_entries;
}

std::size_t UDQDims::entriesPerIUAD()
{
    std::size_t no_entries = 5;
    return no_entries;
}

std::size_t UDQDims::entriesPerZUDN()
{
    std::size_t no_entries = 2;
    return no_entries;
}

std::size_t UDQDims::entriesPerZUDL()
{
    std::size_t no_entries = 16;
    return no_entries;
}

const std::vector<int>& UDQDims::data() const {
    return this->m_data;
}


UDQDims::UDQDims(const UDQConfig& config, const std::vector<int>& inteHead)
{
    this->m_data.resize(13,0);

    this->m_data[ 0] = config.size();
    this->m_data[ 1] = entriesPerIUDQ();
    this->m_data[ 2] = inteHead[VI::intehead::NO_IUADS];
    this->m_data[ 3] = entriesPerIUAD();
    this->m_data[ 4] = entriesPerZUDN();
    this->m_data[ 5] = entriesPerZUDL();
    this->m_data[ 6] = (inteHead[VI::intehead::NO_IUADS] > 0) ? inteHead[20] : 0;
    this->m_data[ 7] = inteHead[VI::intehead::NO_IUAPS];
    this->m_data[ 8] = inteHead[VI::intehead::NWMAXZ];
    this->m_data[ 9] = inteHead[VI::intehead::NO_WELL_UDQS];
    this->m_data[10] = inteHead[VI::intehead::NGMAXZ];
    this->m_data[11] = inteHead[VI::intehead::NO_GROUP_UDQS];
    this->m_data[12] = inteHead[VI::intehead::NO_FIELD_UDQS];
}
}
