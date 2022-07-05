/*
  Copyright 2013 Statoil ASA.

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

#include <opm/input/eclipse/Units/Dimension.hpp>

#include <string>
#include <stdexcept>
#include <cmath>

namespace Opm {

    Dimension::Dimension()
    {
        this->m_SIfactor = 1.0;
        this->m_SIoffset = 0.0;

    }

    Dimension::Dimension(double SIfactor,
                         double SIoffset)
    {
        m_SIfactor = SIfactor;
        m_SIoffset = SIoffset;
    }

    Dimension Dimension::serializeObject()
    {
        return Dimension(1.0, 2.0);
    }

    double Dimension::getSIScaling() const {
        if (!std::isfinite(m_SIfactor))
            throw std::logic_error("The DeckItem contains a field with a context dependent unit. "
                                   "Use getData< double >() and convert the returned value manually!");
        return m_SIfactor;
    }

    double Dimension::getSIOffset() const {
        return m_SIoffset;
    }

    double Dimension::convertRawToSi(double rawValue) const {
        if (!std::isfinite(m_SIfactor))
            throw std::logic_error("The DeckItem contains a field with a context dependent unit. "
                                   "Use getData< double >() and convert the returned value manually!");

        return rawValue*m_SIfactor + m_SIoffset;
    }

    double Dimension::convertSiToRaw(double siValue) const {
        if (!std::isfinite(m_SIfactor))
            throw std::logic_error("The DeckItem contains a field with a context dependent unit. "
                                   "Use getData< double >() and convert the returned value manually!");

        return (siValue - m_SIoffset)/m_SIfactor;
    }


    // only dimensions with zero offset are compositable...
    bool Dimension::isCompositable() const
    { return m_SIoffset == 0.0; }


    bool Dimension::equal(const Dimension& other) const {
        return *this == other;
    }

    bool Dimension::operator==( const Dimension& rhs ) const {
        if( this->m_SIfactor == rhs.m_SIfactor
         && this->m_SIoffset == rhs.m_SIoffset ) return true;

        return std::isnan( this->m_SIfactor ) && std::isnan( rhs.m_SIfactor );
    }

    bool Dimension::operator!=( const Dimension& rhs ) const {
        return !(*this == rhs );
    }
}


