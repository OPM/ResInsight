/*
  Copyright 2019 SINTEF Digital, Mathematics and Cybernetics.

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

#ifndef OPM_WELLFOAMPROPERTIES_HEADER_INCLUDED
#define OPM_WELLFOAMPROPERTIES_HEADER_INCLUDED

namespace Opm
{

class DeckRecord;

struct WellFoamProperties
{
    static WellFoamProperties serializeObject();

    double m_foamConcentration = 0.0;
    void handleWFOAM(const DeckRecord& rec);
    bool operator==(const WellFoamProperties& other) const;
    bool operator!=(const WellFoamProperties& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_foamConcentration);
    }
};

} // namespace Opm

#endif // OPM_WELLFOAMPROPERTIES_HEADER_INCLUDED
