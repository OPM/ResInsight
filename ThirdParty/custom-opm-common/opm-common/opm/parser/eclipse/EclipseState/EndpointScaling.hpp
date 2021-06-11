/*
  Copyright 2017  Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_ENDPOINTSCALING_HPP
#define OPM_ENDPOINTSCALING_HPP

#include <bitset>

namespace Opm {
class Deck;


class EndpointScaling {
    public:
        EndpointScaling() noexcept = default;
        explicit EndpointScaling( const Deck& );

        static EndpointScaling serializeObject();

        /* true if endpoint scaling is enabled, otherwise false */
        operator bool() const noexcept;

        bool directional() const noexcept;
        bool nondirectional() const noexcept;
        bool reversible() const noexcept;
        bool irreversible() const noexcept;
        bool twopoint() const noexcept;
        bool threepoint() const noexcept;

        bool operator==(const EndpointScaling& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            if (serializer.isSerializing())
                serializer(options.to_ulong());
            else {
                unsigned long bits = 0;
                serializer(bits);
                options = std::bitset<4>(bits);
            }
        }

    private:
        enum class option {
            any         = 0,
            directional = 1,
            reversible  = 2,
            threepoint  = 3,
        };

        using ue = std::underlying_type< option >::type;
        std::bitset< 4 > options;
};
}

#endif
