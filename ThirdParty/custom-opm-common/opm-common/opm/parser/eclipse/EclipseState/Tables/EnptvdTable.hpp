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
#ifndef OPM_PARSER_ENPTVD_TABLE_HPP
#define	OPM_PARSER_ENPTVD_TABLE_HPP

#include "SimpleTable.hpp"

namespace Opm {

    class DeckItem;

    class EnptvdTable : public SimpleTable {
    public:
        EnptvdTable( const DeckItem& item );
        
        // using this method is strongly discouraged but the current endpoint scaling
        // code makes it hard to avoid
        using SimpleTable::getColumn;

        const TableColumn& getDepthColumn() const;

        /*!
         * \brief Connate water saturation
         */
        const TableColumn& getSwcoColumn() const;

        /*!
         * \brief Critical water saturation
         */
        const TableColumn& getSwcritColumn() const;

        /*!
         * \brief Maximum water saturation
         */
        const TableColumn& getSwmaxColumn() const;

        /*!
         * \brief Connate gas saturation
         */
        const TableColumn& getSgcoColumn() const;

        /*!
         * \brief Critical gas saturation
         */
        const TableColumn& getSgcritColumn() const;

        /*!
         * \brief Maximum gas saturation
         */
        const TableColumn& getSgmaxColumn() const;

        /*!
         * \brief Critical oil-in-water saturation
         */
        const TableColumn& getSowcritColumn() const;

        /*!
         * \brief Critical oil-in-gas saturation
         */
        const TableColumn& getSogcritColumn() const;
    };
}

#endif

