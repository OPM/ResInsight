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

#include "RiaStimPlanModelDefines.h"

#include "RimCheckableNamedObject.h"
#include "RimExtractionConfiguration.h"
#include "RimPerforationInterval.h"
#include "RimWellPathComponentInterface.h"

#include "Well/RigWellLogExtractor.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimWellPath;
class RimModeledWellPath;
class RimElasticProperties;
class RigEclipseCaseData;
class RimAnnotationCollectionBase;
class RimUserDefinedPolylinesAnnotation;
class RimFaciesProperties;
class RimStimPlanModelTemplate;
class RimTextAnnotation;
class RimStimPlanModelCalculator;
class RimColorLegend;

#include <deque>

//==================================================================================================
///
///
//==================================================================================================
class RimStimPlanModel : public RimCheckableNamedObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ExtractionType
    {
        TRUE_VERTICAL_THICKNESS,
        TRUE_STRATIGRAPHIC_THICKNESS,
    };

    enum class FractureOrientation
    {
        ALONG_WELL_PATH,
        TRANSVERSE_WELL_PATH,
        AZIMUTH
    };

    enum class MissingValueStrategy
    {
        DEFAULT_VALUE,
        LINEAR_INTERPOLATION,
        OTHER_CURVE_PROPERTY,
        CELLS_ABOVE,
        CELLS_BELOW
    };

    enum class BurdenStrategy
    {
        DEFAULT_VALUE,
        GRADIENT
    };

    RimStimPlanModel();
    ~RimStimPlanModel() override;

    void setMD( double md );

    int  timeStep() const;
    void setTimeStep( int timeStep );

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCaseForProperty( RiaDefines::CurveProperty curveProperty ) const;

    void setEclipseCaseAndTimeStep( RimEclipseCase* eclipseCase, int timeStep );

    cvf::Vec3d anchorPosition() const;
    cvf::Vec3d thicknessDirection() const;

    void       fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    cvf::Vec3d fracturePosition() const;

    double defaultPorosity() const;
    double defaultPermeability() const;
    double defaultFaciesValue() const;

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

    double              perforationLength() const;
    FractureOrientation fractureOrientation() const;

    double formationDip() const;
    bool   hasBarrier() const;
    double distanceToBarrier() const;
    double barrierDip() const;
    int    wellPenetrationLayer() const;

    // RimWellPathCompletionsInterface overrides.
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;
    void                              applyOffset( double offsetMD ) override;

    bool isEnabled() const override;

    RimWellPath* wellPath() const;

    void loadDataAndUpdate();
    void resetAnchorPositionAndThicknessDirection();

    RimModeledWellPath* thicknessDirectionWellPath() const;
    void                setThicknessDirectionWellPath( RimModeledWellPath* thicknessDirectionWellPath );

    double getDefaultValueForProperty( RiaDefines::CurveProperty ) const;
    bool   hasDefaultValueForProperty( RiaDefines::CurveProperty ) const;

    RiaDefines::CurveProperty getDefaultPropertyForMissingValues( RiaDefines::CurveProperty curveProperty ) const;
    double                    getDefaultForMissingOverburdenValue( RiaDefines::CurveProperty curveProperty ) const;
    double                    getDefaultForMissingUnderburdenValue( RiaDefines::CurveProperty curveProperty ) const;
    double                    getDefaultForMissingValue( RiaDefines::CurveProperty curveProperty ) const;
    double                    getOverburdenGradient( RiaDefines::CurveProperty curveProperty ) const;
    double                    getUnderburdenGradient( RiaDefines::CurveProperty curveProperty ) const;

    void                      setStimPlanModelTemplate( RimStimPlanModelTemplate* stimPlanModelTemplate );
    RimStimPlanModelTemplate* stimPlanModelTemplate() const;

    void updateReferringPlots();

    std::shared_ptr<RimStimPlanModelCalculator> calculator() const;

    std::deque<RimStimPlanModel::MissingValueStrategy> missingValueStrategies( RiaDefines::CurveProperty curveProperty ) const;
    RimStimPlanModel::BurdenStrategy                   burdenStrategy( RiaDefines::CurveProperty curveProperty ) const;
    RiaDefines::ResultCatType                          eclipseResultCategory( RiaDefines::CurveProperty curveProperty ) const;
    QString                                            eclipseResultVariable( RiaDefines::CurveProperty curveProperty ) const;

    std::deque<RimExtractionConfiguration> extractionConfigurations( RiaDefines::CurveProperty curveProperty ) const;
    RimEclipseCase*                        eclipseCaseForType( RimExtractionConfiguration::EclipseCaseType ) const;

    static double findFaciesValue( const RimColorLegend& colorLegend, const QString& name );

    bool isScaledByNetToGross( RiaDefines::CurveProperty curveProperty ) const;

    QString pressureDate() const;

    QString unitForProperty( RiaDefines::CurveProperty curveProperty ) const;

    static cvf::Vec3d
        projectVectorIntoFracturePlane( const cvf::Vec3d& position, const cvf::Vec3d& fractureDirectionNormal, const cvf::Vec3d& direction );

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void initAfterRead() override;

