/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigNNCData.h"

#include "RiaLogging.h"
#include "RiaPreferencesSystem.h"

#include "RigCellFaceGeometryTools.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "cafProgressInfo.h"

#include "cvfGeometryTools.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNNCData::RigNNCData()
    : m_eclipseConnectionCount( 0 )
    , m_havePolygonsForEclipseConnections( false )
    , m_haveGeneratedConnections( false )
    , m_mainGrid( nullptr )
    , m_activeCellInfo( nullptr )
    , m_computeNncForInactiveCells( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::setSourceDataForProcessing( RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, bool includeInactiveCells )
{
    m_mainGrid                   = mainGrid;
    m_activeCellInfo             = activeCellInfo;
    m_computeNncForInactiveCells = includeInactiveCells;

    m_havePolygonsForEclipseConnections = false;
    m_haveGeneratedConnections          = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::buildPolygonsForEclipseConnections()
{
    if ( m_havePolygonsForEclipseConnections ) return;
    if ( !m_mainGrid ) return;

#pragma omp parallel for
    for ( int cnIdx = 0; cnIdx < static_cast<int>( eclipseConnectionCount() ); ++cnIdx )
    {
        const RigCell& c1 = m_mainGrid->cell( m_connections[cnIdx].c1GlobIdx() );
        const RigCell& c2 = m_mainGrid->cell( m_connections[cnIdx].c2GlobIdx() );

        std::vector<size_t>                connectionPolygon;
        std::vector<cvf::Vec3d>            connectionIntersections;
        cvf::StructGridInterface::FaceType connectionFace = cvf::StructGridInterface::NO_FACE;

        connectionFace =
            RigCellFaceGeometryTools::calculateCellFaceOverlap( c1, c2, *m_mainGrid, &connectionPolygon, &connectionIntersections );

        if ( connectionFace != cvf::StructGridInterface::NO_FACE )
        {
            m_connections[cnIdx].setFace( connectionFace );
            m_connections[cnIdx].setPolygon(
                RigCellFaceGeometryTools::extractPolygon( m_mainGrid->nodes(), connectionPolygon, connectionIntersections ) );
        }
    }

    m_havePolygonsForEclipseConnections = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::computeAdditionalNncs( const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, bool includeInactiveCells )
{
    RigConnectionContainer otherConnections =
        RigCellFaceGeometryTools::computeOtherNncs( mainGrid, m_connections, activeCellInfo, includeInactiveCells );

    if ( !otherConnections.empty() )
    {
        m_connections.push_back( otherConnections );

        // Transmissibility values from Eclipse has been read into propertyNameCombTrans in
        // RifReaderEclipseOutput::transferStaticNNCData(). Initialize computed NNCs with zero transmissibility
        auto it = m_connectionResults.find( RiaDefines::propertyNameCombTrans() );
        if ( it != m_connectionResults.end() )
        {
            if ( !it->second.empty() )
            {
                it->second[0].resize( m_connections.size(), 0.0 );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNNCData::connectionsWithNoCommonArea( QStringList& connectionTextFirstItems, size_t maxItemCount )
{
    size_t connectionWithNoCommonAreaCount = 0;

    for ( size_t connIndex = 0; connIndex < m_connections.size(); connIndex++ )
    {
        if ( !m_connections[connIndex].hasCommonArea() )
        {
            connectionWithNoCommonAreaCount++;

            if ( connectionTextFirstItems.size() < static_cast<int>( maxItemCount ) )
            {
                QString firstConnectionText;
                QString secondConnectionText;

                {
                    size_t             gridLocalCellIndex;
                    const RigGridBase* hostGrid =
                        m_mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( m_connections[connIndex].c1GlobIdx(), &gridLocalCellIndex );

                    size_t i, j, k;
                    if ( hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
                    {
                        // Adjust to 1-based Eclipse indexing
                        i++;
                        j++;
                        k++;

                        if ( !hostGrid->isMainGrid() )
                        {
                            QString gridName    = QString::fromStdString( hostGrid->gridName() );
                            firstConnectionText = gridName + " ";
                        }
                        firstConnectionText += QString( "[%1 %2 %3] - " ).arg( i ).arg( j ).arg( k );
                    }
                }

                {
                    size_t             gridLocalCellIndex;
                    const RigGridBase* hostGrid =
                        m_mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( m_connections[connIndex].c2GlobIdx(), &gridLocalCellIndex );

                    size_t i, j, k;
                    if ( hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
                    {
                        // Adjust to 1-based Eclipse indexing
                        i++;
                        j++;
                        k++;

                        if ( !hostGrid->isMainGrid() )
                        {
                            QString gridName     = QString::fromStdString( hostGrid->gridName() );
                            secondConnectionText = gridName + " ";
                        }
                        secondConnectionText += QString( "[%1 %2 %3]" ).arg( i ).arg( j ).arg( k );
                    }
                }

                connectionTextFirstItems.push_back( firstConnectionText + secondConnectionText );
            }
        }
    }

    return connectionWithNoCommonAreaCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNNCData::ensureAllConnectionDataIsProcessed()
{
    if ( m_haveGeneratedConnections ) return false;

    // Return false if we have no data to process to avoid recursive updates in consuming code
    if ( !m_mainGrid ) return false;

    if ( m_mainGrid )
    {
        caf::ProgressInfo progressInfo( 3, "Computing NNC Data" );

        const QString logKeyword = "RigNNCData";
        RiaLogging::info( "NNC geometry computation - starting process", logKeyword );

        buildPolygonsForEclipseConnections();
        progressInfo.incrementProgress();

        computeAdditionalNncs( m_mainGrid, m_activeCellInfo, m_computeNncForInactiveCells );
        progressInfo.incrementProgress();

        m_haveGeneratedConnections = true;

        m_mainGrid->distributeNNCsToFaults();

        QStringList  noCommonAreaText;
        const size_t maxItemCount = 20;

        size_t noCommonAreaCount = connectionsWithNoCommonArea( noCommonAreaText, maxItemCount );

        RiaLogging::info( "NNC geometry computation - completed process", logKeyword );

        RiaLogging::info( QString( "Native NNC count : %1" ).arg( eclipseConnectionCount() ), logKeyword );
        RiaLogging::info( QString( "Computed NNC count : %1" ).arg( m_connections.size() ), logKeyword );

        RiaLogging::info( QString( "NNCs with no common area count : %1" ).arg( noCommonAreaCount ), logKeyword );

        if ( !noCommonAreaText.isEmpty() )
        {
            RiaLogging::info( QString( "Listing first %1 NNCs with no common area " ).arg( noCommonAreaText.size() ), logKeyword );

            for ( const auto& s : noCommonAreaText )
            {
                RiaLogging::info( s, logKeyword );
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::setEclipseConnections( RigConnectionContainer& eclipseConnections )
{
    m_connections            = eclipseConnections;
    m_eclipseConnectionCount = m_connections.size();

    m_haveGeneratedConnections = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigNNCData::eclipseConnectionCount() const
{
    return m_eclipseConnectionCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigConnectionContainer& RigNNCData::availableConnections() const
{
    // Return connections without calling ensureConnectionDataIsProcessed() to avoid potential heavy computations
    // Relevant if only native connection data is required
    // NB: If computeAdditionalNncs() is called before this method, the size of this collection is larger than
    // nativeConnectionCount()

    return m_connections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigConnectionContainer& RigNNCData::allConnections()
{
    ensureAllConnectionDataIsProcessed();

    return m_connections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>& RigNNCData::makeStaticConnectionScalarResult( QString nncDataType )
{
    ensureAllConnectionDataIsProcessed();

    std::vector<std::vector<double>>& results = m_connectionResults[nncDataType];
    results.resize( 1 );
    results[0].resize( m_connections.size(), HUGE_VAL );
    return results[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::makeScalarResultAndSetValues( const QString& nncDataType, const std::vector<double>& values )
{
    std::vector<std::vector<double>>& results = m_connectionResults[nncDataType];
    results.resize( 1 );
    results[0] = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::staticConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    std::map<QString, std::vector<std::vector<double>>>::const_iterator it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        CVF_ASSERT( it->second.size() == 1 );
        return &( it->second[0] );
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::staticConnectionScalarResultByName( const QString& nncDataType ) const
{
    std::map<QString, std::vector<std::vector<double>>>::const_iterator it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        CVF_ASSERT( it->second.size() == 1 );
        return &( it->second[0] );
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>& RigNNCData::makeDynamicConnectionScalarResult( QString nncDataType, size_t timeStepCount )
{
    auto& results = m_connectionResults[nncDataType];
    results.resize( timeStepCount );
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>* RigNNCData::dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    auto it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        return &( it->second );
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep ) const
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    auto it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        if ( it->second.size() > timeStep )
        {
            return &( it->second[timeStep] );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>* RigNNCData::dynamicConnectionScalarResultByName( const QString& nncDataType ) const
{
    auto it = m_connectionResults.find( nncDataType );
    if ( it != m_connectionResults.end() )
    {
        return &( it->second );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::dynamicConnectionScalarResultByName( const QString& nncDataType, size_t timeStep ) const
{
    auto it = m_connectionResults.find( nncDataType );
    if ( it != m_connectionResults.end() )
    {
        if ( it->second.size() > timeStep )
        {
            return &( it->second[timeStep] );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>& RigNNCData::makeGeneratedConnectionScalarResult( QString nncDataType, size_t timeStepCount )
{
    auto& results = m_connectionResults[nncDataType];
    results.resize( timeStepCount );
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>* RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    auto it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        return &( it->second );
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep ) const
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    auto it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        if ( it->second.size() > timeStep )
        {
            return &( it->second[timeStep] );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>* RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr )
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    auto it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        return &( it->second );
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep )
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return nullptr;

    auto it = m_connectionResults.find( nncDataType );

    if ( it != m_connectionResults.end() )
    {
        if ( it->second.size() > timeStep )
        {
            return &( it->second[timeStep] );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>* RigNNCData::generatedConnectionScalarResultByName( const QString& nncDataType ) const
{
    auto it = m_connectionResults.find( nncDataType );
    if ( it != m_connectionResults.end() )
    {
        return &( it->second );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigNNCData::generatedConnectionScalarResultByName( const QString& nncDataType, size_t timeStep ) const
{
    auto it = m_connectionResults.find( nncDataType );
    if ( it != m_connectionResults.end() )
    {
        if ( it->second.size() > timeStep )
        {
            return &( it->second[timeStep] );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>* RigNNCData::generatedConnectionScalarResultByName( const QString& nncDataType )
{
    auto it = m_connectionResults.find( nncDataType );
    if ( it != m_connectionResults.end() )
    {
        return &( it->second );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigNNCData::generatedConnectionScalarResultByName( const QString& nncDataType, size_t timeStep )
{
    auto it = m_connectionResults.find( nncDataType );
    if ( it != m_connectionResults.end() )
    {
        if ( it->second.size() > timeStep )
        {
            return &( it->second[timeStep] );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigNNCData::availableProperties( NNCResultType resultType ) const
{
    std::vector<QString> properties;

    for ( auto it : m_connectionResults )
    {
        if ( resultType == NNCResultType::NNC_STATIC && it.second.size() == 1 && !it.second[0].empty() && isNative( it.first ) )
        {
            properties.push_back( it.first );
        }
        else if ( resultType == NNCResultType::NNC_DYNAMIC && it.second.size() > 1 && !it.second[0].empty() && isNative( it.first ) )
        {
            properties.push_back( it.first );
        }
        else if ( resultType == NNCResultType::NNC_GENERATED && !isNative( it.first ) )
        {
            properties.push_back( it.first );
        }
    }

    return properties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::setEclResultAddress( const QString& nncDataType, const RigEclipseResultAddress& resVarAddr )
{
    m_resultAddrToNNCDataType[resVarAddr] = nncDataType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNNCData::hasScalarValues( const RigEclipseResultAddress& resVarAddr )
{
    QString nncDataType = getNNCDataTypeFromScalarResultIndex( resVarAddr );
    if ( nncDataType.isNull() ) return false;

    auto it = m_connectionResults.find( nncDataType );
    return ( it != m_connectionResults.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNNCData::generateScalarValues( const RigEclipseResultAddress& resVarAddr )
{
    if ( hasScalarValues( resVarAddr ) ) return true;

    if ( resVarAddr.isDivideByCellFaceAreaActive() && resVarAddr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
    {
        RigEclipseResultAddress tmpAddr = resVarAddr;
        tmpAddr.enableDivideByCellFaceArea( false );

        auto nameit = m_resultAddrToNNCDataType.find( tmpAddr );
        if ( nameit == m_resultAddrToNNCDataType.end() ) return false;

        auto it = m_connectionResults.find( nameit->second );
        if ( it == m_connectionResults.end() ) return false;

        // Connection polygons are used to compute the center for the NNC flow vectors
        // If connection polygons are present, this is a no-op
        buildPolygonsForEclipseConnections();

        auto& srcdata = it->second;
        auto& dstdata = makeDynamicConnectionScalarResult( resVarAddr.resultName(), srcdata.size() );

        const double        epsilon = 1.0e-3;
        std::vector<double> areas( m_connections.size() );

        for ( size_t dataIdx = 0; dataIdx < m_connections.size(); dataIdx++ )
        {
            double area = 0.0;
            if ( m_connections[dataIdx].hasCommonArea() ) area = cvf::GeometryTools::polygonArea( m_connections[dataIdx].polygon() );
            areas[dataIdx] = area;
        }

#pragma omp parallel for
        for ( int i = 0; i < static_cast<int>( srcdata.size() ); i++ )
        {
            size_t timeIdx = i;
            dstdata[timeIdx].resize( srcdata[timeIdx].size() );

            for ( size_t dataIdx = 0; dataIdx < srcdata[timeIdx].size(); dataIdx++ )
            {
                double scaledVal = 0.0;
                if ( areas[dataIdx] > epsilon ) scaledVal = srcdata[timeIdx][dataIdx] / areas[dataIdx];
                dstdata[timeIdx][dataIdx] = scaledVal;
            }
        }

        m_resultAddrToNNCDataType[resVarAddr] = resVarAddr.resultName();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString RigNNCData::getNNCDataTypeFromScalarResultIndex( const RigEclipseResultAddress& resVarAddr ) const
{
    auto it = m_resultAddrToNNCDataType.find( resVarAddr );
    if ( it != m_resultAddrToNNCDataType.end() )
    {
        return it->second;
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigNNCData::isNative( QString nncDataType ) const
{
    return nncDataType == RiaDefines::propertyNameCombTrans() || nncDataType == RiaDefines::propertyNameFluxGas() ||
           nncDataType == RiaDefines::propertyNameFluxOil() || nncDataType == RiaDefines::propertyNameFluxWat() ||
           nncDataType == RiaDefines::propertyNameRiCombMult() || nncDataType == RiaDefines::propertyNameRiCombTrans() ||
           nncDataType == RiaDefines::propertyNameRiCombTransByArea();
}
