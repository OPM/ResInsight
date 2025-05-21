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

#include "RifVtkReader.h"

#include "RifVtkImportUtil.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemTypes.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "cafProgressInfo.h"

#include <QString>

#include <iostream>
#include <limits>
#include <map>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVtkReader::RifVtkReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVtkReader::~RifVtkReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::close()
{
    //    m_stream.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifVtkReader::populateDerivedResultNames() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifVtkReader::openFile( const std::string& fileName, std::string* errorMessage )
{
    std::filesystem::path filePath( fileName );

    auto dataset = RifVtkImportUtil::parsePvdDatasets( filePath );
    if ( dataset.empty() )
    {
        RiaLogging::error( "No timesteps found" );
        m_inputPath.clear();
        return false;
    }
    else
    {
        RiaLogging::info( QString( "Found %1 timesteps" ).arg( dataset.size() ) );
        m_inputPath = filePath;
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifVtkReader::isOpen() const
{
    return !m_inputPath.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifVtkReader::readFemParts( RigFemPartCollection* femParts )
{
    CVF_ASSERT( femParts );

    // The key in the maps is the part ID
    std::map<int, std::string>                                              parts;
    std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>                  nodes;
    std::map<int, std::vector<std::pair<int, std::vector<int>>>>            elements;
    std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>> elementSets;

    auto result = read( m_inputPath, parts, nodes, elements, elementSets, m_stepNames );
    if ( !result )
    {
        RiaLogging::error( QString::fromStdString( result.error() ) );
        return false;
    }

    auto elementType = result.value();

    if ( m_stepNames.empty() ) m_stepNames.push_back( "Geostatic" );

    RiaLogging::debug( QString( "Read FEM parts: %1, steps: %2, element type: %3" )
                           .arg( parts.size() )
                           .arg( m_stepNames.size() )
                           .arg( QString::fromStdString( RigFemTypes::elementTypeText( elementType ) ) ) );

    if ( !RigFemTypes::is8NodeElement( elementType ) )
    {
        RiaLogging::error( QString( "Unsupported element type." ) );
        return false;
    }

    caf::ProgressInfo modelProgress( parts.size() * 2, "Reading Inp Parts" );

    for ( const auto& [partId, partName] : parts )
    {
        modelProgress.setProgressDescription( QString::fromStdString( partName ) + ": Reading Nodes" );

        RigFemPart* femPart = new RigFemPart;
        femPart->setName( partName );

        // Extract nodes
        std::vector<std::pair<int, cvf::Vec3d>> partNodes = nodes[partId];

        std::map<int, int> nodeIdToIdxMap;

        int nodeCount = partNodes.size();
        femPart->nodes().nodeIds.resize( nodeCount );
        femPart->nodes().coordinates.resize( nodeCount );

        for ( int nIdx = 0; nIdx < nodeCount; ++nIdx )
        {
            auto [nodeId, pos]             = partNodes[nIdx];
            femPart->nodes().nodeIds[nIdx] = nodeId;
            femPart->nodes().coordinates[nIdx].set( pos[0], pos[1], pos[2] );
            nodeIdToIdxMap[nodeId] = nIdx;
        }

        modelProgress.incrementProgress();
        modelProgress.setProgressDescription( QString::fromStdString( partName ) + ": Reading Elements" );

        // Extract elements
        std::vector<std::pair<int, std::vector<int>>> partElements = elements[partId];

        int elmCount = partElements.size();
        femPart->preAllocateElementStorage( elmCount );

        std::vector<int> indexBasedConnectivities;

        std::map<int, int> elementIdToIdxMap;

        for ( int elmIdx = 0; elmIdx < elmCount; ++elmIdx )
        {
            auto [elmId, nodesInElement] = partElements[elmIdx];

            elementIdToIdxMap[elmId] = elmIdx;

            int nodeCount = RigFemTypes::elementNodeCount( elementType );

            indexBasedConnectivities.resize( nodeCount );
            for ( int lnIdx = 0; lnIdx < nodeCount; ++lnIdx )
            {
                indexBasedConnectivities[lnIdx] = nodeIdToIdxMap[nodesInElement[lnIdx]];
            }

            femPart->appendElement( elementType, elmId, indexBasedConnectivities.data() );
        }

        // read element sets
        auto& elementSetsForPart = elementSets[partId];
        for ( auto& [setName, elementSet] : elementSetsForPart )
        {
            femPart->addElementSet( setName, elementSet );
        }

        femPart->setElementPartId( femParts->partCount() );
        femParts->addFemPart( femPart );

        modelProgress.incrementProgress();
    }

    //    readScalarData( femParts, parts, m_includeEntries );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigElementType, std::string>
    RifVtkReader::read( const std::filesystem::path&                                             filePath,
                        std::map<int, std::string>&                                              parts,
                        std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>&                  nodes,
                        std::map<int, std::vector<std::pair<int, std::vector<int>>>>&            elements,
                        std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>>& elementSets,
                        std::vector<std::string>&                                                stepNames )
{
    RigElementType elementType = RigElementType::UNKNOWN_ELM_TYPE;

    int         partId = 0;
    std::string partName;
    int         stepId = -1;
    std::string stepName;
    int         timeSteps = 0;

    auto dataset = RifVtkImportUtil::parsePvdDatasets( filePath );

    // Only one part supported
    parts[partId] = "Model";

    //
    elementSets[partId] = {};

    bool isFirst = true;

    for ( const RifVtkImportUtil::PvdDataset& d : dataset )
    {
        if ( isFirst )
        {
            stepNames.push_back( "Dataset " + std::to_string( d.timestep ) );

            pugi::xml_document     doc;
            pugi::xml_parse_result result = doc.load_file( d.filepath.string().c_str() );
            if ( !result ) return std::unexpected( "File did load: " + d.filepath.string() );

            auto root = doc.child( "VTKFile" );
            if ( !root ) return std::unexpected( "Missing VTKFile tag: " + d.filepath.string() );

            auto grid = root.child( "UnstructuredGrid" );
            if ( !grid ) return std::unexpected( "Missing UnstructedGrid tag: " + d.filepath.string() );

            auto piece = grid.child( "Piece" );
            if ( !piece ) return std::unexpected( "Missing Piece tag: " + d.filepath.string() );

            // Read points
            std::vector<cvf::Vec3d> vertices = RifVtkImportUtil::readPoints( piece );
            if ( vertices.empty() ) return std::unexpected( "No points found: " + d.filepath.string() );

            // Read connectivity
            std::vector<unsigned> connectivity = RifVtkImportUtil::readConnectivity( piece );
            if ( connectivity.empty() ) return std::unexpected( "No connectivity found: " + d.filepath.string() );

            RiaLogging::info( QString( "Found %1 vertices and %2 connectivities" ).arg( vertices.size() ).arg( connectivity.size() ) );

            std::vector<std::pair<int, cvf::Vec3d>> partNodes;
            for ( size_t i = 0; i < vertices.size(); i++ )
            {
                partNodes.push_back( { i, vertices[i] } );
            }

            nodes[partId] = partNodes;

            std::vector<std::pair<int, std::vector<int>>> partElements;

            int numNodesPerElement = 8;
            int numCells           = connectivity.size() / numNodesPerElement;
            for ( int i = 0; i < numCells; i++ )
            {
                std::vector<int> els( numNodesPerElement );
                for ( int x = 0; x < numNodesPerElement; x++ )
                {
                    els[x] = connectivity[i * numNodesPerElement + x];
                }
                partElements.push_back( { i, els } );
            }

            elements[partId] = partElements;

            const std::map<std::string, std::vector<float>> properties = RifVtkImportUtil::readProperties( piece );
            for ( auto [key, values] : properties )
            {
                printf( "Prop: [%s] => [%zu]\n", key.c_str(), values.size() );
            }

            elementType = RigElementType::HEX8P;
        }

        isFirst = false;
    }

    return elementType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifVtkReader::allStepNames() const
{
    return m_stepNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifVtkReader::filteredStepNames() const
{
    // no filter supported
    return RifVtkReader::allStepNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifVtkReader::frameTimes( int stepIndex ) const
{
    // only one frame from INP file
    std::vector<double> frameValues( { 1.0 } );
    return frameValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifVtkReader::frameCount( int stepIndex ) const
{
    return frameTimes( stepIndex ).size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifVtkReader::elementSetNames( int partIndex, std::string partName )
{
    if ( partIndex >= (int)m_partElementSetNames.size() ) return {};
    return m_partElementSetNames.at( partIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifVtkReader::elementSet( int partIndex, std::string partName, int setIndex )
{
    // TODO: not implemented yet
    std::vector<size_t> elementIndexes;
    return elementIndexes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifVtkReader::scalarNodeFieldAndComponentNames()
{
    std::map<std::string, std::vector<std::string>> retVal;

    for ( auto& entry : m_propertyPartDataNodes )
    {
        retVal[entry.first] = {};
    }

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifVtkReader::scalarElementFieldAndComponentNames()
{
    std::map<std::string, std::vector<std::string>> retVal;

    for ( auto& entry : m_propertyPartDataElements )
    {
        retVal[entry.first] = {};
    }

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifVtkReader::scalarElementNodeFieldAndComponentNames()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifVtkReader::scalarIntegrationPointFieldAndComponentNames()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readDisplacements( int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements )
{
    CVF_ASSERT( displacements );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readField( RigFemResultPosEnum               resultType,
                              const std::string&                fieldName,
                              int                               partIndex,
                              int                               stepIndex,
                              std::vector<std::vector<float>*>* resultValues )
{
    CVF_ASSERT( resultValues );

    auto dataMap = propertyDataMap( resultType );
    if ( dataMap == nullptr ) return;

    if ( dataMap->count( fieldName ) == 0 ) return;

    // is there only a static result? Use it for all steps.
    if ( ( *dataMap )[fieldName].count( stepIndex ) == 0 )
    {
        if ( ( *dataMap )[fieldName].count( -1 ) == 0 ) return;
        stepIndex = -1;
    }
    if ( ( *dataMap )[fieldName][stepIndex].count( partIndex ) == 0 ) return;

    auto dataSize = ( *dataMap )[fieldName][stepIndex][partIndex].size();

    ( *resultValues )[0]->resize( dataSize );

    std::vector<float>* singleComponentValues = ( *resultValues )[0];
    for ( size_t i = 0; i < dataSize; i++ )
    {
        ( *singleComponentValues )[i] = (float)( *dataMap )[fieldName][stepIndex][partIndex][i];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readNodeField( const std::string&                fieldName,
                                  int                               partIndex,
                                  int                               stepIndex,
                                  int                               frameIndex,
                                  std::vector<std::vector<float>*>* resultValues )
{
    readField( RigFemResultPosEnum::RIG_NODAL, fieldName, partIndex, stepIndex, resultValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readElementField( const std::string&                fieldName,
                                     int                               partIndex,
                                     int                               stepIndex,
                                     int                               frameIndex,
                                     std::vector<std::vector<float>*>* resultValues )
{
    readField( RigFemResultPosEnum::RIG_ELEMENT, fieldName, partIndex, stepIndex, resultValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readElementNodeField( const std::string&                fieldName,
                                         int                               partIndex,
                                         int                               stepIndex,
                                         int                               frameIndex,
                                         std::vector<std::vector<float>*>* resultValues )
{
    readField( RigFemResultPosEnum::RIG_ELEMENT_NODAL, fieldName, partIndex, stepIndex, resultValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readIntegrationPointField( const std::string&                fieldName,
                                              int                               partIndex,
                                              int                               stepIndex,
                                              int                               frameIndex,
                                              std::vector<std::vector<float>*>* resultValues )
{
    readField( RigFemResultPosEnum::RIG_INTEGRATION_POINT, fieldName, partIndex, stepIndex, resultValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RifVtkReader::readScalarData( RigFemPartCollection*            femParts,
//                                    std::map<int, std::string>&      parts,
//                                    std::vector<RifInpIncludeEntry>& includeEntries )
// {
//     for ( auto& entry : includeEntries )
//     {
//         auto map = propertyDataMap( entry.resultType );
//         if ( map == nullptr ) continue;
//         if ( map->count( entry.propertyName ) == 0 )
//         {
//             ( *map )[entry.propertyName] = {};
//         }

//         int stepId = entry.stepId;

//         if ( ( *map )[entry.propertyName].count( stepId ) == 0 )
//         {
//             ( *map )[entry.propertyName][stepId] = {};
//         }

//         for ( int partId = 0; partId < femParts->partCount(); partId++ )
//         {
//             ( *map )[entry.propertyName][stepId][partId] = {};
//             size_t dataSize                              = 0;
//             if ( entry.resultType == RigFemResultPosEnum::RIG_NODAL )
//             {
//                 dataSize = femParts->part( partId )->nodeCount();
//             }
//             if ( entry.resultType == RigFemResultPosEnum::RIG_ELEMENT )
//             {
//                 dataSize = femParts->part( partId )->elementCount();
//             }
//             ( *map )[entry.propertyName][stepId][partId].resize( dataSize, 0.0 );
//         }
//         RifInpIncludeReader reader;
//         if ( !reader.openFile( entry.fileName ) ) continue;
//         reader.readData( entry.columnIndex, parts, ( *map )[entry.propertyName][stepId] );
//     }

//     return;
// }

//--------------------------------------------------------------------------------------------------
///  Map keys: result name / time step / part id
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::map<int, std::map<int, std::vector<double>>>>* RifVtkReader::propertyDataMap( RigFemResultPosEnum resultType )
{
    if ( resultType == RigFemResultPosEnum::RIG_ELEMENT ) return &m_propertyPartDataElements;
    if ( resultType == RigFemResultPosEnum::RIG_NODAL ) return &m_propertyPartDataNodes;

    return nullptr;
}
