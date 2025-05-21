/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimFractureSurface.h"

#include "RiaLogging.h"

#include "RigStatisticsMath.h"
#include "Surface/RigSurface.h"
#include "Surface/RigTriangleMeshData.h"

#include "RimRegularLegendConfig.h"
#include "RimSurfaceCollection.h"

#include "RifSurfaceImporter.h"
#include "RifVtkImportUtil.h"
#include "RifVtkSurfaceImporter.h"

#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimFractureSurface, "FractureSurface", "RimFractureSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureSurface::RimFractureSurface()
{
    CAF_PDM_InitScriptableObject( "Surface", ":/ReservoirSurface16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceDefinitionFilePath, "SurfaceFilePath", "File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureSurface::~RimFractureSurface()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::setSurfaceFilePath( const QString& filePath )
{
    m_surfaceDefinitionFilePath = filePath;

    clearCachedNativeData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureSurface::surfaceFilePath()
{
    return m_surfaceDefinitionFilePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::onLoadData()
{
    return updateSurfaceData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimFractureSurface::createCopy()
{
    auto newSurface = copyObject<RimFractureSurface>();
    if ( !newSurface->onLoadData() )
    {
        delete newSurface;
        return nullptr;
    }

    return newSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::loadSurfaceDataForTimeStep( int timeStep )
{
    if ( m_surfacePerTimeStep.empty() )
    {
        loadDataFromFile();
    }

    if ( timeStep >= static_cast<int>( m_surfacePerTimeStep.size() ) )
    {
        QString message =
            QString( "Failed to load surface data for time step: %1. Available time steps: %2" ).arg( timeStep ).arg( m_surfacePerTimeStep.size() );
        RiaLogging::warning( message );
        setSurfaceData( nullptr );
        return;
    }

    auto surface = new RigSurface;

    auto triangleMeshData              = m_surfacePerTimeStep[timeStep].get();
    const auto& [coordinates, indices] = triangleMeshData->geometry();

    surface->setTriangleData( indices, coordinates );
    auto propertyNames = triangleMeshData->propertyNames();
    for ( const auto& name : propertyNames )
    {
        auto values = triangleMeshData->propertyValues( name );
        surface->addVertexResult( name, values );
    }
    setSurfaceData( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFractureSurface::timeStepCount() const
{
    return m_surfacePerTimeStep.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimFractureSurface::valuesForProperty( const QString& propertyName ) const
{
    std::vector<std::vector<double>> values;
    for ( const auto& allTimeStepValues : m_surfacePerTimeStep )
    {
        const auto& [coordinates, indices] = allTimeStepValues->geometry();

        auto valuesOneTimeStep = allTimeStepValues->propertyValues( propertyName );

        // convert to double vector
        std::vector<double> valuesOneTimeStepDouble( valuesOneTimeStep.begin(), valuesOneTimeStep.end() );

        values.push_back( valuesOneTimeStepDouble );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSurface::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_surfaceDefinitionFilePath )
    {
        clearCachedNativeData();
        updateSurfaceData();

        auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();
        surfColl->updateViews( { this } );
    }
}

//--------------------------------------------------------------------------------------------------
/// Regenerate the surface geometry, using the offset specified.
/// If the surface data hasn't been loaded from file yet, load it.
/// Returns false for fatal failure
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::updateSurfaceData()
{
    loadSurfaceDataForTimeStep( 0 );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::clearCachedNativeData()
{
    m_secondsSinceSimulationStart.clear();
    m_surfacePerTimeStep.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::loadDataFromFile()
{
    std::filesystem::path filepath    = m_surfaceDefinitionFilePath().path().toLocal8Bit().constData();
    auto                  surfaceInfo = RifVtkImportUtil::parsePvdDatasets( filepath );

    for ( const auto& s : surfaceInfo )
    {
        if ( auto triangleMeshData = RifVtkSurfaceImporter::importFromFile( s.filepath ) )
        {
            m_secondsSinceSimulationStart.push_back( s.timestep );
            m_surfacePerTimeStep.push_back( std::move( triangleMeshData ) );
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureSurface::updateMinMaxValues( RimRegularLegendConfig* legendConfig, const QString& propertyName, int currentTimeStep ) const
{
    double localMin              = 0.0;
    double localMax              = 0.0;
    double localPosClosestToZero = 0.0;
    double localNegClosestToZero = 0.0;

    auto valuesForTimeSteps = valuesForProperty( propertyName );

    MinMaxAccumulator minMaxAccumulator;
    PosNegAccumulator posNegAccumulator;

    for ( size_t timeIndex = 0; timeIndex < valuesForTimeSteps.size(); timeIndex++ )
    {
        auto values = valuesForTimeSteps[timeIndex];
        minMaxAccumulator.addData( values );
        posNegAccumulator.addData( values );

        if ( static_cast<int>( timeIndex ) == currentTimeStep )
        {
            MinMaxAccumulator localMinMaxAccumulator;
            PosNegAccumulator localPosNegAccumulator;
            localMinMaxAccumulator.addData( values );
            localPosNegAccumulator.addData( values );

            localPosClosestToZero = localPosNegAccumulator.pos;
            localNegClosestToZero = localPosNegAccumulator.neg;
            localMin              = localMinMaxAccumulator.min;
            localMax              = localMinMaxAccumulator.max;
        }
    }

    double globalPosClosestToZero = posNegAccumulator.pos;
    double globalNegClosestToZero = posNegAccumulator.neg;
    double globalMin              = minMaxAccumulator.min;
    double globalMax              = minMaxAccumulator.max;

    legendConfig->setClosestToZeroValues( globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero );
    legendConfig->setAutomaticRanges( globalMin, globalMax, localMin, localMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureSurface::isMeshLinesEnabledDefault() const
{
    return true;
}
