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

#include "RimThermalFractureTemplate.h"

#include "RiaApplication.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"

#include "RifThermalFractureReader.h"

#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigThermalFractureDefinition.h"
#include "RigThermalFractureResultUtil.h"

#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTextEditor.h"

#include "cvfMath.h"
#include "cvfVector3.h"

#include <QDateTime>
#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <vector>

CAF_PDM_SOURCE_INIT( RimThermalFractureTemplate, "ThermalFractureTemplate", "RimThermalFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimThermalFractureTemplate::RimThermalFractureTemplate()
{
    CAF_PDM_InitScriptableObject( "Fracture Template", ":/FractureTemplate16x16.png" );

    m_readError = false;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimThermalFractureTemplate::~RimThermalFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimThermalFractureTemplate::setDefaultsBasedOnFile()
{
    if ( !m_fractureDefinitionData ) return;

    computeDepthOfWellPathAtFracture();
    computePerforationLength();

    RiaLogging::info( QString( "Setting well/fracture intersection depth at %1" ).arg( m_wellPathDepthAtFracture ) );

    m_activeTimeStepIndex = static_cast<int>( m_fractureDefinitionData->numTimeSteps() - 1 );

    bool polygonPropertySet = setBorderPolygonResultNameToDefault();
    if ( polygonPropertySet )
        RiaLogging::info( QString( "Calculating polygon outline based on %1 at timestep %2" )
                              .arg( m_borderPolygonResultName )
                              .arg( m_fractureDefinitionData->timeSteps()[m_activeTimeStepIndex] ) );
    else
        RiaLogging::info( QString( "Property for polygon calculation not set." ) );

    QStringList resultNames = conductivityResultNames();
    if ( !resultNames.isEmpty() )
    {
        m_conductivityResultNameOnFile = resultNames.front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimThermalFractureTemplate::setBorderPolygonResultNameToDefault()
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
        m_borderPolygonResultName = conductivityResultNames().first();
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
void RimThermalFractureTemplate::loadDataAndUpdate()
{
    if ( m_readError ) return;

    auto [fractureDefinitionData, errorMessage] =
        RifThermalFractureReader::readFractureCsvFile( m_stimPlanFileName().path() );
    if ( errorMessage.size() > 0 ) RiaLogging::error( errorMessage );

    m_fractureDefinitionData = fractureDefinitionData;
    if ( m_fractureDefinitionData )
    {
        setDefaultConductivityResultIfEmpty();

        if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
        {
            setUnitSystem( m_fractureDefinitionData->unitSystem() );
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

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimThermalFractureTemplate::conductivityResultNames() const
{
    QStringList resultNames;
    if ( !m_fractureDefinitionData ) return resultNames;

    for ( auto [name, unit] : m_fractureDefinitionData->getPropertyNamesUnits() )
    {
        resultNames.append( name );
    }

    return resultNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimThermalFractureTemplate::computeDepthOfWellPathAtFracture()
{
    if ( m_fractureDefinitionData )
    {
        double z = m_fractureDefinitionData->centerPosition().z();
        m_wellPathDepthAtFracture.setValueWithFieldChanged( z );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimThermalFractureTemplate::computePerforationLength()
{
    if ( m_fractureDefinitionData )
    {
        auto [firstTvd, lastTvd] =
            RigThermalFractureResultUtil::minMaxDepth( m_fractureDefinitionData, m_activeTimeStepIndex );

        if ( firstTvd != HUGE_VAL && lastTvd != HUGE_VAL )
        {
            m_perforationLength = cvf::Math::abs( firstTvd - lastTvd );
        }
    }

    double minPerforationLength = 10.0;
    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && m_perforationLength < minPerforationLength )
    {
        m_perforationLength = minPerforationLength;
    }
    else if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD &&
              m_perforationLength < RiaEclipseUnitTools::meterToFeet( minPerforationLength ) )
    {
        m_perforationLength = std::round( RiaEclipseUnitTools::meterToFeet( minPerforationLength ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RimThermalFractureTemplate::fractureGridResultsForUnitSystem( const QString&                resultName,
                                                                  const QString&                unitName,
                                                                  size_t                        timeStepIndex,
                                                                  RiaDefines::EclipseUnitSystem requiredUnitSystem ) const
{
    auto resultValues = fractureGridResults( resultName, unitName, m_activeTimeStepIndex );

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
std::pair<QString, QString> RimThermalFractureTemplate::widthParameterNameAndUnit() const
{
    return widthParameterNameAndUnit( m_fractureDefinitionData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString>
    RimThermalFractureTemplate::widthParameterNameAndUnit( std::shared_ptr<RigThermalFractureDefinition> fractureDefinitionData )
{
    if ( fractureDefinitionData )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            fractureDefinitionData->getPropertyNamesUnits();

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
std::pair<QString, QString> RimThermalFractureTemplate::conductivityParameterNameAndUnit() const
{
    if ( m_fractureDefinitionData )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_fractureDefinitionData->getPropertyNamesUnits();

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
std::pair<QString, QString> RimThermalFractureTemplate::betaFactorParameterNameAndUnit() const
{
    if ( m_fractureDefinitionData )
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_fractureDefinitionData->getPropertyNamesUnits();

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
bool RimThermalFractureTemplate::isBetaFactorAvailableOnFile() const
{
    auto nameAndUnit = betaFactorParameterNameAndUnit();

    return !nameAndUnit.first.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimThermalFractureTemplate::conversionFactorForBetaValues() const
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
void RimThermalFractureTemplate::setDefaultConductivityResultIfEmpty()
{
    if ( m_conductivityResultNameOnFile().isEmpty() )
    {
        if ( !conductivityResultNames().isEmpty() )
        {
            m_conductivityResultNameOnFile = conductivityResultNames().front();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimThermalFractureTemplate::mapUiResultNameToFileResultName( const QString& uiResultName ) const
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
void RimThermalFractureTemplate::convertToUnitSystem( RiaDefines::EclipseUnitSystem neededUnit )
{
    if ( m_fractureTemplateUnit() == neededUnit ) return;

    setUnitSystem( neededUnit );
    RimFractureTemplate::convertToUnitSystem( neededUnit );

    m_readError = false;
    loadDataAndUpdate();

    if ( !m_fractureDefinitionData )
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

    m_activeTimeStepIndex   = static_cast<int>( m_fractureDefinitionData->numTimeSteps() - 1 );
    bool polygonPropertySet = setBorderPolygonResultNameToDefault();

    if ( polygonPropertySet )
        RiaLogging::info( QString( "Calculating polygon outline based on %1 at timestep %2" )
                              .arg( m_borderPolygonResultName )
                              .arg( m_fractureDefinitionData->timeSteps()[m_activeTimeStepIndex] ) );
    else
        RiaLogging::info( QString( "Property for polygon calculation not set." ) );

    if ( !conductivityResultNames().isEmpty() )
    {
        m_conductivityResultNameOnFile = conductivityResultNames().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimThermalFractureTemplate::timeSteps()
{
    if ( m_fractureDefinitionData )
    {
        return m_fractureDefinitionData->timeSteps();
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimThermalFractureTemplate::timeStepsStrings()
{
    std::vector<QString> steps;
    std::vector<double>  timeStepsAsDouble = timeSteps();
    for ( auto d : timeStepsAsDouble )
    {
        QDateTime dateTime;
        dateTime.setSecsSinceEpoch( d );
        steps.push_back( dateTime.toString() );
    }

    return steps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RimThermalFractureTemplate::uiResultNamesWithUnit() const
{
    std::vector<std::pair<QString, QString>> propertyNamesAndUnits;

    if ( m_fractureDefinitionData )
    {
        QString conductivityUnit = "mD/s";

        std::vector<std::pair<QString, QString>> tmp;

        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_fractureDefinitionData->getPropertyNamesUnits();
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
std::vector<std::vector<double>> RimThermalFractureTemplate::resultValues( const QString& uiResultName,
                                                                           const QString& unitName,
                                                                           size_t         timeStepIndex ) const
{
    if ( m_fractureDefinitionData )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );

        return RigThermalFractureResultUtil::getDataAtTimeIndex( m_fractureDefinitionData,
                                                                 fileResultName,
                                                                 unitName,
                                                                 timeStepIndex );
    }

    return std::vector<std::vector<double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimThermalFractureTemplate::fractureGridResults( const QString& uiResultName,
                                                                     const QString& unitName,
                                                                     size_t         timeStepIndex ) const
{
    if ( m_fractureDefinitionData )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );

        return RigThermalFractureResultUtil::fractureGridResults( m_fractureDefinitionData,
                                                                  fileResultName,
                                                                  unitName,
                                                                  timeStepIndex );
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimThermalFractureTemplate::hasConductivity() const
{
    return ( m_fractureDefinitionData && !conductivityResultNames().isEmpty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimThermalFractureTemplate::resultValueAtIJ( const RigFractureGrid* fractureGrid,
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
void RimThermalFractureTemplate::appendDataToResultStatistics( const QString&     uiResultName,
                                                               const QString&     unit,
                                                               MinMaxAccumulator& minMaxAccumulator,
                                                               PosNegAccumulator& posNegAccumulator ) const
{
    if ( m_fractureDefinitionData )
    {
        QString fileResultName = mapUiResultNameToFileResultName( uiResultName );
        RigThermalFractureResultUtil::appendDataToResultStatistics( m_fractureDefinitionData,
                                                                    fileResultName,
                                                                    unit,
                                                                    minMaxAccumulator,
                                                                    posNegAccumulator );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimThermalFractureTemplate::fractureTriangleGeometry( std::vector<cvf::Vec3f>* nodeCoords,
                                                           std::vector<cvf::uint>*  triangleIndices,
                                                           double                   wellPathDepthAtFracture ) const
{
    if ( m_fractureDefinitionData )
    {
        RigThermalFractureResultUtil::createFractureTriangleGeometry( m_fractureDefinitionData,
                                                                      m_activeTimeStepIndex,
                                                                      m_halfLengthScaleFactor(),
                                                                      m_heightScaleFactor(),
                                                                      wellPathDepthAtFracture,
                                                                      nodeCoords,
                                                                      triangleIndices );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimThermalFractureTemplate::getFileSelectionFilter() const
{
    return "Reveal Open-Server Files (*.csv);;All Files (*.*)";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimThermalFractureTemplate::wellPathDepthAtFractureRange() const
{
    if ( !m_fractureDefinitionData ) return std::make_pair( 0.0, 1.0 );

    return RigThermalFractureResultUtil::minMaxDepth( m_fractureDefinitionData, m_activeTimeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::cref<RigFractureGrid> RimThermalFractureTemplate::createFractureGrid( double wellPathDepthAtFracture ) const
{
    if ( m_fractureDefinitionData )
    {
        return RigThermalFractureResultUtil::createFractureGrid( m_fractureDefinitionData,
                                                                 m_conductivityResultNameOnFile,
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
QString RimThermalFractureTemplate::wellPathDepthAtFractureUiName() const
{
    return "Well/Fracture Intersection Depth";
}
