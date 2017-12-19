/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimCheckableNamedObject.h"
#include "Rim3dPropertiesInterface.h"
#include "RimFishbonesPipeProperties.h"

#include "RiaEclipseUnitTools.h"

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cvfColor3.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"
#include "cafPdmChildField.h"

#include <algorithm>
#include <memory>

class RigFisbonesGeometry;

//==================================================================================================
///  
///  
//==================================================================================================
struct SubLateralIndex {
    size_t              subIndex;
    std::vector<size_t> lateralIndices;
};

//==================================================================================================
///  
///  
//==================================================================================================
class RimFishbonesMultipleSubs : public RimCheckableNamedObject, public Rim3dPropertiesInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum LocationType
    {
        FB_SUB_COUNT_END,
        FB_SUB_SPACING_END,
        FB_SUB_USER_DEFINED
    };

    enum LateralsOrientationType
    {
        FB_LATERAL_ORIENTATION_FIXED,
        FB_LATERAL_ORIENTATION_RANDOM
    };

public:
    RimFishbonesMultipleSubs();
    virtual ~RimFishbonesMultipleSubs();


    void                setMeasuredDepthAndCount(double measuredDepth, double spacing, int subCount);

    double              measuredDepth(size_t subIndex) const;
    double              rotationAngle(size_t subIndex) const;

    double              exitAngle() const;
    double              buildAngle() const;

    double              tubingDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const;
    double              holeDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const { return m_pipeProperties()->holeDiameter(unitSystem); }
    double              skinFactor() const { return m_pipeProperties()->skinFactor(); }
    double              openHoleRoughnessFactor(RiaEclipseUnitTools::UnitSystem unitSystem) const;
    double              icdOrificeDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const;
    double              icdFlowCoefficient() const { return m_icdFlowCoefficient(); }
    size_t              icdCount() const { return m_icdCount(); }
    std::vector<double> lateralLengths() const;

    const std::vector<SubLateralIndex>&         installedLateralIndices() const { return m_subLateralIndices; };
    std::vector<cvf::Vec3d>                     coordsForLateral(size_t subIndex, size_t lateralIndex) const;
    std::vector<std::pair<cvf::Vec3d, double>>  coordsAndMDForLateral(size_t subIndex, size_t lateralIndex) const;
    void                                        recomputeLateralLocations();

    void                                        setUnitSystemSpecificDefaults();
    
    // Override from Rim3dPropertiesInterface
    virtual cvf::BoundingBox boundingBoxInDomainCoords() override;

public:
    caf::PdmField<cvf::Color3f>         fishbonesColor;

protected:
    virtual void        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    virtual void        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void        initAfterRead() override;

private:
    void                        computeRangesAndLocations();
    void                        computeRotationAngles();
    void                        computeSubLateralIndices();

    static std::vector<double>  locationsFromStartSpacingAndCount(double start, double spacing, size_t count);
    static int                  randomValueFromRange(int min, int max);
    
private:
    caf::PdmField<int>                  m_lateralCountPerSub;
    caf::PdmField<QString>              m_lateralLength;

    caf::PdmField<double>               m_lateralExitAngle;
    caf::PdmField<double>               m_lateralBuildAngle;

    caf::PdmField<double>               m_lateralTubingDiameter;

    caf::PdmField<double>               m_lateralOpenHoleRoghnessFactor;
    caf::PdmField<double>               m_lateralTubingRoghnessFactor;

    caf::PdmField<double>               m_lateralInstallSuccessFraction;

    caf::PdmField<int>                  m_icdCount;
    caf::PdmField<double>               m_icdOrificeDiameter;
    caf::PdmField<double>               m_icdFlowCoefficient;

    caf::PdmField<caf::AppEnum<LocationType> >    m_subsLocationMode;
    caf::PdmField<double>               m_rangeStart;
    caf::PdmField<double>               m_rangeEnd;
    caf::PdmField<double>               m_rangeSubSpacing;
    caf::PdmField<int>                  m_rangeSubCount;

    caf::PdmField<caf::AppEnum<LateralsOrientationType> >    m_subsOrientationMode;

    caf::PdmField<std::vector<double>>  m_locationOfSubs; // Given in measured depth

    caf::PdmField<std::vector<double>>  m_installationRotationAngles;
    caf::PdmField<double>               m_fixedInstallationRotationAngle;

    caf::PdmChildField<RimFishbonesPipeProperties*> m_pipeProperties;

    caf::PdmField<uint>                 m_randomSeed;

    std::unique_ptr<RigFisbonesGeometry>    m_rigFishbonesGeometry;
    std::vector<SubLateralIndex>            m_subLateralIndices;
};
