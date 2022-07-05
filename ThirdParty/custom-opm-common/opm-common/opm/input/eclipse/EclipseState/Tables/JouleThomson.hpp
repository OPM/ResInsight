/*
  Copyright (C) 2020 by Equinor

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
#ifndef OPM_PARSER_JOULETHOMSON_HPP
#define OPM_PARSER_JOULETHOMSON_HPP

#include <cstddef>
#include <vector>

namespace Opm {

    class DeckKeyword;
    class DeckRecord;

    class JouleThomson {
    public:

        struct entry {
            double P0;
            double C1;

            entry() = default;
            entry(double P0_, double C1_);
            explicit entry(const DeckRecord& record);
            bool operator==(const entry& other) const;

            template<class Serializer>
            void serializeOp(Serializer& serializer)
            {
                serializer(P0);
                serializer(C1);
            }
        };

        JouleThomson() = default;
        explicit JouleThomson(const DeckKeyword& keyword);

        static JouleThomson serializeObject();

        const entry& operator[](const std::size_t index) const;
        bool operator==(const JouleThomson& other) const;
        std::size_t size() const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer.vector(m_records);
        }

    private:
        std::vector<entry> m_records;
    };
}

#endif
