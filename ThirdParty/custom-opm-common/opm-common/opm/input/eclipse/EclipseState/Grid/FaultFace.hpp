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
#ifndef OPM_PARSER_FAULT_FACE_HPP
#define OPM_PARSER_FAULT_FACE_HPP

#include <cstddef>
#include <vector>

#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>

namespace Opm {


class FaultFace {
public:
    FaultFace() = default;
    FaultFace(size_t nx , size_t ny , size_t nz,
              size_t I1 , size_t I2,
              size_t J1 , size_t J2,
              size_t K1 , size_t K2,
              FaceDir::DirEnum faceDir);

    static FaultFace serializeObject();

    std::vector<size_t>::const_iterator begin() const;
    std::vector<size_t>::const_iterator end() const;
    FaceDir::DirEnum getDir() const;

    bool operator==( const FaultFace& rhs ) const;
    bool operator!=( const FaultFace& rhs ) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_faceDir);
        serializer(m_indexList);
    }

private:
    static void checkCoord(size_t dim , size_t l1 , size_t l2);
    FaceDir::DirEnum m_faceDir = FaceDir::XPlus;
    std::vector<size_t> m_indexList;
};


}

#endif // OPM_PARSER_FAULT_FACE_HPP
