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
#include <stdexcept>

#include <opm/input/eclipse/EclipseState/Grid/FaultFace.hpp>

namespace Opm {

    FaultFace::FaultFace(size_t nx , size_t ny , size_t nz,
                         size_t I1 , size_t I2,
                         size_t J1 , size_t J2,
                         size_t K1 , size_t K2,
                         FaceDir::DirEnum faceDir)
        : m_faceDir( faceDir )
    {
        checkCoord(nx , I1,I2);
        checkCoord(ny , J1,J2);
        checkCoord(nz , K1,K2);


        if ((faceDir == FaceDir::XPlus) || (faceDir == FaceDir::XMinus))
            if (I1 != I2)
                throw std::invalid_argument("When the face is in X direction we must have I1 == I2");

        if ((faceDir == FaceDir::YPlus) || (faceDir == FaceDir::YMinus))
            if (J1 != J2)
                throw std::invalid_argument("When the face is in Y direction we must have J1 == J2");

        if ((faceDir == FaceDir::ZPlus) || (faceDir == FaceDir::ZMinus))
            if (K1 != K2)
                throw std::invalid_argument("When the face is in Z direction we must have K1 == K2");


        for (size_t k=K1; k <= K2; k++)
            for (size_t j=J1; j <= J2; j++)
                for (size_t i=I1; i <= I2; i++) {
                    size_t globalIndex = i + j*nx + k*nx*ny;
                    m_indexList.push_back( globalIndex );
                }
    }

    FaultFace FaultFace::serializeObject()
    {
        FaultFace result;
        result.m_faceDir = FaceDir::YPlus;
        result.m_indexList = {1,2,3,4,5,6};

        return result;
    }

    void FaultFace::checkCoord(size_t dim , size_t l1 , size_t l2) {
        if (l1 > l2)
            throw std::invalid_argument("Invalid coordinates");

        if (l2 >= dim)
            throw std::invalid_argument("Invalid coordinates");
    }


    std::vector<size_t>::const_iterator FaultFace::begin() const {
        return m_indexList.begin();
    }

    std::vector<size_t>::const_iterator FaultFace::end() const {
        return m_indexList.end();
    }


    FaceDir::DirEnum FaultFace::getDir() const {
        return m_faceDir;
    }

    bool FaultFace::operator==( const FaultFace& rhs ) const {
        return this->m_faceDir == rhs.m_faceDir
            && this->m_indexList.size() == rhs.m_indexList.size()
            && std::equal( this->m_indexList.begin(),
                           this->m_indexList.end(),
                           rhs.m_indexList.begin() );
    }

    bool FaultFace::operator!=( const FaultFace& rhs ) const {
        return !( *this == rhs );
    }
}
