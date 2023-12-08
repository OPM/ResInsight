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

#include "RimFaultReactivationDataAccessor.h"
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
class RimFaultReactivationDataAccessorStress : public RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessorStress( RimGeoMechCase* geoMechCase, RimFaultReactivation::Property property, double gradient );
    ~RimFaultReactivationDataAccessorStress();

    bool isMatching( RimFaultReactivation::Property property ) const override;

    double valueAtPosition( const cvf::Vec3d&                position,
                            const RigFaultReactivationModel& model,
                            RimFaultReactivation::GridPart   gridPart,
                            double                           topDepth     = std::numeric_limits<double>::infinity(),
                            double                           bottomDepth  = std::numeric_limits<double>::infinity(),
                            size_t                           elementIndex = std::numeric_limits<size_t>::max() ) const override;

private:
    void updateResultAccessor() override;

    static RigFemResultAddress getResultAddress( const std::string& fieldName, const std::string& componentName );

    double interpolatedResultValue( RimWellIADataAccess&      iaDataAccess,
                                    const RigFemPart*         femPart,
                                    const cvf::Vec3d&         position,
                                    const std::vector<float>& scalarResults ) const;

    std::pair<double, cvf::Vec3d> getPorBar( RimWellIADataAccess& iaDataAccess,
                                             const RigFemPart*    femPart,
                                             const cvf::Vec3d&    position,
                                             double               gradient,
                                             int                  timeStepIndex,
                                             int                  frameIndex ) const;

    static std::pair<bool, RimFaultReactivation::ElementSets>
        findElementSetContainingElement( const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets,
                                         unsigned int                                                                  elmIdx );

    static int                 getPartIndexFromPoint( const RigFemPartCollection& partCollection, const cvf::Vec3d& point );
    static std::pair<int, int> findIntersectionsForTvd( const std::vector<cvf::Vec3d>& intersections, double tvd );
    static std::pair<int, int> findOverburdenAndUnderburdenIndex( const std::vector<double>& values );
    static double              computePorBarWithGradient( const std::vector<cvf::Vec3d>& intersections,
                                                          const std::vector<double>&     values,
                                                          int                            i1,
                                                          int                            i2,
                                                          double                         gradient );
    static void fillInMissingValues( const std::vector<cvf::Vec3d>& intersections, std::vector<double>& values, double gradient );

    static std::vector<double> generateMds( const std::vector<cvf::Vec3d>& points );
    static std::vector<cvf::Vec3d>
        generateWellPoints( const cvf::Vec3d& faultTopPosition, const cvf::Vec3d& faultBottomPosition, const cvf::Vec3d& offset );

    RimGeoMechCase*                m_geoMechCase;
    RimFaultReactivation::Property m_property;
    double                         m_gradient;
    RigGeoMechCaseData*            m_geoMechCaseData;
    RigFemScalarResultFrames*      m_s11Frames;
    RigFemScalarResultFrames*      m_s22Frames;
    RigFemScalarResultFrames*      m_s33Frames;
    const RigFemPart*              m_femPart;

    cvf::ref<RigWellPath>                m_faceAWellPath;
    cvf::ref<RigWellPath>                m_faceBWellPath;
    int                                  m_partIndexA;
    int                                  m_partIndexB;
    cvf::ref<RigGeoMechWellLogExtractor> m_extractorA;
    cvf::ref<RigGeoMechWellLogExtractor> m_extractorB;
};
