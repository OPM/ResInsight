/*
  Copyright 2021 NORCE.

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

#ifndef OPM_WELLMICPPROPERTIES_HEADER_INCLUDED
#define OPM_WELLMICPPROPERTIES_HEADER_INCLUDED

namespace Opm
{

class DeckRecord;

struct WellMICPProperties
{
    static WellMICPProperties serializeObject();

    double m_microbialConcentration = 0.0;
    double m_oxygenConcentration = 0.0;
    double m_ureaConcentration = 0.0;
    void handleWMICP(const DeckRecord& rec);
    bool operator==(const WellMICPProperties& other) const;
    bool operator!=(const WellMICPProperties& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_microbialConcentration);
        serializer(m_oxygenConcentration);
        serializer(m_ureaConcentration);
    }
};

}

#endif
