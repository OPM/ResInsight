/*
  Copyright 2020 Equinor ASA.

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


#ifndef FROM_WSEG_HPP
#define FROM_WSEG_HPP

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <map>
#include <string>
#include <vector>

namespace Opm {

template<class ICD>
std::map<std::string, std::vector<std::pair<int, ICD> > >
fromWSEG(const DeckKeyword& wseg) {
         std::map<std::string, std::vector<std::pair<int, ICD> > > res;

    for (const DeckRecord &record : wseg) {
        const std::string well_name = record.getItem("WELL").getTrimmedString(0);

        const int start_segment = record.getItem("SEGMENT1").get<int>(0);
        const int end_segment = record.getItem("SEGMENT2").get<int>(0);

        if (start_segment < 2 || end_segment < 2 || end_segment < start_segment) {
            const std::string message = "Segment numbers " + std::to_string(start_segment) + " and "
                + std::to_string(end_segment) + " specified in WSEGSICD for well " +
                well_name
                + " are illegal ";
            throw std::invalid_argument(message);
        }

        const ICD spiral_icd(record);
        for (int seg = start_segment; seg <= end_segment; seg++) {
            res[well_name].push_back(std::make_pair(seg, spiral_icd));
        }
    }
    return res;
}

}

#endif
