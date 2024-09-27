#include "RicCreateDepthAdjustedLasFilesFeature.h"
/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicCreateDepthAdjustedLasFilesImpl.h"

#include "RiaLogging.h"

#include "RicCreateDepthAdjustedLasFilesUi.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogFile.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"

#include "NRLib/nrlib/well/laswell.hpp"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LasDepthValueAndIndexPerKLayer::insertIndexAndValue( int kLayer, size_t index, double value )
{
    m_kLayerIndexAndValuePairsMap[kLayer][index] = value;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool LasDepthValueAndIndexPerKLayer::hasKLayer( int kLayer ) const
{
    return m_kLayerIndexAndValuePairsMap.find( kLayer ) != m_kLayerIndexAndValuePairsMap.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, double> LasDepthValueAndIndexPerKLayer::indexAndValuePairs( int kLayer ) const
{
    if ( !hasKLayer( kLayer ) ) return std::map<size_t, double>();

    return m_kLayerIndexAndValuePairsMap.at( kLayer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RicCreateDepthAdjustedLasFilesImpl::createIndexKResultAccessor( RimEclipseCase* eclipseCase )
{
    const int               firstTimeStep = 0;
    const int               gridIndex     = 0;
    RigEclipseResultAddress indexKResAdr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::indexKResultName() );
    eclipseCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( indexKResAdr );

    return RigResultAccessorFactory::createFromResultAddress( eclipseCase->eclipseCaseData(),
                                                              gridIndex,
                                                              RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                              firstTimeStep,
                                                              indexKResAdr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LasDepthValueAndIndexPerKLayer
    RicCreateDepthAdjustedLasFilesImpl::createLasDepthIndexAndPercValuePerKLayerFromMap( const std::vector<double>& lasWellDepths,
                                                                                         const std::map<int, IndexKDepthData>& indexKDepthDataMap )
{
    // Create container of depth value (in percent) and its original index in a LAS file vector
    // categorized by K-layer. Depth value as percentage value between MD top and MD bottom for K-layer.
    auto lasWellDepthValueAndIndexPerKLayer = LasDepthValueAndIndexPerKLayer();
    for ( size_t i = 0; i < lasWellDepths.size(); ++i )
    {
        const double depth = lasWellDepths[i];
        for ( const auto& [indexK, depthData] : indexKDepthDataMap )
        {
            if ( depthData.mdTop <= depth && depth <= depthData.mdBottom )
            {
                const double percentage = ( depth - depthData.mdTop ) / ( depthData.mdBottom - depthData.mdTop );
                lasWellDepthValueAndIndexPerKLayer.insertIndexAndValue( indexK, i, percentage );
                break;
            }
        }
    }
    return lasWellDepthValueAndIndexPerKLayer;
}

//--------------------------------------------------------------------------------------------------
/// NOTE: map createIndexKDepthDataMapFromCase is created using well extractor, while sourceWellLogData depth
/// values are from LAS file. Floating point rounding in LAS file can occur, thus depth values might be placed
/// outside of K-layer close to top/bottom due to inaccuracy.
//--------------------------------------------------------------------------------------------------
void RicCreateDepthAdjustedLasFilesImpl::createDestinationWellsLasFiles( RimCase*                        selectedCase,
                                                                         RimWellPath*                    sourceWell,
                                                                         RimWellLogLasFile*              soureWellLogFile,
                                                                         const std::vector<RimWellPath*> destinationWells,
                                                                         const std::vector<QString>&     selectedResultProperties,
                                                                         const QString&                  exportFolder,
                                                                         double                          rkbDiff )
{
    if ( !selectedCase || !sourceWell || !soureWellLogFile || destinationWells.empty() ) return;

    auto*      sourceWellLogData  = soureWellLogFile->wellLogData();
    const auto defaultPropertyMap = createDefaultPropertyMap( selectedResultProperties, sourceWellLogData );

    // NOTE: map createIndexKDepthDataMapFromCase is created using well extractor, while sourceWellLogData depth
    // values are from LAS file. Floating point rounding in LAS file can occur, thus depth values might be placed
    // outside of K-layer close to top/bottom due to inaccuracy.
    const auto sourceWellDepthIndexAndPercValuePerKLayer =
        createLasDepthIndexAndPercValuePerKLayerFromMap( sourceWellLogData->depthValues(),
                                                         createIndexKDepthDataMapFromCase( selectedCase, sourceWell ) );
    for ( RimWellPath* well : destinationWells )
    {
        const std::map<int, IndexKDepthData> destinationWellIndexKDepthsMap = createIndexKDepthDataMapFromCase( selectedCase, well );

        if ( destinationWellIndexKDepthsMap.empty() ) continue;

        std::vector<double>                    mdValues;
        std::vector<double>                    tvdMslValues;
        std::vector<double>                    tvdRkbValues;
        std::map<QString, std::vector<double>> propertyMap = defaultPropertyMap;
        for ( const auto& [indexK, depthData] : destinationWellIndexKDepthsMap )
        {
            if ( !sourceWellDepthIndexAndPercValuePerKLayer.hasKLayer( indexK ) ) continue;

            for ( const auto& [index, depthPerc] : sourceWellDepthIndexAndPercValuePerKLayer.indexAndValuePairs( indexK ) )
            {
                if ( sourceWellLogData->hasTvdMslChannel() )
                {
                    const double tvdMslValue = depthPerc * ( depthData.tvdBottom - depthData.tvdTop ) + depthData.tvdTop;
                    tvdMslValues.push_back( tvdMslValue );
                }
                if ( sourceWellLogData->hasTvdRkbChannel() )
                {
                    const double tvdRkbValue = depthPerc * ( depthData.tvdBottom - depthData.tvdTop ) + depthData.tvdTop + rkbDiff;
                    tvdRkbValues.push_back( tvdRkbValue );
                }

                const double mdValue = depthPerc * ( depthData.mdBottom - depthData.mdTop ) + depthData.mdTop;
                mdValues.push_back( mdValue );
                for ( auto& [propertyName, values] : propertyMap )
                {
                    double value = sourceWellLogData->values( propertyName )[index];
                    value        = value == HUGE_VAL ? sourceWellLogData->getMissingValue() : value;
                    values.push_back( value );
                }
            }
        }
        createDestinationWellLasFile( well->name(),
                                      selectedCase->caseUserDescription(),
                                      mdValues,
                                      tvdMslValues,
                                      tvdRkbValues,
                                      propertyMap,
                                      sourceWellLogData,
                                      exportFolder );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDepthAdjustedLasFilesImpl::createDestinationWellLasFile( const QString&                                wellName,
                                                                       const QString&                                caseDescription,
                                                                       const std::vector<double>&                    mdValues,
                                                                       const std::vector<double>&                    tvdMslValues,
                                                                       const std::vector<double>&                    tvdRkbValues,
                                                                       const std::map<QString, std::vector<double>>& propertyMap,
                                                                       const RigWellLogLasFile*                      sourceWellLogData,
                                                                       const QString&                                exportFolder )
{
    const auto depthUnitText    = createDepthUnitText( sourceWellLogData->depthUnit() );
    const auto depthUnitComment = createDepthUnitComment( sourceWellLogData->depthUnit() );

    const auto deptUnit = sourceWellLogData->depthUnit();

    // Build LAS file
    NRLib::LasWell lasFile;

    lasFile.setVersionInfo( "2.0" );
    lasFile.setDepthUnit( depthUnitText );
    lasFile.SetMissing( sourceWellLogData->getMissingValue() );
    lasFile.setStartDepth( *std::min_element( mdValues.begin(), mdValues.end() ) );
    lasFile.setStopDepth( *std::max_element( mdValues.begin(), mdValues.end() ) );

    lasFile.addWellInfo( "WELL", wellName.toStdString() );
    lasFile.addWellInfo( "DATE", sourceWellLogData->date().toStdString() );

    // Add Measured depth
    lasFile.AddLog( RiaDefines::propertyNameMeasuredDepth().toStdString(), depthUnitText, "Depth " + depthUnitComment, mdValues );

    // Add tvd msl values if existing
    if ( !tvdMslValues.empty() )
    {
        const auto unitText =
            sourceWellLogData->convertedWellLogChannelUnitString( RiaDefines::propertyNameTvdMslDepth(), deptUnit ).toStdString();
        lasFile.AddLog( RiaDefines::propertyNameTvdMslDepth().toStdString(), unitText, "True vertical depth " + depthUnitComment, tvdMslValues );
    }

    // Add tvd rkb values if existing
    if ( !tvdRkbValues.empty() )
    {
        const auto unitText =
            sourceWellLogData->convertedWellLogChannelUnitString( RiaDefines::propertyNameTvdRkbDepth(), deptUnit ).toStdString();
        lasFile.AddLog( RiaDefines::propertyNameTvdRkbDepth().toStdString(), unitText, "True vertical depth (Rotary Kelly Bushing)", tvdRkbValues );
    }

    // Add property values
    for ( auto& [name, values] : propertyMap )
    {
        std::string unitText = sourceWellLogData->wellLogChannelUnitString( name ).toStdString();
        lasFile.AddLog( name.toUpper().toStdString(), unitText, "", values );
    }

    // Add comment to LAS file
    const std::vector<std::string> commentHeader = {
        QString( "Note: Generated depth adjusted LAS file for '%1', using '%2'" ).arg( wellName ).arg( sourceWellLogData->wellName() ).toStdString() };

    // Add property value to file name if single property
    QString propertyNameStr;
    if ( propertyMap.size() == 1 )
    {
        propertyNameStr = QString( "-%1" ).arg( propertyMap.begin()->first );
    }

    // Replace white space from well names in file name
    QString sourceWell      = sourceWellLogData->wellName();
    sourceWell              = sourceWell.replace( QRegularExpression( "[\\s]+" ), "_" );
    QString destinationWell = wellName;
    destinationWell         = destinationWell.replace( QRegularExpression( "[\\s]+" ), "_" );

    // Create full file path name
    QString fullPathName = exportFolder + "/" + destinationWell + "_Depth_Adjusted_Using_" + sourceWell + "_" + caseDescription +
                           propertyNameStr + "-" + sourceWellLogData->date() + ".las";
    lasFile.WriteToFile( fullPathName.toStdString(), commentHeader );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RicCreateDepthAdjustedLasFilesImpl::createDepthUnitText( RiaDefines::DepthUnitType depthUnitType )
{
    return depthUnitType == RiaDefines::DepthUnitType::UNIT_METER ? "M" : depthUnitType == RiaDefines::DepthUnitType::UNIT_FEET ? "FT" : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RicCreateDepthAdjustedLasFilesImpl::createDepthUnitComment( RiaDefines::DepthUnitType depthUnitType )
{
    return depthUnitType == RiaDefines::DepthUnitType::UNIT_METER  ? "in meters"
           : depthUnitType == RiaDefines::DepthUnitType::UNIT_FEET ? "in feet"
                                                                   : "in Connection number";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, RicCreateDepthAdjustedLasFilesImpl::IndexKDepthData>
    RicCreateDepthAdjustedLasFilesImpl::createIndexKDepthDataMapFromCase( RimCase* selectedCase, RimWellPath* wellPath )
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( selectedCase );
    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( selectedCase );

    RimWellLogPlotCollection* wellLogCollection = RimMainPlotCollection::current()->wellLogPlotCollection();
    if ( eclipseCase != nullptr )
    {
        cvf::ref<RigEclipseWellLogExtractor> wellExtractor = wellLogCollection->findOrCreateExtractor( wellPath, eclipseCase );
        if ( wellExtractor.isNull() )
        {
            RiaLogging::info( QString( "Could not create RigEclipseWellLogExtractor for %1" ).arg( wellPath->name() ) );
        }
        const auto result = createIndexKDepthDataMap( wellExtractor, createIndexKResultAccessor( eclipseCase ) );
        if ( result.empty() )
        {
            RiaLogging::info( QString( "Not able to create Index-K depth map for %1" ).arg( wellPath->name() ) );
        }
        return result;
    }
    else if ( geomCase != nullptr )
    {
        cvf::ref<RigGeoMechWellLogExtractor> wellExtractor = wellLogCollection->findOrCreateExtractor( wellPath, geomCase );
        if ( wellExtractor.isNull() )
        {
            RiaLogging::info( QString( "Could not create RigGeoMechWellLogExtractor for %1" ).arg( wellPath->name() ) );
        }
        const auto result = createIndexKDepthDataMap( wellExtractor );
        if ( result.empty() )
        {
            RiaLogging::info( QString( "Not able to create Index-K depth map for %1" ).arg( wellPath->name() ) );
        }
        return result;
    }

    RiaLogging::info( QString( "Invalid case when creating Index-K depth map for %1" ).arg( wellPath->name() ) );
    return std::map<int, RicCreateDepthAdjustedLasFilesImpl::IndexKDepthData>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, RicCreateDepthAdjustedLasFilesImpl::IndexKDepthData>
    RicCreateDepthAdjustedLasFilesImpl::createIndexKDepthDataMap( cvf::ref<RigEclipseWellLogExtractor> wellExtractor,
                                                                  cvf::ref<RigResultAccessor>          indexKResAcc )
{
    std::vector<double> wellIndexKValues;
    wellExtractor->curveData( indexKResAcc.p(), &wellIndexKValues );
    return createIndexKDepthDataMapFromVectors( wellExtractor->cellIntersectionMDs(), wellExtractor->cellIntersectionTVDs(), wellIndexKValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, RicCreateDepthAdjustedLasFilesImpl::IndexKDepthData>
    RicCreateDepthAdjustedLasFilesImpl::createIndexKDepthDataMap( cvf::ref<RigGeoMechWellLogExtractor> wellExtractor )
{
    const int           frameIdx    = 0;
    const int           timeStepIdx = 0;
    RigFemResultAddress indexKResAdr( RigFemResultPosEnum::RIG_ELEMENT_NODAL, "INDEX", "INDEX_K" );
    std::vector<double> wellIndexKValues;
    wellExtractor->curveData( indexKResAdr, timeStepIdx, frameIdx, &wellIndexKValues );
    return createIndexKDepthDataMapFromVectors( wellExtractor->cellIntersectionMDs(), wellExtractor->cellIntersectionTVDs(), wellIndexKValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, RicCreateDepthAdjustedLasFilesImpl::IndexKDepthData>
    RicCreateDepthAdjustedLasFilesImpl::createIndexKDepthDataMapFromVectors( const std::vector<double>& wellMdValues,
                                                                             const std::vector<double>& wellTvdValues,
                                                                             const std::vector<double>& wellIndexKValues )
{
    std::map<int, IndexKDepthData> indexKDepthsMap;

    // Must have non-empty equal length vectors!
    if ( wellIndexKValues.empty() )
    {
        RiaLogging::info( QString( "Empty vector of index-K values" ) );
        return indexKDepthsMap;
    }
    if ( wellMdValues.size() != wellTvdValues.size() || wellMdValues.size() != wellIndexKValues.size() )
    {
        return indexKDepthsMap;
    }

    int prevKLayer = -1;
    for ( size_t i = 0; i < wellIndexKValues.size(); ++i )
    {
        // Asymptotically increasing k-indexes!
        const auto kLayer = static_cast<int>( wellIndexKValues[i] );
        if ( kLayer < prevKLayer ) break;

        if ( indexKDepthsMap.find( kLayer ) == indexKDepthsMap.end() )
        {
            indexKDepthsMap[kLayer]           = IndexKDepthData();
            indexKDepthsMap[kLayer].mdTop     = wellMdValues[i];
            indexKDepthsMap[kLayer].mdBottom  = wellMdValues[i];
            indexKDepthsMap[kLayer].tvdTop    = wellTvdValues[i];
            indexKDepthsMap[kLayer].tvdBottom = wellTvdValues[i];
        }
        else
        {
            indexKDepthsMap[kLayer].mdBottom  = wellMdValues[i];
            indexKDepthsMap[kLayer].tvdBottom = wellTvdValues[i];
        }
    }

    return indexKDepthsMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<double>>
    RicCreateDepthAdjustedLasFilesImpl::createDefaultPropertyMap( const std::vector<QString>& selectedProperties,
                                                                  const RigWellLogLasFile*    wellLogFile )
{
    const QStringList lasDepthNames = QStringList(
        { RiaDefines::propertyNameMeasuredDepth(), RiaDefines::propertyNameTvdMslDepth(), RiaDefines::propertyNameTvdRkbDepth() } );
    std::vector<QString> validPropertyNames;
    for ( const auto& propertyName : selectedProperties )
    {
        if ( !lasDepthNames.contains( propertyName ) && wellLogFile->wellLogChannelNames().contains( propertyName ) )
        {
            validPropertyNames.push_back( propertyName );
        }
    }
    std::map<QString, std::vector<double>> defaultPropertyMap;
    for ( const auto& name : validPropertyNames )
    {
        defaultPropertyMap[name];
    }
    return defaultPropertyMap;
}
