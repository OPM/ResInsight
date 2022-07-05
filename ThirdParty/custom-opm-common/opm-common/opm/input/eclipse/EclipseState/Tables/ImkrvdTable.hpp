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
#ifndef OPM_PARSER_IMKRVD_TABLE_HPP
#define	OPM_PARSER_IMKRVD_TABLE_HPP

#include "SimpleTable.hpp"

namespace Opm {

    class DeckItem;

    class ImkrvdTable : public SimpleTable {
    public:
        ImkrvdTable( const DeckItem& item, const int tableID );

        /*!
         * \brief The datum depth for the remaining columns
         */
        const TableColumn& getDepthColumn() const;

        /*!
         * \brief Maximum relative permeability of water
         */
        const TableColumn& getKrwmaxColumn() const;

        /*!
         * \brief Maximum relative permeability of gas
         */
        const TableColumn& getKrgmaxColumn() const;

        /*!
         * \brief Maximum relative permeability of oil
         */
        const TableColumn& getKromaxColumn() const;

        /*!
         * \brief Relative permeability of water at the critical oil (or gas) saturation
         */
        const TableColumn& getKrwcritColumn() const;

        /*!
         * \brief Relative permeability of gas at the critical oil (or water) saturation
         */
        const TableColumn& getKrgcritColumn() const;

        /*!
         * \brief Oil relative permeability of oil at the critical gas saturation
         */
        const TableColumn& getKrocritgColumn() const;

        /*!
         * \brief Oil relative permeability of oil at the critical water saturation
         */
        const TableColumn& getKrocritwColumn() const;
    };
}

#endif
