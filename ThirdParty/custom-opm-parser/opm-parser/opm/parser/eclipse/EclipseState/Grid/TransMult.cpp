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

#include <opm/parser/eclipse/EclipseState/Grid/Fault.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultFace.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaultCollection.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/TransMult.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/MULTREGTScanner.hpp>


namespace Opm {

    TransMult::TransMult(size_t nx , size_t ny , size_t nz) :
        m_nx(nx),
        m_ny(ny),
        m_nz(nz)
    {
        m_names[FaceDir::XPlus]  = "MULTX";
        m_names[FaceDir::YPlus]  = "MULTY";
        m_names[FaceDir::ZPlus]  = "MULTZ";
        m_names[FaceDir::XMinus] = "MULTX-";
        m_names[FaceDir::YMinus] = "MULTY-";
        m_names[FaceDir::ZMinus] = "MULTZ-";
    }



    void TransMult::assertIJK(size_t i , size_t j , size_t k) const {
        if ((i >= m_nx) || (j >= m_ny) || (k >= m_nz))
            throw std::invalid_argument("Invalid ijk");
    }


    size_t TransMult::getGlobalIndex(size_t i , size_t j , size_t k) const {
        assertIJK(i,j,k);
        return i + j*m_nx + k*m_nx*m_ny;
    }


    double TransMult::getMultiplier(size_t globalIndex,  FaceDir::DirEnum faceDir) const {
        if (globalIndex < m_nx * m_ny * m_nz)
            return getMultiplier__(globalIndex , faceDir);
        else
            throw std::invalid_argument("Invalid global index");
    }

    double TransMult::getMultiplier__(size_t globalIndex,  FaceDir::DirEnum faceDir) const {
        if (hasDirectionProperty( faceDir )) {
            return m_trans.at(faceDir).iget( globalIndex );
        } else
            return 1.0;
    }




    double TransMult::getMultiplier(size_t i , size_t j , size_t k, FaceDir::DirEnum faceDir) const {
        size_t globalIndex = getGlobalIndex(i,j,k);
        return getMultiplier__( globalIndex , faceDir );
    }

    double TransMult::getRegionMultiplier(size_t globalCellIndex1,  size_t globalCellIndex2, FaceDir::DirEnum faceDir) const {
        if (m_multregtScanner == nullptr)
            throw new std::logic_error("MULTREGTScanner has not been initialized.");
        return m_multregtScanner->getRegionMultiplier(globalCellIndex1, globalCellIndex2, faceDir);
    }

    bool TransMult::hasDirectionProperty(FaceDir::DirEnum faceDir) const {
        return m_trans.count(faceDir) == 1;
    }

    void TransMult::insertNewProperty(FaceDir::DirEnum faceDir) {
        GridPropertySupportedKeywordInfo<double> kwInfo(m_names[faceDir] , 1.0 , "1");
        GridProperty< double > prop( m_nx, m_ny, m_nz, kwInfo );
        m_trans.emplace( faceDir, std::move( prop ) );
    }


    GridProperty<double>& TransMult::getDirectionProperty(FaceDir::DirEnum faceDir) {
        if (m_trans.count(faceDir) == 0)
            insertNewProperty(faceDir);

        return m_trans.at( faceDir );
    }

    void TransMult::applyMULT(const GridProperty<double>& srcProp, FaceDir::DirEnum faceDir)
    {
        auto& dstProp = getDirectionProperty(faceDir);

        const std::vector<double> &srcData = srcProp.getData();
        for (size_t i = 0; i < srcData.size(); ++i)
            dstProp.multiplyValueAtIndex(i, srcData[i]);
    }


    void TransMult::applyMULTFLT(const Fault& fault) {
        double transMult = fault.getTransMult();

        for (auto face_iter = fault.begin(); face_iter != fault.end(); ++face_iter) {
            std::shared_ptr<const FaultFace> face = *face_iter;
            FaceDir::DirEnum faceDir = face->getDir();
            auto& multProperty = getDirectionProperty(faceDir);

            for (auto cell_iter = face->begin(); cell_iter != face->end(); ++cell_iter) {
                size_t globalIndex = *cell_iter;
                multProperty.multiplyValueAtIndex( globalIndex , transMult);
            }
        }
    }


    void TransMult::applyMULTFLT(const FaultCollection& faults) {
        for (size_t faultIndex = 0; faultIndex < faults.size(); faultIndex++) {
            auto& fault = faults.getFault(faultIndex);
            applyMULTFLT(fault);
        }
    }



    void TransMult::createMultregtScanner(const Eclipse3DProperties& e3DProps,
                                          std::vector< const DeckKeyword* > multregtKeywords) {
        m_multregtScanner = new MULTREGTScanner(e3DProps, multregtKeywords);
    }
}
