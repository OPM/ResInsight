/*
  Copyright 2019 by Norce.

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

#ifndef OPM_WELLBRINEPROPERTIES_HEADER_INCLUDED
#define OPM_WELLBRINEPROPERTIES_HEADER_INCLUDED

namespace Opm
{

class DeckRecord;

struct WellBrineProperties
{
    double m_saltConcentration = 0.0;

    void handleWSALT(const DeckRecord& rec);
    bool operator!=(const WellBrineProperties& other) const;
    bool operator==(const WellBrineProperties& other) const;

    static WellBrineProperties serializeObject();

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_saltConcentration);
    }
};

} // namespace Opm

#endif // OPM_WELLBRINEPROPERTIES_HEADER_INCLUDED
