/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiaEclipseUnitTools.h"
#include "RiaFractureModelDefines.h"

#include "RimCheckableNamedObject.h"
#include "RimWellPathComponentInterface.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimWellPath;
class RimModeledWellPath;
class RimElasticProperties;
class RigEclipseCaseData;

//==================================================================================================
///
///
//==================================================================================================
class RimFractureModel : public RimCheckableNamedObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ExtractionType
    {
        TRUE_VERTICAL_THICKNESS,
        TRUE_STRATIGRAPHIC_THICKNESS,
    };

    RimFractureModel( void );
    ~RimFractureModel( void ) override;

    void setMD( double md );

    cvf::Vec3d anchorPosition() const;
    cvf::Vec3d thicknessDirection() const;

    void       fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    cvf::Vec3d fracturePosition() const;

    double defaultPorosity() const;
    double defaultPermeability() const;

    double verticalStress() const;
    double verticalStressGradient() const;
    double stressDepth() const;

    double overburdenHeight() const;
    double underburdenHeight() const;

    double defaultOverburdenPorosity() const;
    double defaultUnderburdenPorosity() const;

    double defaultOverburdenPermeability() const;
    double defaultUnderburdenPermeability() const;

    QString overburdenFormation() const;
    QString overburdenFacies() const;

    QString underburdenFormation() const;
    QString underburdenFacies() const;

    double referenceTemperature() const;
    double referenceTemperatureGradient() const;
    double referenceTemperatureDepth() const;

    bool useDetailedFluidLoss() const;

    // RimWellPathCompletionsInterface overrides.
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;
    bool                              isEnabled() const override;

    RimWellPath* wellPath() const;

    void loadDataAndUpdate();

    RimModeledWellPath*   thicknessDirectionWellPath() const;
    void                  setThicknessDirectionWellPath( RimModeledWellPath* thicknessDirectionWellPath );
    void                  setElasticProperties( RimElasticProperties* elasticProperties );
    RimElasticProperties* elasticProperties() const;

    double getDefaultValueForProperty( RiaDefines::CurveProperty ) const;
    bool   hasDefaultValueForProperty( RiaDefines::CurveProperty ) const;

    RiaDefines::CurveProperty getDefaultPropertyForMissingValues( const QString& keyword ) const;
    double                    getDefaultForMissingOverburdenValue( const QString& keyword ) const;
    double                    getDefaultForMissingUnderburdenValue( const QString& keyword ) const;
    double                    getDefaultForMissingValue( const QString& keyword ) const;
    double                    getOverburdenGradient( const QString& keyword ) const;
    double                    getUnderburdenGradient( const QString& keyword ) const;

    void updateReferringPlots();

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    void           updatePositionFromMeasuredDepth();
    void           updateThicknessDirection();
    cvf::Vec3d     calculateTSTDirection() const;
    void           findThicknessTargetPoints( cvf::Vec3d& topPosition, cvf::Vec3d& bottomPosition );
    static QString vecToString( const cvf::Vec3d& vec );
    void           updateThicknessDirectionWellPathName();
    static double  computeDefaultStressDepth();

    static RigEclipseCaseData* getEclipseCaseData();
    static RimEclipseCase*     getEclipseCase();

protected:
    caf::PdmField<double>                       m_MD;
    caf::PdmField<caf::AppEnum<ExtractionType>> m_extractionType;
    caf::PdmField<cvf::Vec3d>                   m_anchorPosition;
    caf::PdmField<cvf::Vec3d>                   m_thicknessDirection;
    caf::PdmField<double>                       m_boundingBoxVertical;
    caf::PdmField<double>                       m_boundingBoxHorizontal;
    caf::PdmPtrField<RimModeledWellPath*>       m_thicknessDirectionWellPath;
    caf::PdmChildField<RimElasticProperties*>   m_elasticProperties;
    caf::PdmField<double>                       m_defaultPorosity;
    caf::PdmField<double>                       m_defaultPermeability;
    caf::PdmField<double>                       m_verticalStress;
    caf::PdmField<double>                       m_verticalStressGradient;
    caf::PdmField<double>                       m_stressDepth;
    caf::PdmField<double>                       m_overburdenHeight;
    caf::PdmField<double>                       m_overburdenPorosity;
    caf::PdmField<double>                       m_overburdenPermeability;
    caf::PdmField<QString>                      m_overburdenFormation;
    caf::PdmField<QString>                      m_overburdenFacies;
    caf::PdmField<double>                       m_overburdenFluidDensity;
    caf::PdmField<double>                       m_underburdenHeight;
    caf::PdmField<double>                       m_underburdenPorosity;
    caf::PdmField<double>                       m_underburdenPermeability;
    caf::PdmField<QString>                      m_underburdenFormation;
    caf::PdmField<QString>                      m_underburdenFacies;
    caf::PdmField<double>                       m_underburdenFluidDensity;
    caf::PdmField<double>                       m_referenceTemperature;
    caf::PdmField<double>                       m_referenceTemperatureGradient;
    caf::PdmField<double>                       m_referenceTemperatureDepth;
    caf::PdmField<double>                       m_relativePermeabilityFactorDefault;
    caf::PdmField<double>                       m_poroElasticConstantDefault;
    caf::PdmField<double>                       m_thermalExpansionCoeffientDefault;
    caf::PdmField<bool>                         m_useDetailedFluidLoss;
};
