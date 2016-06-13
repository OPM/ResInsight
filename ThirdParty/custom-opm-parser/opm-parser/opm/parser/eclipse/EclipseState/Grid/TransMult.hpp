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

/**
   This class implements a small container which holds the
   transmissibility mulitpliers for all the faces in the grid. The
   multipliers in this class are built up from the transmissibility
   modifier keywords:

      {MULTX , MULTX- , MULTY , MULTY- , MULTZ , MULTZ-, MULTFLT , MULTREGT}

*/
#ifndef TRANSMULT_HPP
#define TRANSMULT_HPP


#include <cstddef>
#include <map>
#include <memory>

#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>


namespace Opm {
    template< typename > class GridProperty;
    class Fault;
    class FaultCollection;
    class MULTREGTScanner;

    class TransMult {

    public:
        TransMult(size_t nx , size_t ny , size_t nz);
        double getMultiplier(size_t globalIndex, FaceDir::DirEnum faceDir) const;
        double getMultiplier(size_t i , size_t j , size_t k, FaceDir::DirEnum faceDir) const;
        double getRegionMultiplier( size_t globalCellIndex1, size_t globalCellIndex2, FaceDir::DirEnum faceDir) const;
        void applyMULT(const GridProperty<double>& srcMultProp, FaceDir::DirEnum faceDir);
        void applyMULTFLT(const FaultCollection& faults);
        void applyMULTFLT(const Fault& fault);
        void setMultregtScanner(std::shared_ptr<const MULTREGTScanner> multregtScanner);

    private:
        size_t getGlobalIndex(size_t i , size_t j , size_t k) const;
        void assertIJK(size_t i , size_t j , size_t k) const;
        double getMultiplier__(size_t globalIndex , FaceDir::DirEnum faceDir) const;
        void insertNewProperty(FaceDir::DirEnum faceDir);
        bool hasDirectionProperty(FaceDir::DirEnum faceDir) const;
        GridProperty<double>& getDirectionProperty(FaceDir::DirEnum faceDir);

        size_t m_nx , m_ny , m_nz;
        std::map<FaceDir::DirEnum , GridProperty<double> > m_trans;
        std::map<FaceDir::DirEnum , std::string> m_names;
        std::shared_ptr<const MULTREGTScanner> m_multregtScanner;
    };

}

#endif
