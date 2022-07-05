/*
  Copyright 2014 Statoil ASA.

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

#include <opm/input/eclipse/EclipseState/Grid/Fault.hpp>

namespace Opm {

    Fault::Fault(const std::string& faultName) :
        m_name( faultName ),
        m_transMult( 1 )
    {
    }

    Fault Fault::serializeObject()
    {
        Fault result;
        result.m_name = "test";
        result.m_transMult = 1.0;
        result.m_faceList = {FaultFace::serializeObject()};

        return result;
    }


    const std::string& Fault::getName() const {
        return m_name;
    }

    double Fault::getTransMult() const {
        return m_transMult;
    }

    void Fault::setTransMult(double transMult) {
        m_transMult = transMult;
    }


    void Fault::addFace( FaultFace face ) {
        m_faceList.push_back( std::move( face ) );
    }

    std::vector< FaultFace >::const_iterator Fault::begin() const {
        return m_faceList.begin();
    }


    std::vector< FaultFace >::const_iterator Fault::end() const {
        return m_faceList.end();
    }

    bool Fault::operator==( const Fault& rhs ) const {
        return this->m_name == rhs.m_name
            && this->m_transMult == rhs.m_transMult
            && this->m_faceList.size() == rhs.m_faceList.size()
            && std::equal( this->m_faceList.begin(),
                           this->m_faceList.end(),
                           rhs.m_faceList.begin() );
    }

    bool Fault::operator!=( const Fault& rhs ) const {
        return !( *this == rhs );
    }

}
