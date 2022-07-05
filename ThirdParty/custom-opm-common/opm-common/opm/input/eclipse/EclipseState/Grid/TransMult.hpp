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
#ifndef OPM_PARSER_TRANSMULT_HPP
#define OPM_PARSER_TRANSMULT_HPP


#include <cstddef>
#include <map>
#include <memory>

#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/input/eclipse/EclipseState/Grid/MULTREGTScanner.hpp>

namespace Opm {
    template< typename > class GridProperty;
    class Fault;
    class FaultCollection;
    class DeckKeyword;
    class FieldPropsManager;

    class TransMult {

    public:
        TransMult() = default;
        TransMult(const GridDims& dims, const Deck& deck, const FieldPropsManager& fp);

        static TransMult serializeObject();

        double getMultiplier(size_t globalIndex, FaceDir::DirEnum faceDir) const;
        double getMultiplier(size_t i , size_t j , size_t k, FaceDir::DirEnum faceDir) const;
        double getRegionMultiplier( size_t globalCellIndex1, size_t globalCellIndex2, FaceDir::DirEnum faceDir) const;
        void applyMULT(const std::vector<double>& srcMultProp, FaceDir::DirEnum faceDir);
        void applyMULTFLT(const FaultCollection& faults);
        void applyMULTFLT(const Fault& fault);

        bool operator==(const TransMult& data) const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_nx);
            serializer(m_ny);
            serializer(m_nz);
            // map used to avoid explicit instances with FaceDir::DirEnum in serializer
            serializer.template map<decltype(m_trans),false>(m_trans);
            serializer.template map<decltype(m_names),false>(m_names);
            m_multregtScanner.serializeOp(serializer);
        }

    private:
        size_t getGlobalIndex(size_t i , size_t j , size_t k) const;
        void assertIJK(size_t i , size_t j , size_t k) const;
        double getMultiplier__(size_t globalIndex , FaceDir::DirEnum faceDir) const;
        bool hasDirectionProperty(FaceDir::DirEnum faceDir) const;
        std::vector<double>& getDirectionProperty(FaceDir::DirEnum faceDir);

        size_t m_nx = 0, m_ny = 0, m_nz = 0;
        std::map<FaceDir::DirEnum , std::vector<double> > m_trans;
        std::map<FaceDir::DirEnum , std::string> m_names;
        MULTREGTScanner m_multregtScanner;
    };

}

#endif // OPM_PARSER_TRANSMULT_HPP
