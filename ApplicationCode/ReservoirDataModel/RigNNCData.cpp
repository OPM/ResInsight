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

#include "RigCellFaceGeometryTools.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNNCData::RigNNCData() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::processNativeConnections( const RigMainGrid& mainGrid )
{
    // cvf::Trace::show("NNC: Total number: " + cvf::String((int)m_connections.size()));

    for ( size_t cnIdx = 0; cnIdx < m_connections.size(); ++cnIdx )
    {
        const RigCell& c1 = mainGrid.globalCellArray()[m_connections[cnIdx].m_c1GlobIdx];
        const RigCell& c2 = mainGrid.globalCellArray()[m_connections[cnIdx].m_c2GlobIdx];

        std::vector<size_t>                connectionPolygon;
        std::vector<cvf::Vec3d>            connectionIntersections;
        cvf::StructGridInterface::FaceType connectionFace = cvf::StructGridInterface::NO_FACE;

        connectionFace = RigCellFaceGeometryTools::calculateCellFaceOverlap( c1,
                                                                             c2,
                                                                             mainGrid,
                                                                             &connectionPolygon,
                                                                             &connectionIntersections );

        if ( connectionFace != cvf::StructGridInterface::NO_FACE )
        {
            // Found an overlap polygon. Store data about connection

            m_connections[cnIdx].m_c1Face = connectionFace;

            m_connections[cnIdx].m_polygon = RigCellFaceGeometryTools::extractPolygon( mainGrid.nodes(),
                                                                                       connectionPolygon,
                                                                                       connectionIntersections );

            // Add to search map, possibly not needed
            // m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c1GlobIdx][connectionFace].push_back(cnIdx);
            // m_cellIdxToFaceToConnectionIdxMap[m_connections[cnIdx].m_c2GlobIdx][cvf::StructGridInterface::oppositeFace(connectionFace].push_back(cnIdx);
        }
        else
        {
            // cvf::Trace::show("NNC: No overlap found for : C1: " + cvf::String((int)m_connections[cnIdx].m_c1GlobIdx)
            // + "C2: " + cvf::String((int)m_connections[cnIdx].m_c2GlobIdx));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::computeCompleteSetOfNncs( const RigMainGrid* mainGrid )
{
    m_nativeConnectionCount = m_connections.size();

    std::vector<RigConnection> otherConnections = RigCellFaceGeometryTools::computeOtherNncs( mainGrid, m_connections );

    m_connections.insert( m_connections.end(), otherConnections.begin(), otherConnections.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigNNCData::setConnections( std::vector<RigConnection>& connections )
{
    m_connections = connections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigConnection>& RigNNCData::connections() const
{
    return m_connections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>& RigNNCData::makeStaticConnectionScalarResult( QString nncDataType )
{
    std::vector<std::vector<double>>& results = m_connectionResults[nncDataType];
    results.resize( 1 );
    results[0].resize( m_connections.size(), HUGE_VAL );
    return results[0];
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
std::vector<std::vector<double>>& RigNNCData::makeDynamicConnectionScalarResult( QString nncDataType,
                                                                                 size_t  timeStepCount )
{
    auto& results = m_connectionResults[nncDataType];
    results.resize( timeStepCount );
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>*
    RigNNCData::dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const
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
const std::vector<double>* RigNNCData::dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr,
                                                                      size_t                         timeStep ) const
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
const std::vector<double>* RigNNCData::dynamicConnectionScalarResultByName( const QString& nncDataType,
                                                                            size_t         timeStep ) const
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
std::vector<std::vector<double>>& RigNNCData::makeGeneratedConnectionScalarResult( QString nncDataType,
                                                                                   size_t  timeStepCount )
{
    auto& results = m_connectionResults[nncDataType];
    results.resize( timeStepCount );
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>*
    RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const
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
const std::vector<double>* RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr,
                                                                        size_t                         timeStep ) const
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
std::vector<double>* RigNNCData::generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr,
                                                                  size_t                         timeStep )
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
const std::vector<double>* RigNNCData::generatedConnectionScalarResultByName( const QString& nncDataType,
                                                                              size_t         timeStep ) const
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
        if ( resultType == NNC_STATIC && it.second.size() == 1 && it.second[0].size() > 0 && isNative( it.first ) )
        {
            properties.push_back( it.first );
        }
        else if ( resultType == NNC_DYNAMIC && it.second.size() > 1 && it.second[0].size() > 0 && isNative( it.first ) )
        {
            properties.push_back( it.first );
        }
        else if ( resultType == NNC_GENERATED && !isNative( it.first ) )
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
    if ( nncDataType == RiaDefines::propertyNameCombTrans() || nncDataType == RiaDefines::propertyNameFluxGas() ||
         nncDataType == RiaDefines::propertyNameFluxOil() || nncDataType == RiaDefines::propertyNameFluxWat() ||
         nncDataType == RiaDefines::propertyNameRiCombMult() || nncDataType == RiaDefines::propertyNameRiCombTrans() ||
         nncDataType == RiaDefines::propertyNameRiCombTransByArea() )
    {
        return true;
    }
    return false;
}
