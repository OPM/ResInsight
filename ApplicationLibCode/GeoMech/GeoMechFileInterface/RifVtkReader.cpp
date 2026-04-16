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
#include "RigFemResultPosEnum.h"
#include "RigFemTypes.h"

#include "RiaLogging.h"

#include "cafProgressInfo.h"

#include <QString>

#include <map>

namespace RifVtkFieldNames
{
constexpr const char* separator = "__";

constexpr const char* S   = "S_VTK";
constexpr const char* S11 = "S11";
constexpr const char* S22 = "S22";
constexpr const char* S33 = "S33";
constexpr const char* S12 = "S12";
constexpr const char* S13 = "S13";
constexpr const char* S23 = "S23";

constexpr const char* E   = "E_VTK";
constexpr const char* E11 = "E11";
constexpr const char* E22 = "E22";
constexpr const char* E33 = "E33";
constexpr const char* E12 = "E12";
constexpr const char* E13 = "E13";
constexpr const char* E23 = "E23";
} // namespace RifVtkFieldNames

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
    std::map<int, std::vector<std::map<std::string, std::vector<float>>>>   properties;

    auto result = read( m_inputPath, parts, nodes, elements, elementSets, properties, m_displacements, m_stepNames );
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

        int nodeCount = static_cast<int>( partNodes.size() );
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

        int elmCount = static_cast<int>( partElements.size() );
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

        // VTK coordinates are negated to match ResInsight internal convention (z positive downward),
        // which flips the IJK directions. Invert all so IJK=0 is at the correct corner.
        femPart->setInvertIJKInStructGrid( true );

        femPart->setElementPartId( femParts->partCount() );
        femParts->addFemPart( femPart );

        modelProgress.incrementProgress();
    }

    size_t elementCount = femParts->part( 0 )->elementCount();

    readScalarData( femParts, parts, properties, elementCount );

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
                        std::map<int, std::vector<std::map<std::string, std::vector<float>>>>&   properties,
                        std::map<int, std::vector<std::vector<cvf::Vec3f>>>&                     displacements,
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

        // Use the geometry from the first time step only.
        // The geometry is assumed to not change between time steps.
        if ( isFirst )
        {
            // Read points
            std::vector<cvf::Vec3d> vertices = RifVtkImportUtil::readPoints( piece );
            if ( vertices.empty() ) return std::unexpected( "No points found: " + d.filepath.string() );

            // Read connectivity
            std::vector<unsigned> connectivity = RifVtkImportUtil::readConnectivity( piece );
            if ( connectivity.empty() ) return std::unexpected( "No connectivity found: " + d.filepath.string() );

            std::vector<std::pair<int, cvf::Vec3d>> partNodes;
            for ( int i = 0; i < static_cast<int>( vertices.size() ); i++ )
            {
                partNodes.push_back( { i, vertices[i] } );
            }

            nodes[partId] = partNodes;

            std::vector<std::pair<int, std::vector<int>>> partElements;

            int numNodesPerElement = 8;
            int numCells           = static_cast<int>( connectivity.size() / numNodesPerElement );
            for ( int i = 0; i < numCells; i++ )
            {
                std::vector<int> els( numNodesPerElement );
                for ( int x = 0; x < numNodesPerElement; x++ )
                {
                    els[x] = connectivity[i * numNodesPerElement + x];
                }
                partElements.emplace_back( i, els );
            }

            elements[partId] = partElements;

            elementType = RigElementType::HEX8P;
        }

        // Read displacements for all timesteps
        std::vector<cvf::Vec3f> partDisplacements = RifVtkImportUtil::readDisplacements( piece );
        if ( partDisplacements.empty() ) return std::unexpected( "No displacements found: " + d.filepath.string() );

        const std::map<std::string, std::vector<float>> partProperties = RifVtkImportUtil::readProperties( piece );

        RiaLogging::info( QString( "Found %1 displacements and %2 properties for timestep %3." )
                              .arg( partDisplacements.size() )
                              .arg( partProperties.size() )
                              .arg( d.timestep ) );

        properties[partId].push_back( partProperties );
        displacements[partId].push_back( partDisplacements );

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
    return static_cast<int>( frameTimes( stepIndex ).size() );
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

    retVal["U"] = { "U1", "U2", "U3" };

    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifVtkReader::scalarElementFieldAndComponentNames()
{
    using namespace RifVtkFieldNames;

    std::map<std::string, std::vector<std::string>> retVal;

    for ( auto& entry : m_propertyPartDataElements )
    {
        // Skip internal S/E component keys (stored as "S_VTK__S11", "E_VTK__E11", etc.)
        const std::string& key = entry.first;
        if ( key.starts_with( std::string( S ) + separator ) || key.starts_with( std::string( E ) + separator ) ) continue;

        retVal[key] = {};
    }

    // Add S field with components if any S component data was loaded
    if ( m_propertyPartDataElements.count( std::string( S ) + separator + S11 ) > 0 )
    {
        retVal[S] = { S11, S22, S33, S12, S13, S23 };
    }

    // Add E field with components if any E component data was loaded
    if ( m_propertyPartDataElements.count( std::string( E ) + separator + E11 ) > 0 )
    {
        retVal[E] = { E11, E22, E33, E12, E13, E23 };
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
    if ( auto it = m_displacements.find( partIndex ); it != m_displacements.end() )
    {
        *displacements = it->second[stepIndex];
    }
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
    CVF_ASSERT( resultValues );

    if ( fieldName == "U" )
    {
        std::vector<cvf::Vec3f> disp;
        readDisplacements( partIndex, stepIndex, frameIndex, &disp );
        if ( disp.empty() ) return;

        size_t n = disp.size();

        for ( int comp = 0; comp < 3 && comp < static_cast<int>( resultValues->size() ); comp++ )
        {
            if ( ( *resultValues )[comp] == nullptr ) continue;
            ( *resultValues )[comp]->resize( n );
            for ( size_t i = 0; i < n; i++ )
            {
                ( *( *resultValues )[comp] )[i] = disp[i][comp];
            }
        }
        return;
    }

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
    // S and E fields have 6 components stored as individual internal keys with a separator
    if ( fieldName == RifVtkFieldNames::S || fieldName == RifVtkFieldNames::E )
    {
        using namespace RifVtkFieldNames;
        static const std::vector<std::string> sComponents = { S11, S22, S33, S12, S13, S23 };
        static const std::vector<std::string> eComponents = { E11, E22, E33, E12, E13, E23 };
        const auto&                           components  = ( fieldName == RifVtkFieldNames::S ) ? sComponents : eComponents;
        size_t                                count       = std::min( components.size(), resultValues->size() );
        for ( size_t i = 0; i < count; i++ )
        {
            std::string                      internalKey = fieldName + separator + components[i];
            std::vector<std::vector<float>*> singleComp  = { ( *resultValues )[i] };
            readField( RigFemResultPosEnum::RIG_ELEMENT, internalKey, partIndex, stepIndex, &singleComp );
        }
        return;
    }

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
void RifVtkReader::readScalarData( RigFemPartCollection*                                                        femParts,
                                   std::map<int, std::string>&                                                  parts,
                                   const std::map<int, std::vector<std::map<std::string, std::vector<float>>>>& properties,
                                   size_t                                                                       numElements )
{
    for ( auto [partId, partName] : parts )
    {
        if ( auto partProperties = properties.find( partId ); partProperties != properties.end() )
        {
            int stepId = 0;

            for ( const auto& propertiesPerTimeStep : partProperties->second )
            {
                for ( const auto& [propertyName, values] : propertiesPerTimeStep )
                {
                    RigFemResultPosEnum resultType = values.size() == numElements ? RigFemResultPosEnum::RIG_ELEMENT
                                                                                  : RigFemResultPosEnum::RIG_NODAL;

                    // TODO: handle other result types...
                    if ( resultType == RigFemResultPosEnum::RIG_ELEMENT )
                    {
                        auto map = propertyDataMap( resultType );
                        if ( map == nullptr ) continue;
                        if ( map->count( propertyName ) == 0 )
                        {
                            ( *map )[propertyName] = {};
                        }

                        if ( ( *map )[propertyName].count( stepId ) == 0 )
                        {
                            ( *map )[propertyName][stepId] = {};
                        }

                        for ( int partId = 0; partId < femParts->partCount(); partId++ )
                        {
                            ( *map )[propertyName][stepId][partId] = {};
                            size_t dataSize                        = 0;
                            if ( resultType == RigFemResultPosEnum::RIG_NODAL )
                            {
                                dataSize = femParts->part( partId )->nodeCount();
                            }
                            if ( resultType == RigFemResultPosEnum::RIG_ELEMENT )
                            {
                                dataSize = femParts->part( partId )->elementCount();
                            }

                            std::vector<double> valuesAsDouble( dataSize, 0.0 );
                            // TODO: fix annoying conversion from float to double
                            for ( size_t i = 0; i < dataSize; i++ )
                            {
                                valuesAsDouble[i] = values[i];
                            }

                            ( *map )[propertyName][stepId][partId] = valuesAsDouble;
                        }
                    }
                    else if ( values.size() == 3 * numElements )
                    {
                        auto map = propertyDataMap( RigFemResultPosEnum::RIG_ELEMENT );
                        if ( map == nullptr ) continue;

                        // Multi-component element results: stress[0/1] -> S_VTK, strain[0/1] -> E_VTK
                        // Each VTK array contains 3 components per element.
                        // Mapping:
                        //   stress[0]: S_VTK.S11 (index 0), S_VTK.S22 (index 1), S_VTK.S33 (index 2)
                        //   stress[1]: S_VTK.S12 (index 0), S_VTK.S13 (index 1), S_VTK.S23 (index 2)
                        //   strain[0]: E_VTK.E11 (index 0), E_VTK.E22 (index 1), E_VTK.E33 (index 2)
                        //   strain[1]: E_VTK.E12 (index 0), E_VTK.E13 (index 1), E_VTK.E23 (index 2)

                        struct StressStrainMapping
                        {
                            std::string                vtkName;
                            std::string                fieldName;
                            std::array<std::string, 3> componentNames;
                        };

                        using namespace RifVtkFieldNames;
                        static const std::vector<StressStrainMapping> mappings = {
                            { "stress[0]", S, { S11, S22, S33 } },
                            { "stress[1]", S, { S12, S13, S23 } },
                            { "strain[0]", E, { E11, E22, E33 } },
                            { "strain[1]", E, { E12, E13, E23 } },
                        };

                        for ( const auto& mapping : mappings )
                        {
                            if ( propertyName != mapping.vtkName ) continue;

                            for ( int compIdx = 0; compIdx < 3; compIdx++ )
                            {
                                std::string internalKey = mapping.fieldName + separator + mapping.componentNames[compIdx];

                                if ( map->count( internalKey ) == 0 ) ( *map )[internalKey] = {};
                                if ( ( *map )[internalKey].count( stepId ) == 0 ) ( *map )[internalKey][stepId] = {};

                                for ( int partIdx = 0; partIdx < femParts->partCount(); partIdx++ )
                                {
                                    size_t              dataSize = femParts->part( partIdx )->elementCount();
                                    std::vector<double> compValues( dataSize, 0.0 );
                                    for ( size_t i = 0; i < dataSize; i++ )
                                    {
                                        compValues[i] = values[i * 3 + compIdx];
                                    }
                                    ( *map )[internalKey][stepId][partIdx] = compValues;
                                }
                            }
                        }
                    }
                }
                stepId++;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///  Map keys: result name / time step / part id
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::map<int, std::map<int, std::vector<double>>>>* RifVtkReader::propertyDataMap( RigFemResultPosEnum resultType )
{
    if ( resultType == RigFemResultPosEnum::RIG_ELEMENT ) return &m_propertyPartDataElements;
    if ( resultType == RigFemResultPosEnum::RIG_NODAL ) return &m_propertyPartDataNodes;

    return nullptr;
}
