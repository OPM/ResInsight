/*
  Copyright 2015 Statoil ASA.

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

#include <string>
#include <iostream>

#include <opm/input/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableContainer.hpp>

namespace Opm {

    TableContainer::TableContainer() :
        m_maxTables(0)
    {
    }

    TableContainer::TableContainer(size_t maxTables) :
        m_maxTables(maxTables)
    {
    }

    TableContainer TableContainer::serializeObject()
    {
        TableContainer result;
        result.m_maxTables = 2;
        result.addTable(0, std::make_shared<Opm::SimpleTable>(Opm::SimpleTable::serializeObject()));
        result.addTable(1, std::make_shared<Opm::SimpleTable>(Opm::SimpleTable::serializeObject()));

        return result;
    }

    bool TableContainer::empty() const {
        return m_tables.empty();
    }


    size_t TableContainer::size() const {
        return m_tables.size();
    }


    size_t TableContainer::max() const {
        return m_maxTables;
    }

    const TableContainer::TableMap& TableContainer::tables() const {
        return m_tables;
    }


    size_t TableContainer::hasTable(size_t tableNumber) const {
        if (m_tables.find( tableNumber ) == m_tables.end())
            return false;
        else
            return true;
    }


    const SimpleTable& TableContainer::getTable(size_t tableNumber) const {
        if (tableNumber >= m_maxTables)
            throw std::invalid_argument("TableContainer - invalid tableNumber");

        if (hasTable(tableNumber)) {
            auto pair = m_tables.find( tableNumber );
            return *(pair->second.get());
        } else {
            if (tableNumber > 0)
                return getTable(tableNumber -1);
            else
                throw std::invalid_argument("TableContainer does not have any table in the range 0..." + std::to_string( tableNumber ));
        }
    }


    const SimpleTable& TableContainer::operator[](size_t tableNumber) const {
        return getTable(tableNumber);
    }

    void TableContainer::addTable(size_t tableNumber , std::shared_ptr<SimpleTable> table) {
        if (tableNumber >= m_maxTables)
            throw std::invalid_argument("TableContainer has max: " + std::to_string( m_maxTables ) + " tables. Table number: " + std::to_string( tableNumber ) + " illegal.");

        m_tables[tableNumber] = table;
    }


    bool TableContainer::operator==(const TableContainer& data) const {
        if (this->max() != data.max())
            return false;
        if (this->size() != data.size())
            return false;
        for (const auto& it : m_tables) {
            auto it2 = data.m_tables.find(it.first);
            if (it2 == data.m_tables.end())
                return false;
            if (!(*it.second == *it2->second))
                return false;
        }

        return true;
    }
}



