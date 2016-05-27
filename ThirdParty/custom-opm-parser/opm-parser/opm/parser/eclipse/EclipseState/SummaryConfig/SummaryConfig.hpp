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

#ifndef OPM_SUMMARY_CONFIG_HPP
#define OPM_SUMMARY_CONFIG_HPP

#include <vector>

#include <ert/ecl/Smspec.hpp>

namespace Opm {

    class Deck;
    class EclipseState;
    class ParserKeyword;

    class SummaryConfig {
        public:
            typedef std::vector< ERT::smspec_node >::const_iterator const_iterator;

            SummaryConfig( const Deck&, const EclipseState& );

            const_iterator begin() const;
            const_iterator end() const;

        private:
            std::vector< ERT::smspec_node > keywords;
    };

} //namespace Opm

#endif
