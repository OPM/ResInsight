/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -    Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RimFaultReactivationDataAccessorStress.h"
#include "RimFaultReactivationEnums.h"

#include "RigFemResultAddress.h"

#include <vector>

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RigFemPart;
class RigFemPartResultsCollection;
class RigFemScalarResultFrames;
class RigGeoMechCaseData;
class RigGriddedPart3d;
class RimGeoMechCase;
class RimWellIADataAccess;
class RimModeledWellPath;
class RigWellPath;
class RigFemPartCollection;
class RigGeoMechWellLogExtractor;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorStressGeoMech : public RimFaultReactivationDataAccessorStress
{
public:
    RimFaultReactivationDataAccessorStressGeoMech( RimGeoMechCase*                geoMechCase,
                                                   RimFaultReactivation::Property property,
                                                   double                         gradient,
                                                   double                         seabedDepth );
    ~RimFaultReactivationDataAccessorStressGeoMech() override;

private:
    void updateResultAccessor() override;

    bool isDataAvailable() const override;

    double extractStressValue( StressType stressType, const cvf::Vec3d& position ) const override;

    std::pair<double, cvf::Vec3d>
        calculatePorBar( const cvf::Vec3d& position, double gradient, RimFaultReactivation::GridPart gridPart ) const override;

    bool isPositionValid( const cvf::Vec3d&              position,
                          const cvf::Vec3d&              topPosition,
                          const cvf::Vec3d&              bottomPosition,
                          RimFaultReactivation::GridPart gridPart ) const override;

    static RigFemResultAddress getResultAddress( const std::string& fieldName, const std::string& componentName );

    double interpolatedResultValue( RimWellIADataAccess&      iaDataAccess,
                                    const RigFemPart*         femPart,
                                    const cvf::Vec3d&         position,
                                    const std::vector<float>& scalarResults ) const;

    std::pair<double, cvf::Vec3d> calculatePorBar( const cvf::Vec3d& position, double gradient, int timeStepIndex, int frameIndex ) const;

    RigFemScalarResultFrames* dataFrames( StressType stressType ) const;

    RimGeoMechCase*           m_geoMechCase;
    RigGeoMechCaseData*       m_geoMechCaseData;
    RigFemScalarResultFrames* m_s11Frames;
    RigFemScalarResultFrames* m_s22Frames;
    RigFemScalarResultFrames* m_s33Frames;
    const RigFemPart*         m_femPart;

    cvf::ref<RigWellPath>                m_faceAWellPath;
    cvf::ref<RigWellPath>                m_faceBWellPath;
    int                                  m_partIndexA;
    int                                  m_partIndexB;
    cvf::ref<RigGeoMechWellLogExtractor> m_extractorA;
    cvf::ref<RigGeoMechWellLogExtractor> m_extractorB;
};
