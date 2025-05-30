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

#include "RiaDefines.h"

#include "Rim3dPropertiesInterface.h"
#include "RimCheckableNamedObject.h"
#include "RimFishbonesDefines.h"
#include "RimWellPathComponentInterface.h"

#include "cvfColor3.h"
#include "cvfVector3.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmProxyValueField.h"

#include <algorithm>
#include <memory>

class RigFisbonesGeometry;
class RimFishbonesPipeProperties;
class RimMultipleValveLocations;

//==================================================================================================
///
///
//==================================================================================================
class RimFishbones : public caf::PdmObject, public Rim3dPropertiesInterface, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    using SubAndLateralIndex = std::pair<size_t, size_t>;

public:
    RimFishbones();
    ~RimFishbones() override;

    bool    isActive() const;
    QString generatedName() const;

    void setMeasuredDepthAndCount( double startMD, double spacing, int subCount );
    void setValveLocations( const std::vector<double>& measuredDepths );

    void setSystemParameters( int lateralsPerSub, double lateralLength, double holeDiameter, double buildAngle, int icdsPerSub );

    double measuredDepth( size_t subIndex ) const;
    double rotationAngle( size_t subIndex ) const;

    double exitAngle() const;
    double buildAngle() const;

    double              tubingDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double              holeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double              equivalentDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double              skinFactor() const;
    double              openHoleRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double              icdOrificeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double              icdFlowCoefficient() const;
    size_t              icdCount() const;
    std::vector<double> lateralLengths() const;

    void geometryUpdated();

    const std::vector<SubAndLateralIndex>&     installedLateralIndices() const;
    std::vector<cvf::Vec3d>                    coordsForLateral( size_t subIndex, size_t lateralIndex ) const;
    std::vector<std::pair<cvf::Vec3d, double>> coordsAndMDForLateral( size_t subIndex, size_t lateralIndex ) const;
    void                                       recomputeLateralLocations();

    void setUnitSystemSpecificDefaults();

    // Override from Rim3dPropertiesInterface
    cvf::BoundingBox boundingBoxInDomainCoords() const override;

    // Overrides from RimWellPathCompletionsInterface
    bool                              isEnabled() const override;
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;
    void                              applyOffset( double offsetMD ) override;

public:
    caf::PdmField<cvf::Color3f> fishbonesColor;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

private:
    void computeRangesAndLocations();
    void computeRotationAngles();
    void computeSubLateralIndices();

    void initialiseObsoleteFields();
    void initValveLocationFromLegacyData();

private:
    caf::PdmField<bool>              m_isActive;
    caf::PdmProxyValueField<QString> m_name;

    caf::PdmField<int>     m_lateralCountPerSub;
    caf::PdmField<QString> m_lateralLength;

    caf::PdmField<double> m_lateralExitAngle;
    caf::PdmField<double> m_lateralBuildAngle;

    caf::PdmField<double> m_lateralTubingDiameter;

    caf::PdmField<double> m_lateralOpenHoleRoghnessFactor;
    caf::PdmField<double> m_lateralTubingRoghnessFactor;

    caf::PdmField<double> m_lateralInstallSuccessFraction;

    caf::PdmField<int>    m_icdCount;
    caf::PdmField<double> m_icdOrificeDiameter;
    caf::PdmField<double> m_icdFlowCoefficient;

    caf::PdmChildField<RimMultipleValveLocations*>                            m_valveLocations;
    caf::PdmField<caf::AppEnum<RimFishbonesDefines::LateralsOrientationType>> m_subsOrientationMode;

    caf::PdmField<std::vector<double>> m_installationRotationAngles;
    caf::PdmField<double>              m_fixedInstallationRotationAngle;

    caf::PdmChildField<RimFishbonesPipeProperties*> m_pipeProperties;

    caf::PdmProxyValueField<double> m_lateralDiameter;
    caf::PdmProxyValueField<double> m_lateralSkinFactor;

    caf::PdmField<uint> m_randomSeed;

    std::unique_ptr<RigFisbonesGeometry> m_rigFishbonesGeometry;
    std::vector<SubAndLateralIndex>      m_subLateralIndices;

    // Moved to RimMultipleValveLocations
    caf::PdmField<caf::AppEnum<RimFishbonesDefines::LocationType_OBSOLETE>> m_subsLocationMode_OBSOLETE;
    caf::PdmField<double>                                                   m_rangeStart_OBSOLETE;
    caf::PdmField<double>                                                   m_rangeEnd_OBSOLETE;
    caf::PdmField<double>                                                   m_rangeSubSpacing_OBSOLETE;
    caf::PdmField<int>                                                      m_rangeSubCount_OBSOLETE;
    caf::PdmField<std::vector<double>>                                      m_locationOfSubs_OBSOLETE; // Given in measured depth
};