private:
    void updatePositionFromMeasuredDepth();
    void updateThicknessDirection();
    void updateDistanceToBarrierAndDip();
    void updateThicknessDirectionWellPathName();
    void updatePerforationInterval();

    cvf::Vec3d computeFractureDirectionNormal( RimWellPath* wellPath, const cvf::Vec3d& position ) const;

    RigEclipseCaseData* getEclipseCaseData() const;

    void updateBarrierProperties();
    void addBarrierAnnotation( const cvf::Vec3d& startPosition, const cvf::Vec3d& endPosition, const QString& barrierText );
    void clearBarrierAnnotation();
    RimAnnotationCollectionBase* annotationCollection();

    void updateViewsAndPlots();
    void stimPlanModelTemplateChanged( const caf::SignalEmitter* emitter );

    void hideOtherFaults( const QString& targetFaultName );
    void showAllFaults();

    RimColorLegend* getFaciesColorLegend() const;
    void            updateExtractionDepthBoundaries();

    static bool useStaticEclipseCase( RiaDefines::CurveProperty curveProperty );

    cvf::Vec3d anchorPositionForUi() const;

protected:
    caf::PdmField<double>                       m_MD;
    caf::PdmPtrField<RimEclipseCase*>           m_eclipseCase;
    caf::PdmField<int>                          m_timeStep;
    caf::PdmPtrField<RimEclipseCase*>           m_initialPressureEclipseCase;
    caf::PdmPtrField<RimEclipseCase*>           m_staticEclipseCase;
    caf::PdmField<caf::AppEnum<ExtractionType>> m_extractionType;
    caf::PdmField<double>                       m_extractionOffsetTop;
    caf::PdmField<double>                       m_extractionOffsetBottom;
    caf::PdmField<double>                       m_extractionDepthTop;
    caf::PdmField<double>                       m_extractionDepthBottom;
    caf::PdmField<cvf::Vec3d>                   m_anchorPosition;
    caf::PdmProxyValueField<cvf::Vec3d>         m_anchorPositionForUi;
    caf::PdmField<cvf::Vec3d>                   m_thicknessDirection;
    caf::PdmField<cvf::Vec3d>                   m_originalThicknessDirection;
    caf::PdmField<double>                       m_boundingBoxVertical;
    caf::PdmField<double>                       m_boundingBoxHorizontal;
    caf::PdmPtrField<RimModeledWellPath*>       m_thicknessDirectionWellPath;
    caf::PdmField<double>                       m_relativePermeabilityFactorDefault;
    caf::PdmField<double>                       m_poroElasticConstantDefault;
    caf::PdmField<double>                       m_thermalExpansionCoeffientDefault;
    caf::PdmField<bool>                         m_useDetailedFluidLoss;

    caf::PdmPtrField<RimStimPlanModelTemplate*> m_stimPlanModelTemplate;
    caf::PdmField<bool>                         m_editStimPlanModelTemplate;

    caf::PdmField<caf::AppEnum<FractureOrientation>> m_fractureOrientation;
    caf::PdmField<double>                            m_azimuthAngle;
    caf::PdmField<double>                            m_perforationLength;

    caf::PdmField<double>                     m_formationDip;
    caf::PdmField<bool>                       m_autoComputeBarrier;
    caf::PdmField<bool>                       m_hasBarrier;
    caf::PdmField<double>                     m_distanceToBarrier;
    caf::PdmField<double>                     m_barrierDip;
    caf::PdmField<int>                        m_wellPenetrationLayer;
    caf::PdmPtrField<RimTextAnnotation*>      m_barrierTextAnnotation;
    caf::PdmField<QString>                    m_barrierFaultName;
    caf::PdmField<bool>                       m_showOnlyBarrierFault;
    caf::PdmField<bool>                       m_showAllFaults;
    caf::PdmPtrField<RimPerforationInterval*> m_perforationInterval;

    std::shared_ptr<RimStimPlanModelCalculator> m_calculator;
};
