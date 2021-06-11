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
#ifndef OPM_PARSER_SGOF_TABLE_HPP
#define	OPM_PARSER_SGOF_TABLE_HPP

#include "SimpleTable.hpp"

namespace Opm {
    class DeckItem;

    class SgofTable : public SimpleTable {

    public:
        SgofTable( const DeckItem& item, const bool jfunc );

        const TableColumn& getSgColumn() const;
        const TableColumn& getKrgColumn() const;
        const TableColumn& getKrogColumn() const;

        // this column is p_g - p_o (non-wetting phase pressure minus
        // wetting phase pressure for a given gas saturation. the name
        // is inconsistent, but it is the one used in the Eclipse
        // manual...)
        const TableColumn& getPcogColumn() const;

        const TableColumn& getJFuncColumn() const;
    };
}

#endif
