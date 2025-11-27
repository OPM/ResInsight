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
#include "RiaNumberFormat.h"
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

CAF_PDM_SOURCE_INIT( RimStimPlanFractureTemplate, "StimPlanFractureTemplate", "RimStimPlanFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate()
{
    CAF_PDM_InitScriptableObject( "Fracture Template", ":/FractureTemplate16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_propertiesTable, "PropertiesTable", "Properties Table" );
    m_propertiesTable.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_propertiesTable.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_propertiesTable.uiCapability()->setUiReadOnly( true );
    m_propertiesTable.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_showStimPlanMesh_OBSOLETE, "ShowStimPlanMesh", true, "" );
    m_showStimPlanMesh_OBSOLETE.uiCapability()->setUiHidden( true );
    m_showStimPlanMesh_OBSOLETE.xmlCapability()->setIOWritable( false );

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
void RimStimPlanFractureTemplate::setDefaultsBasedOnFile()
{
    if ( m_stimPlanFractureDefinitionData.isNull() ) return;

    computeDepthOfWellPathAtFracture();
    computePerforationLength();

    RiaLogging::info( QString( "Setting well/fracture intersection depth at %1" ).arg( m_wellPathDepthAtFracture() ) );

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

    m_stimPlanFractureDefinitionData = RifStimPlanXmlReader::readStimPlanXMLFile( m_stimPlanFileName().path(),
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
QStringList RimStimPlanFractureTemplate::conductivityResultNames() const
{
    if ( m_stimPlanFractureDefinitionData.isNull() ) return QStringList();

    return m_stimPlanFractureDefinitionData->conductivityResultNames();
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
std::vector<double> RimStimPlanFractureTemplate::fractureGridResultsForUnitSystem( const QString&                resultName,
                                                                                   const QString&                unitName,
                                                                                   size_t                        timeStepIndex,
                                                                                   RiaDefines::EclipseUnitSystem requiredUnitSystem ) const
{
    auto resultValues = m_stimPlanFractureDefinitionData->fractureGridResults( resultName, unitName, m_activeTimeStepIndex );

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
std::pair<QString, QString> RimStimPlanFractureTemplate::widthParameterNameAndUnit() const
{
    return widthParameterNameAndUnit( m_stimPlanFractureDefinitionData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString>
    RimStimPlanFractureTemplate::widthParameterNameAndUnit( cvf::ref<RigStimPlanFractureDefinition> stimPlanFractureDefinitionData )
{
    if ( stimPlanFractureDefinitionData.notNull() )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile = stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

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
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile = m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

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
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile = m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

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
std::vector<QString> RimStimPlanFractureTemplate::timeStepsStrings()
{
    std::vector<QString> steps;
    std::vector<double>  timeStepsAsDouble = timeSteps();
    for ( auto d : timeStepsAsDouble )
    {
        steps.push_back( QString::number( d ) );
    }

    return steps;
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

        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile = m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();
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
std::vector<std::vector<double>>
    RimStimPlanFractureTemplate::resultValues( const QString& uiResultName, const QString& unitName, size_t timeStepIndex ) const
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
std::vector<double>
    RimStimPlanFractureTemplate::fractureGridResults( const QString& uiResultName, const QString& unitName, size_t timeStepIndex ) const
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
    return m_stimPlanFractureDefinitionData.notNull() && !m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty();
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
void RimStimPlanFractureTemplate::appendDataToResultStatistics( const QString&     uiResultName,
                                                                const QString&     unit,
                                                                MinMaxAccumulator& minMaxAccumulator,
                                                                PosNegAccumulator& posNegAccumulator ) const
{
    if ( m_stimPlanFractureDefinitionData.notNull() )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );

        m_stimPlanFractureDefinitionData->appendDataToResultStatistics( fileResultName, unit, minMaxAccumulator, posNegAccumulator );
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
    RimMeshFractureTemplate::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_propertiesTable );

    // if ( widthResultValues().empty() )
    // {
    //     m_fractureWidthType = USER_DEFINED_WIDTH;
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::getFileSelectionFilter() const
{
    return "StimPlan Xml Files(*.xml);;All Files (*.*)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    RimMeshFractureTemplate::defineEditorAttribute( field, uiConfigName, attribute );

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
            std::vector<double> values = RigEnsembleFractureStatisticsCalculator::calculateProperty( fractureDefinitions, propertyType );
            if ( !values.empty() )
            {
                appendTextIfValidValue( body, values[0], propertyType );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::appendTextIfValidValue( QString&                                              body,
                                                          double                                                value,
                                                          RigEnsembleFractureStatisticsCalculator::PropertyType propertyType )
{
    if ( value != HUGE_VAL )
    {
        QString name                   = caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::uiText( propertyType );
        auto [numberFormat, precision] = RigEnsembleFractureStatisticsCalculator::numberFormatForProperty( propertyType );
        body += QString( "%1: %2<br>" ).arg( name ).arg( RiaNumberFormat::valueToText( value, numberFormat, precision ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::isValidResult( double value ) const
{
    return value > 1e-7;
}
