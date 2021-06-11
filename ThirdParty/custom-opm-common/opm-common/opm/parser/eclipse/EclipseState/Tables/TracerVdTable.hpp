/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.

  Consult the COPYING file in the top-level source directory of this
  module for the precise wording of the license and the list of
  copyright holders.
*/
/*!
 * \file
 *
 * \copydoc Opm::TracerVdTable
 */
#ifndef TRACER_VD_TABLE_HPP
#define TRACER_VD_TABLE_HPP

#include <opm/parser/eclipse/EclipseState/Tables/SimpleTable.hpp>

namespace Opm {

/*!
 * \brief A class that contains tracer concentration vs depth table
 */
class TracerVdTable : public SimpleTable
{
public:
    TracerVdTable() = default;
    explicit TracerVdTable(const Opm::DeckItem& item, double inv_volume);

    /*!
     * \brief Return the depth column
     */
    const Opm::TableColumn& getDepthColumn() const;

    /*!
     * \brief Return the tracer concentration column
     */
    const Opm::TableColumn& getTracerConcentration() const;
};
}


#endif // TRACER_VD_TABLE_HPP
