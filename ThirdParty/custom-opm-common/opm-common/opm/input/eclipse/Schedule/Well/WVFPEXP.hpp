/*
  Copyright 2022 Equinor.

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

#ifndef WVFPEXP_HPP_HEADER_INCLUDED
#define WVFPEXP_HPP_HEADER_INCLUDED

namespace Opm {
    class DeckRecord;
} // namespace Opm

namespace Opm {

    class WVFPEXP
    {
    public:
        static WVFPEXP serializeObject();

        void update(const DeckRecord& record);

        bool explicit_lookup() const;
        bool shut() const;
        bool prevent() const;

        bool report_first() const;
        bool report_every() const;

        bool operator==(const WVFPEXP& other) const;
        bool operator!=(const WVFPEXP& other) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_explicit);
            serializer(m_shut);
            serializer(m_prevent);
        }

    private:
        enum class Prevent : unsigned char { No, ReportFirst, ReportEvery };

        bool m_explicit{false};
        bool m_shut{false};
        Prevent m_prevent{Prevent::No};
    };

} // namespace Opm

#endif  // WVFPEXP_HPP_HEADER_INCLUDED
