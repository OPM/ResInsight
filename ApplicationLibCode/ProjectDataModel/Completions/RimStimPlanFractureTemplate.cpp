/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimStimPlanFractureTemplate.h"

#include "RiaApplication.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RifStimPlanXmlReader.h"

#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RigFractureGrid.h"
#include "RigStimPlanFractureDefinition.h"
#include "RigTransmissibilityEquations.h"
#include "RigWellPathStimplanIntersector.h"

#include "RigFractureCell.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanLegendConfig.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "RivWellFracturePartMgr.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTextEditor.h"

#include "cvfMath.h"
#include "cvfVector3.h"

#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <vector>

static std::vector<double> EMPTY_DOUBLE_VECTOR;

CAF_PDM_SOURCE_INIT( RimStimPlanFractureTemplate, "StimPlanFractureTemplate", "RimStimPlanFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate()
{
    CAF_PDM_InitScriptableObject( "Fracture Template", ":/FractureTemplate16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_stimPlanFileName, "StimPlanFileName", "File Name", "", "", "" );
    m_stimPlanFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_userDefinedWellPathDepthAtFracture,
                       "UserDefinedWellPathDepthAtFracture",
                       false,
                       "User-Defined Well/Fracture Intersection Depth",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_borderPolygonResultName, "BorderPolygonResultName", QString( "" ), "Parameter", "", "", "" );
    m_borderPolygonResultName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_activeTimeStepIndex, "ActiveTimeStepIndex", 0, "Active TimeStep Index", "", "", "" );
    CAF_PDM_InitField( &m_conductivityResultNameOnFile,
                       "ConductivityResultName",
                       QString( "" ),
                       "Active Conductivity Result Name",
                       "",
                       "",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_propertiesTable, "PropertiesTable", "Properties Table", "", "", "" );
    m_propertiesTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_propertiesTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_propertiesTable.uiCapability()->setUiReadOnly( true );
    m_propertiesTable.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_showStimPlanMesh_OBSOLETE, "ShowStimPlanMesh", true, "", "", "", "" );
    m_showStimPlanMesh_OBSOLETE.uiCapability()->setUiHidden( true );

    m_readError = false;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::~RimStimPlanFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanFractureTemplate::activeTimeStepIndex()
{
    return m_activeTimeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue )
{
    RimFractureTemplate::fieldChangedByUi( changedField, oldValue, newValue );

    if ( &m_stimPlanFileName == changedField )
    {
        m_readError = false;
        loadDataAndUpdate();
        setDefaultsBasedOnXMLfile();
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
void RimStimPlanFractureTemplate::setFileName( const QString& fileName )
{
    m_stimPlanFileName = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::fileName()
{
    return m_stimPlanFileName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::updateFilePathsFromProjectPath( const QString& newProjectPath,
                                                                  const QString& oldProjectPath )
{
    // m_stimPlanFileName = RimTools::relocateFile( m_stimPlanFileName(), newProjectPath, oldProjectPath, nullptr,
    // nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setDefaultsBasedOnXMLfile()
{
    if ( m_stimPlanFractureDefinitionData.isNull() ) return;

    computeDepthOfWellPathAtFracture();
    computePerforationLength();

    RiaLogging::info( QString( "Setting well/fracture intersection depth at %1" ).arg( m_wellPathDepthAtFracture ) );

    m_activeTimeStepIndex = static_cast<int>( m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1 );

    bool polygonPropertySet = setBorderPolygonResultNameToDefault();
    if ( polygonPropertySet )
        RiaLogging::info( QString( "Calculating polygon outline based on %1 at timestep %2" )
                              .arg( m_borderPolygonResultName )
                              .arg( m_stimPlanFractureDefinitionData->timeSteps()[m_activeTimeStepIndex] ) );
    else
        RiaLogging::info( QString( "Property for polygon calculation not set." ) );

    if ( m_stimPlanFractureDefinitionData->orientation() == RigStimPlanFractureDefinition::Orientation::TRANSVERSE )
    {
        m_orientationType = TRANSVERSE_WELL_PATH;
    }
    else if ( m_stimPlanFractureDefinitionData->orientation() == RigStimPlanFractureDefinition::Orientation::LONGITUDINAL )
    {
        m_orientationType = ALONG_WELL_PATH;
    }

    if ( !m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty() )
    {
        m_conductivityResultNameOnFile = m_stimPlanFractureDefinitionData->conductivityResultNames().front();
    }

    m_propertiesTable = generatePropertiesTable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::setBorderPolygonResultNameToDefault()
{
    // first option: Width
    for ( std::pair<QString, QString> property : uiResultNamesWithUnit() )
    {
        if ( property.first == "WIDTH" )
        {
            m_borderPolygonResultName = property.first;
            return true;
        }
    }

    // if width not found, use conductivity
    if ( hasConductivity() )
    {
        m_borderPolygonResultName = m_stimPlanFractureDefinitionData->conductivityResultNames().first();
        return true;
    }

    // else: Set to first property
    if ( !uiResultNamesWithUnit().empty() )
    {
        m_borderPolygonResultName = uiResultNamesWithUnit()[0].first;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::loadDataAndUpdate()
{
    QString errorMessage;

    if ( m_readError ) return;

    m_stimPlanFractureDefinitionData =
        RifStimPlanXmlReader::readStimPlanXMLFile( m_stimPlanFileName().path(),
                                                   m_conductivityScaleFactor(),
                                                   RifStimPlanXmlReader::MirrorMode::MIRROR_AUTO,
                                                   fractureTemplateUnit(),
                                                   &errorMessage );
    if ( errorMessage.size() > 0 ) RiaLogging::error( errorMessage );

    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        setDefaultConductivityResultIfEmpty();

        if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
        {
            setUnitSystem( m_stimPlanFractureDefinitionData->unitSet() );
        }

        if ( !m_userDefinedWellPathDepthAtFracture )
        {
            computeDepthOfWellPathAtFracture();
        }

        m_readError = false;
    }
    else
    {
        m_readError = true;
    }

    for ( RimFracture* fracture : fracturesUsingThisTemplate() )
    {
        fracture->updateFractureGrid();
        fracture->clearCachedNonDarcyProperties();
    }

    if ( widthResultValues().empty() )
    {
        m_fractureWidthType = USER_DEFINED_WIDTH;
    }

    // Todo: Must update all views using this fracture template
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
    if ( activeView ) activeView->fractureColors()->loadDataAndUpdate();

    m_propertiesTable = generatePropertiesTable();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimStimPlanFractureTemplate::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
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
        std::vector<double> timeValues = timeSteps();
        int                 index      = 0;
        for ( double value : timeValues )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( value ), index ) );
            index++;
        }
    }
    else if ( fieldNeedingOptions == &m_conductivityResultNameOnFile )
    {
        if ( m_stimPlanFractureDefinitionData.notNull() )
        {
            QStringList conductivityResultNames = m_stimPlanFractureDefinitionData->conductivityResultNames();
            for ( const auto& resultName : conductivityResultNames )
            {
                options.push_back( caf::PdmOptionItemInfo( resultName, resultName ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::computeDepthOfWellPathAtFracture()
{
    if ( !m_stimPlanFractureDefinitionData.isNull() )
    {
        double firstTvd = m_stimPlanFractureDefinitionData->topPerfTvd();
        double lastTvd  = m_stimPlanFractureDefinitionData->bottomPerfTvd();

        if ( firstTvd != HUGE_VAL && lastTvd != HUGE_VAL )
        {
            m_wellPathDepthAtFracture.setValueWithFieldChanged( ( firstTvd + lastTvd ) / 2 );
        }
        else
        {
            firstTvd = m_stimPlanFractureDefinitionData->minDepth();
            lastTvd  = m_stimPlanFractureDefinitionData->maxDepth();
            m_wellPathDepthAtFracture.setValueWithFieldChanged( ( firstTvd + lastTvd ) / 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::computePerforationLength()
{
    if ( !m_stimPlanFractureDefinitionData.isNull() )
    {
        double firstTvd = m_stimPlanFractureDefinitionData->topPerfTvd();
        double lastTvd  = m_stimPlanFractureDefinitionData->bottomPerfTvd();

        if ( firstTvd != HUGE_VAL && lastTvd != HUGE_VAL )
        {
            m_perforationLength = cvf::Math::abs( firstTvd - lastTvd );
        }
    }

    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && m_perforationLength < 10 )
    {
        m_perforationLength = 10;
    }
    else if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD &&
              m_perforationLength < RiaEclipseUnitTools::meterToFeet( 10 ) )
    {
        m_perforationLength = std::round( RiaEclipseUnitTools::meterToFeet( 10 ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RimStimPlanFractureTemplate::fractureGridResultsForUnitSystem( const QString&                resultName,
                                                                   const QString&                unitName,
                                                                   size_t                        timeStepIndex,
                                                                   RiaDefines::EclipseUnitSystem requiredUnitSystem ) const
{
    auto resultValues =
        m_stimPlanFractureDefinitionData->fractureGridResults( resultName, unitName, m_activeTimeStepIndex );

    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        for ( auto& v : resultValues )
        {
            v = RiaEclipseUnitTools::convertToMeter( v, unitName );
        }
    }
    else if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        for ( auto& v : resultValues )
        {
            v = RiaEclipseUnitTools::convertToFeet( v, unitName );
        }
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellFractureIntersectionData
    RimStimPlanFractureTemplate::wellFractureIntersectionData( const RimFracture* fractureInstance ) const
{
    WellFractureIntersectionData values;

    const RigFractureGrid* fractureGrid = fractureInstance->fractureGrid();
    if ( fractureGrid )
    {
        if ( orientationType() == ALONG_WELL_PATH )
        {
            CVF_ASSERT( fractureInstance );

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
                        auto nameUnit = betaFactorParameterNameAndUnit();
                        betaFactorResultValues =
                            m_stimPlanFractureDefinitionData->fractureGridResults( nameUnit.first,
                                                                                   nameUnit.second,
                                                                                   m_activeTimeStepIndex );
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
                        values.m_width = widthInRequiredUnit;
                        values.m_permeability =
                            RigTransmissibilityEquations::permeability( conductivity, widthInRequiredUnit );
                    }
                }
            }

            {
                auto                nameUnit = betaFactorParameterNameAndUnit();
                std::vector<double> betaFactorResultValues =
                    m_stimPlanFractureDefinitionData->fractureGridResults( nameUnit.first,
                                                                           nameUnit.second,
                                                                           m_activeTimeStepIndex );

                if ( wellCellIndex < betaFactorResultValues.size() )
                {
                    double nativeBetaValue = betaFactorResultValues[wellCellIndex];

                    double conversionFactorForBeta = conversionFactorForBetaValues();
                    double betaFactorForcheimer    = nativeBetaValue / conversionFactorForBeta;

                    values.m_betaFactorInForcheimerUnits = betaFactorForcheimer;
                }
            }
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::widthParameterNameAndUnit() const
{
    return widthParameterNameAndUnit( m_stimPlanFractureDefinitionData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::widthParameterNameAndUnit(
    cvf::ref<RigStimPlanFractureDefinition> stimPlanFractureDefinitionData )
{
    if ( stimPlanFractureDefinitionData.notNull() )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

        for ( const auto& nameUnit : propertyNamesUnitsOnFile )
        {
            if ( nameUnit.first.contains( "effective width", Qt::CaseInsensitive ) )
            {
                return nameUnit;
            }

            if ( nameUnit.first.contains( "width", Qt::CaseInsensitive ) )
            {
                return nameUnit;
            }
        }
    }

    return std::pair<QString, QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::conductivityParameterNameAndUnit() const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

        for ( const auto& nameUnit : propertyNamesUnitsOnFile )
        {
            if ( nameUnit.first.contains( m_conductivityResultNameOnFile, Qt::CaseInsensitive ) )
            {
                return nameUnit;
            }
        }
    }

    return std::pair<QString, QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::betaFactorParameterNameAndUnit() const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

        for ( const auto& nameUnit : propertyNamesUnitsOnFile )
        {
            if ( nameUnit.first.contains( "beta", Qt::CaseInsensitive ) )
            {
                return nameUnit;
            }
        }
    }

    return std::pair<QString, QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::isBetaFactorAvailableOnFile() const
{
    auto nameAndUnit = betaFactorParameterNameAndUnit();

    return !nameAndUnit.first.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanFractureTemplate::conversionFactorForBetaValues() const
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
void RimStimPlanFractureTemplate::setDefaultConductivityResultIfEmpty()
{
    if ( m_conductivityResultNameOnFile().isEmpty() )
    {
        if ( !m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty() )
        {
            m_conductivityResultNameOnFile = m_stimPlanFractureDefinitionData->conductivityResultNames().front();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::mapUiResultNameToFileResultName( const QString& uiResultName ) const
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
bool RimStimPlanFractureTemplate::showStimPlanMesh() const
{
    return m_showStimPlanMesh_OBSOLETE();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::convertToUnitSystem( RiaDefines::EclipseUnitSystem neededUnit )
{
    if ( m_fractureTemplateUnit() == neededUnit ) return;

    setUnitSystem( neededUnit );
    RimFractureTemplate::convertToUnitSystem( neededUnit );

    m_readError = false;
    loadDataAndUpdate();

    if ( m_stimPlanFractureDefinitionData.isNull() )
    {
        m_readError = true;
        // Force needed unit system when file reading fails to be able to open the project.
        setUnitSystem( neededUnit );
        return;
    }

    if ( neededUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        m_wellPathDepthAtFracture = RiaEclipseUnitTools::meterToFeet( m_wellPathDepthAtFracture );
    }
    else if ( neededUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        m_wellPathDepthAtFracture = RiaEclipseUnitTools::feetToMeter( m_wellPathDepthAtFracture );
    }

    m_activeTimeStepIndex   = static_cast<int>( m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1 );
    bool polygonPropertySet = setBorderPolygonResultNameToDefault();

    if ( polygonPropertySet )
        RiaLogging::info( QString( "Calculating polygon outline based on %1 at timestep %2" )
                              .arg( m_borderPolygonResultName )
                              .arg( m_stimPlanFractureDefinitionData->timeSteps()[m_activeTimeStepIndex] ) );
    else
        RiaLogging::info( QString( "Property for polygon calculation not set." ) );

    if ( !m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty() )
    {
        m_conductivityResultNameOnFile = m_stimPlanFractureDefinitionData->conductivityResultNames().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::onLoadDataAndUpdateGeometryHasChanged()
{
    loadDataAndUpdate();

    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::timeSteps()
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        return m_stimPlanFractureDefinitionData->timeSteps();
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RimStimPlanFractureTemplate::uiResultNamesWithUnit() const
{
    std::vector<std::pair<QString, QString>> propertyNamesAndUnits;

    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        QString conductivityUnit = "mD/s";

        std::vector<std::pair<QString, QString>> tmp;

        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();
        for ( const auto& nameUnitPair : propertyNamesUnitsOnFile )
        {
            if ( nameUnitPair.first.contains( RiaDefines::conductivityResultName(), Qt::CaseInsensitive ) )
            {
                conductivityUnit = nameUnitPair.second;
            }
            else
            {
                tmp.push_back( nameUnitPair );
            }
        }

        propertyNamesAndUnits.push_back( std::make_pair( RiaDefines::conductivityResultName(), conductivityUnit ) );

        for ( const auto& nameUnitPair : tmp )
        {
            propertyNamesAndUnits.push_back( nameUnitPair );
        }
    }

    return propertyNamesAndUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimStimPlanFractureTemplate::resultValues( const QString& uiResultName,
                                                                            const QString& unitName,
                                                                            size_t         timeStepIndex ) const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );

        return m_stimPlanFractureDefinitionData->getDataAtTimeIndex( fileResultName, unitName, timeStepIndex );
    }

    return std::vector<std::vector<double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::fractureGridResults( const QString& uiResultName,
                                                                      const QString& unitName,
                                                                      size_t         timeStepIndex ) const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );

        return m_stimPlanFractureDefinitionData->fractureGridResults( fileResultName, unitName, timeStepIndex );
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::hasConductivity() const
{
    if ( m_stimPlanFractureDefinitionData.notNull() &&
         !m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanFractureTemplate::resultValueAtIJ( const RigFractureGrid* fractureGrid,
                                                     const QString&         uiResultName,
                                                     const QString&         unitName,
                                                     size_t                 timeStepIndex,
                                                     size_t                 i,
                                                     size_t                 j )
{
    auto values = resultValues( uiResultName, unitName, timeStepIndex );

    if ( values.empty() ) return HUGE_VAL;

    size_t adjustedI = i + 1;
    size_t adjustedJ = j + 1;

    if ( adjustedI >= fractureGrid->iCellCount() || adjustedJ >= fractureGrid->jCellCount() )
    {
        return HUGE_VAL;
    }

    return values[adjustedJ][adjustedI];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::widthResultValues() const
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
void RimStimPlanFractureTemplate::appendDataToResultStatistics( const QString&     uiResultName,
                                                                const QString&     unit,
                                                                MinMaxAccumulator& minMaxAccumulator,
                                                                PosNegAccumulator& posNegAccumulator ) const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );

        m_stimPlanFractureDefinitionData->appendDataToResultStatistics( fileResultName,
                                                                        unit,
                                                                        minMaxAccumulator,
                                                                        posNegAccumulator );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fractureTriangleGeometry( std::vector<cvf::Vec3f>* nodeCoords,
                                                            std::vector<cvf::uint>*  triangleIndices,
                                                            double                   wellPathDepthAtFracture ) const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        m_stimPlanFractureDefinitionData->createFractureTriangleGeometry( m_halfLengthScaleFactor(),
                                                                          m_heightScaleFactor(),
                                                                          wellPathDepthAtFracture,
                                                                          name(),
                                                                          nodeCoords,
                                                                          triangleIndices );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
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
        group->setCollapsedByDefault( true );
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

    uiOrdering.add( &m_propertiesTable );

    if ( widthResultValues().empty() )
    {
        m_fractureWidthType = USER_DEFINED_WIDTH;
    }

    RimFractureTemplate::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    RimFractureTemplate::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_stimPlanFileName )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_fileSelectionFilter = "StimPlan Xml Files(*.xml);;All Files (*.*)";
        }
    }

    if ( field == &m_propertiesTable )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->wrapMode = caf::PdmUiTextEditorAttribute::NoWrap;
            myAttr->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2020.10.0" ) )
    {
        m_userDefinedWellPathDepthAtFracture = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimStimPlanFractureTemplate::wellPathDepthAtFractureRange() const
{
    if ( m_stimPlanFractureDefinitionData.isNull() ) return std::make_pair( 0.0, 1.0 );

    return std::make_pair( m_stimPlanFractureDefinitionData->minDepth(), m_stimPlanFractureDefinitionData->maxDepth() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::cref<RigFractureGrid> RimStimPlanFractureTemplate::createFractureGrid( double wellPathDepthAtFracture ) const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        return m_stimPlanFractureDefinitionData->createFractureGrid( m_conductivityResultNameOnFile,
                                                                     m_activeTimeStepIndex,
                                                                     m_halfLengthScaleFactor(),
                                                                     m_heightScaleFactor(),
                                                                     wellPathDepthAtFracture,
                                                                     m_fractureTemplateUnit() );
    }

    return cvf::cref<RigFractureGrid>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::wellPathDepthAtFractureUiName() const
{
    return "Well/Fracture Intersection Depth";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanFractureTemplate::formationDip() const
{
    if ( m_stimPlanFractureDefinitionData.isNull() ) return HUGE_VAL;

    return m_stimPlanFractureDefinitionData->formationDip();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::generatePropertiesTable() const
{
    QString body;

    if ( !m_stimPlanFractureDefinitionData.isNull() )
    {
        appendTextIfValidValue( body, "Top MD", m_stimPlanFractureDefinitionData->topPerfMd() );
        appendTextIfValidValue( body, "Bottom MD", m_stimPlanFractureDefinitionData->bottomPerfMd() );
        appendTextIfValidValue( body, "Top TVD", m_stimPlanFractureDefinitionData->topPerfTvd() );
        appendTextIfValidValue( body, "Bottom TVD", m_stimPlanFractureDefinitionData->bottomPerfTvd() );

        std::vector<cvf::ref<RigStimPlanFractureDefinition>> fractureDefinitions = { m_stimPlanFractureDefinitionData };

        std::vector<RigEnsembleFractureStatisticsCalculator::PropertyType> propertyTypes =
            RigEnsembleFractureStatisticsCalculator::propertyTypes();
        for ( auto propertyType : propertyTypes )
        {
            std::vector<double> values =
                RigEnsembleFractureStatisticsCalculator::calculateProperty( fractureDefinitions, propertyType );
            if ( !values.empty() )
            {
                QString name = caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::uiText( propertyType );
                appendTextIfValidValue( body, name, values[0] );
            }
        }
    }

    return body;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::appendTextIfValidValue( QString& body, const QString& title, double value )
{
    if ( value != HUGE_VAL )
    {
        body += QString( "%1: %2<br>" ).arg( title ).arg( value );
    }
}
