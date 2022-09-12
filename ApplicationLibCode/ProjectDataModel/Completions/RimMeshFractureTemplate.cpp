/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "RimMeshFractureTemplate.h"

#include "RiaApplication.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigTransmissibilityEquations.h"
#include "RigWellPathStimplanIntersector.h"

#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiFilePathEditor.h"

#include <algorithm>
#include <cmath>
#include <vector>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimMeshFractureTemplate, "MeshFractureTemplate", "RimMeshFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeshFractureTemplate::RimMeshFractureTemplate()
{
    CAF_PDM_InitScriptableObject( "Fracture Template", ":/FractureTemplate16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_stimPlanFileName, "StimPlanFileName", "File Name" );
    m_stimPlanFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_userDefinedWellPathDepthAtFracture,
                       "UserDefinedWellPathDepthAtFracture",
                       false,
                       "User-Defined Well/Fracture Intersection Depth" );

    CAF_PDM_InitField( &m_borderPolygonResultName, "BorderPolygonResultName", QString( "" ), "Parameter" );
    m_borderPolygonResultName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_activeTimeStepIndex, "ActiveTimeStepIndex", 0, "Active TimeStep Index" );
    CAF_PDM_InitField( &m_conductivityResultNameOnFile,
                       "ConductivityResultName",
                       QString( "" ),
                       "Active Conductivity Result Name" );

    m_readError = false;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeshFractureTemplate::~RimMeshFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimMeshFractureTemplate::activeTimeStepIndex()
{
    return m_activeTimeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeshFractureTemplate::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue )
{
    RimFractureTemplate::fieldChangedByUi( changedField, oldValue, newValue );

    if ( &m_stimPlanFileName == changedField )
    {
        m_readError = false;
        loadDataAndUpdate();
        setDefaultsBasedOnFile();
    }

    if ( &m_userDefinedWellPathDepthAtFracture == changedField )
    {
        if ( !m_userDefinedWellPathDepthAtFracture )
        {
            m_readError = false;
            loadDataAndUpdate();
            RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }

    if ( &m_activeTimeStepIndex == changedField )
    {
        // Changes to this parameters should change all fractures with this fracture template attached.
        RimProject* proj = RimProject::current();
        for ( RimFracture* fracture : fracturesUsingThisTemplate() )
        {
            fracture->setStimPlanTimeIndexToPlot( m_activeTimeStepIndex );
        }
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    if ( &m_borderPolygonResultName == changedField || &m_activeTimeStepIndex == changedField ||
         &m_stimPlanFileName == changedField || &m_conductivityResultNameOnFile == changedField )
    {
        // Update fracture grid for all fractures using this template
        RimProject* proj = RimProject::current();
        for ( RimFracture* fracture : fracturesUsingThisTemplate() )
        {
            fracture->updateFractureGrid();
        }
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
    }

    if ( changedField == &m_scaleApplyButton )
    {
        m_scaleApplyButton = false;
        onLoadDataAndUpdateGeometryHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeshFractureTemplate::setFileName( const QString& fileName )
{
    m_stimPlanFileName = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMeshFractureTemplate::fileName()
{
    return m_stimPlanFileName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMeshFractureTemplate::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_fractureWidthType )
    {
        options.push_back(
            caf::PdmOptionItemInfo( caf::AppEnum<WidthEnum>::uiText( USER_DEFINED_WIDTH ), USER_DEFINED_WIDTH ) );

        if ( !widthResultValues().empty() )
        {
            options.push_back(
                caf::PdmOptionItemInfo( caf::AppEnum<WidthEnum>::uiText( WIDTH_FROM_FRACTURE ), WIDTH_FROM_FRACTURE ) );
        }
    }

    if ( fieldNeedingOptions == &m_betaFactorType )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<BetaFactorEnum>::uiText( USER_DEFINED_BETA_FACTOR ),
                                                   USER_DEFINED_BETA_FACTOR ) );

        if ( isBetaFactorAvailableOnFile() )
        {
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<BetaFactorEnum>::uiText( BETA_FACTOR_FROM_FRACTURE ),
                                                       BETA_FACTOR_FROM_FRACTURE ) );
        }
    }

    if ( fieldNeedingOptions == &m_borderPolygonResultName )
    {
        for ( std::pair<QString, QString> nameUnit : uiResultNamesWithUnit() )
        {
            options.push_back( caf::PdmOptionItemInfo( nameUnit.first, nameUnit.first ) );
        }
    }
    else if ( fieldNeedingOptions == &m_activeTimeStepIndex )
    {
        std::vector<QString> timeValues = timeStepsStrings();
        int                  index      = 0;
        for ( QString value : timeValues )
        {
            options.push_back( caf::PdmOptionItemInfo( value, index ) );
            index++;
        }
    }
    else if ( fieldNeedingOptions == &m_conductivityResultNameOnFile )
    {
        QStringList resultNames = conductivityResultNames();
        for ( const auto& resultName : resultNames )
        {
            options.push_back( caf::PdmOptionItemInfo( resultName, resultName ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellFractureIntersectionData RimMeshFractureTemplate::wellFractureIntersectionData( const RimFracture* fractureInstance ) const
{
    if ( !fractureInstance || !fractureInstance->fractureGrid() ) return {};

    WellFractureIntersectionData values;

    const RigFractureGrid* fractureGrid = fractureInstance->fractureGrid();
    if ( orientationType() == ALONG_WELL_PATH )
    {
        RimWellPath* rimWellPath = nullptr;
        fractureInstance->firstAncestorOrThisOfType( rimWellPath );

        if ( rimWellPath && rimWellPath->wellPathGeometry() )
        {
            double totalLength              = 0.0;
            double weightedConductivity     = 0.0;
            double weightedWidth            = 0.0;
            double weightedBetaFactorOnFile = 0.0;

            {
                std::vector<double> widthResultValues;
                {
                    auto nameUnit     = widthParameterNameAndUnit();
                    widthResultValues = fractureGridResultsForUnitSystem( nameUnit.first,
                                                                          nameUnit.second,
                                                                          m_activeTimeStepIndex,
                                                                          fractureTemplateUnit() );
                }

                std::vector<double> conductivityResultValues;
                {
                    auto nameUnit            = conductivityParameterNameAndUnit();
                    conductivityResultValues = fractureGridResultsForUnitSystem( nameUnit.first,
                                                                                 nameUnit.second,
                                                                                 m_activeTimeStepIndex,
                                                                                 fractureTemplateUnit() );
                }

                std::vector<double> betaFactorResultValues;
                {
                    auto nameUnit          = betaFactorParameterNameAndUnit();
                    betaFactorResultValues = fractureGridResults( nameUnit.first, nameUnit.second, m_activeTimeStepIndex );
                }

                RiaWeightedMeanCalculator<double>  widthCalc;
                RiaWeightedMeanCalculator<double>  conductivityCalc;
                RiaWeightedGeometricMeanCalculator betaFactorCalc;

                RigWellPathStimplanIntersector intersector( rimWellPath->wellPathGeometry(), fractureInstance );
                for ( const auto& v : intersector.intersections() )
                {
                    size_t fractureGlobalCellIndex = v.first;
                    double intersectionLength      = v.second.computeLength();

                    if ( fractureGlobalCellIndex < widthResultValues.size() )
                    {
                        widthCalc.addValueAndWeight( widthResultValues[fractureGlobalCellIndex], intersectionLength );
                    }

                    if ( fractureGlobalCellIndex < conductivityResultValues.size() )
                    {
                        conductivityCalc.addValueAndWeight( conductivityResultValues[fractureGlobalCellIndex],
                                                            intersectionLength );
                    }

                    if ( fractureGlobalCellIndex < betaFactorResultValues.size() )
                    {
                        double nativeBetaFactor = betaFactorResultValues[fractureGlobalCellIndex];

                        // Guard against zero beta values, as these values will set the geometric mean to zero
                        // Consider using the conductivity threshold instead of a local beta threshold
                        const double threshold = 1e-6;
                        if ( fabs( nativeBetaFactor ) > threshold )
                        {
                            betaFactorCalc.addValueAndWeight( nativeBetaFactor, intersectionLength );
                        }
                    }
                }
                if ( conductivityCalc.validAggregatedWeight() )
                {
                    weightedConductivity = conductivityCalc.weightedMean();
                }
                if ( widthCalc.validAggregatedWeight() )
                {
                    weightedWidth = widthCalc.weightedMean();
                    totalLength   = widthCalc.aggregatedWeight();
                }
                if ( betaFactorCalc.validAggregatedWeight() )
                {
                    weightedBetaFactorOnFile = betaFactorCalc.weightedMean();
                }
            }

            if ( totalLength > 1e-7 )
            {
                values.m_width        = weightedWidth;
                values.m_conductivity = weightedConductivity;

                double conversionFactorForBeta = conversionFactorForBetaValues();
                double betaFactorForcheimer    = weightedBetaFactorOnFile / conversionFactorForBeta;

                values.m_betaFactorInForcheimerUnits = betaFactorForcheimer;
            }

            values.m_permeability = RigTransmissibilityEquations::permeability( weightedConductivity, weightedWidth );
        }
    }
    else
    {
        std::pair<size_t, size_t> wellCellIJ = fractureGrid->fractureCellAtWellCenter();
        size_t wellCellIndex            = fractureGrid->getGlobalIndexFromIJ( wellCellIJ.first, wellCellIJ.second );
        const RigFractureCell& wellCell = fractureGrid->cellFromIndex( wellCellIndex );

        double conductivity   = wellCell.getConductivityValue();
        values.m_conductivity = conductivity;

        {
            auto nameUnit = widthParameterNameAndUnit();
            if ( !nameUnit.first.isEmpty() )
            {
                double widthInRequiredUnit = HUGE_VAL;
                {
                    auto resultValues = fractureGridResultsForUnitSystem( nameUnit.first,
                                                                          nameUnit.second,
                                                                          m_activeTimeStepIndex,
                                                                          fractureTemplateUnit() );

                    if ( wellCellIndex < resultValues.size() )
                    {
                        widthInRequiredUnit = resultValues[wellCellIndex];
                    }
                }

                if ( widthInRequiredUnit != HUGE_VAL && fabs( widthInRequiredUnit ) > 1e-20 )
                {
                    values.m_width        = widthInRequiredUnit;
                    values.m_permeability = RigTransmissibilityEquations::permeability( conductivity, widthInRequiredUnit );
                }
            }
        }

        {
            auto                nameUnit = betaFactorParameterNameAndUnit();
            std::vector<double> betaFactorResultValues =
                fractureGridResults( nameUnit.first, nameUnit.second, m_activeTimeStepIndex );

            if ( wellCellIndex < betaFactorResultValues.size() )
            {
                double nativeBetaValue = betaFactorResultValues[wellCellIndex];

                double conversionFactorForBeta = conversionFactorForBetaValues();
                double betaFactorForcheimer    = nativeBetaValue / conversionFactorForBeta;

                values.m_betaFactorInForcheimerUnits = betaFactorForcheimer;
            }
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMeshFractureTemplate::conversionFactorForBetaValues() const
{
    auto nameUnit = betaFactorParameterNameAndUnit();

    double conversionFactorForBeta = 1.0;

    QString trimmedUnit = nameUnit.second.trimmed().toLower();
    if ( trimmedUnit == "/m" )
    {
        conversionFactorForBeta = 1.01325E+08;
    }
    else if ( trimmedUnit == "/cm" )
    {
        conversionFactorForBeta = 1.01325E+06;
    }
    else if ( trimmedUnit == "/ft" )
    {
        conversionFactorForBeta = 3.088386E+07;
    }

    return conversionFactorForBeta;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMeshFractureTemplate::mapUiResultNameToFileResultName( const QString& uiResultName ) const
{
    QString fileResultName;

    if ( uiResultName == RiaDefines::conductivityResultName() )
    {
        fileResultName = m_conductivityResultNameOnFile();
    }
    else
    {
        fileResultName = uiResultName;
    }

    return fileResultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeshFractureTemplate::onLoadDataAndUpdateGeometryHasChanged()
{
    loadDataAndUpdate();

    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimMeshFractureTemplate::widthResultValues() const
{
    std::vector<double> resultValues;

    auto nameUnit = widthParameterNameAndUnit();
    if ( !nameUnit.first.isEmpty() )
    {
        resultValues = fractureGridResultsForUnitSystem( nameUnit.first,
                                                         nameUnit.second,
                                                         m_activeTimeStepIndex,
                                                         fractureTemplateUnit() );
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeshFractureTemplate::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_id );

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Input" );
        group->add( &m_stimPlanFileName );
        group->add( &m_activeTimeStepIndex );
        group->add( &m_userDefinedWellPathDepthAtFracture );
        group->add( &m_wellPathDepthAtFracture );

        m_wellPathDepthAtFracture.uiCapability()->setUiReadOnly( !m_userDefinedWellPathDepthAtFracture() );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Geometry" );
        group->add( &m_orientationType );
        group->add( &m_azimuthAngle );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Fracture Truncation" );
        group->setCollapsedByDefault();
        m_fractureContainment()->uiOrdering( uiConfigName, *group );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Properties" );
        group->add( &m_conductivityResultNameOnFile );
        group->add( &m_conductivityType );
        group->add( &m_skinFactor );
        group->add( &m_perforationLength );
        group->add( &m_perforationEfficiency );
        group->add( &m_wellDiameter );
    }

    if ( widthResultValues().empty() )
    {
        m_fractureWidthType = USER_DEFINED_WIDTH;
    }

    RimFractureTemplate::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMeshFractureTemplate::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                     QString                    uiConfigName,
                                                     caf::PdmUiEditorAttribute* attribute )
{
    RimFractureTemplate::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_stimPlanFileName )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_fileSelectionFilter = getFileSelectionFilter();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMeshFractureTemplate::wellPathDepthAtFractureUiName() const
{
    return "Well/Fracture Intersection Depth";
}
