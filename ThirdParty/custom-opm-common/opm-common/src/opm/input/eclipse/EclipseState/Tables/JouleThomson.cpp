/*
  Copyright (C) 2020 by Equinor ASA

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
#include <opm/input/eclipse/EclipseState/Tables/JouleThomson.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>

namespace Opm {

JouleThomson::JouleThomson(const DeckKeyword& keyword) {
    for (const auto& record : keyword)
        this->m_records.emplace_back(record);
}

JouleThomson::entry::entry(const DeckRecord& record) :
    P0(record.getItem(0).getSIDouble(0)),
    C1(record.getItem(1).getSIDouble(0))
{
}

JouleThomson JouleThomson::serializeObject()
{
   JouleThomson result;
    result.m_records = {{1,2}, {3,4}};

    return result;
}

std::size_t JouleThomson::size() const {
    return this->m_records.size();
}


JouleThomson::entry::entry(double P0_, double C1_) :
    P0(P0_),
    C1(C1_)
{}


bool JouleThomson::entry::operator==(const JouleThomson::entry& other) const {
    return this->P0 == other.P0 &&
           this->C1 == other.C1;
}

bool JouleThomson::operator==(const JouleThomson& other) const {
    return this->m_records == other.m_records;
}

const JouleThomson::entry& JouleThomson::operator[](const std::size_t index) const {
    return this->m_records.at(index);
}

}
