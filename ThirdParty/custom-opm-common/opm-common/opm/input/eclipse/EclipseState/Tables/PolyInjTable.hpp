/*
  Copyright (C) 2018 Statoil ASA

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

#ifndef OPM_PARSER_POLY_INJ_TABLE_HPP
#define OPM_PARSER_POLY_INJ_TABLE_HPP


/* This class is introduced for the following keywords related to polymer injectivity study.
 * PLYMWINJ, SKPRWAT, SKPRPOLY .
 * These keywords share very similar structure with small difference.
 *
 * KEYWORD
 *  1 / --table number
 *  0  20  30 / -- water throughputs
 *  0 0.1 0.2 0.3 / -- water velocities
 *  -- the rest is the table data,
 *  -- each row corresponds to one value in throughputs
 *  -- each column corresponds to one value in water velocities
 *  20 19 18 17 /
 *  20 18 17 16 /
 *  20 17 16 15 /
 */

#include <vector>

namespace Opm {

    class PolyInjTable {
    public:
        static PolyInjTable serializeObject();

        int getTableNumber() const;

        const std::vector<double>& getThroughputs() const;
        const std::vector<double>& getVelocities() const;
        const std::vector<std::vector<double>>& getTableData() const;

        bool operator==(const PolyInjTable& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_throughputs);
            serializer(m_velocities);
            serializer(m_table_number);
            serializer(m_data);
        }

    protected:
        std::vector<double> m_throughputs;
        std::vector<double> m_velocities;

        // TODO: maybe not needed, since this is also stored in the std::map
        int m_table_number = 0;

        // each vector corresponds to the values corresponds to one value related to one x sampling point
        // as a result, the number of the vector should be equal to be the size of m_x_points,
        // the size of each vector should be equal to the size of m_y_points
        std::vector<std::vector<double> > m_data;
    };
}

#endif // OPM_PARSER_POLY_INJ_TABLE_HPP
