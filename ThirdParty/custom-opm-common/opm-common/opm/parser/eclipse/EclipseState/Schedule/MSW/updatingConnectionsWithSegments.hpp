/*
  Copyright 2017 SINTEF Digital, Mathematics and Cybernetics.

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

#ifndef UPDATING_CONNECTIONS_WITH_SEGMENTS
#define UPDATING_CONNECTIONS_WITH_SEGMENTS

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/MSW/WellSegments.hpp>

namespace Opm {
    WellConnections * newConnectionsWithSegments(const DeckKeyword& compsegs, const WellConnections& input_connections,
                                                 const WellSegments& segments, const EclipseGrid& grid,
                                                 const ParseContext& parseContext, ErrorGuard& errors);
}

#endif
