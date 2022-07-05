/*
  Copyright (C) 2014 by Andreas Lauser

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
#ifndef OPM_PARSER_PLYROCK_TABLE_HPP
#define	OPM_PARSER_PLYROCK_TABLE_HPP

#include "SimpleTable.hpp"

namespace Opm {

    class DeckReckord;

    class PlyrockTable : public SimpleTable {
    public:

        // This is not really a table; every column has only one element.
        PlyrockTable( const DeckRecord& record );

        // since this keyword is not necessarily monotonic, it cannot be evaluated!
        //using SimpleTable::evaluate;

        const TableColumn& getDeadPoreVolumeColumn() const;
        const TableColumn& getResidualResistanceFactorColumn() const;
        const TableColumn& getRockDensityFactorColumn() const;

        // is column is actually an integer, but this is not yet
        // supported by opm-parser (yet?) as it would require quite a
        // few changes in the table support classes (read: it would
        // probably require a lot of template vodoo) and some in the
        // JSON-to-C conversion code. In the meantime, the index is
        // just a double which can be converted to an integer in the
        // calling code. (Make sure, that you don't interpolate
        // indices, though!)
        const TableColumn& getAdsorbtionIndexColumn() const;
        const TableColumn& getMaxAdsorbtionColumn() const;
    };
}

#endif
