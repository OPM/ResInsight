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
#include "RiaQStringFormatter.h"

#include "cafAssert.h"
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

// Fields in the convention of the derived results
constexpr const char* derivedS = "S";
constexpr const char* derivedE = "E";
constexpr const char* por      = "POR";
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
    using namespace RifVtkFieldNames;
    return m_propertyPartDataElements.contains( std::string( S ) + separator + S11 ) && m_propertyPartDataNodes.contains( por );
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
        RiaLogging::info( std::format( "Found {} timesteps", dataset.size() ) );
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
    CAF_ASSERT( femParts );

    // The key in the maps is the part ID
    std::map<int, std::string>                                              parts;
    std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>                  nodes;
    std::map<int, std::vector<std::pair<int, std::vector<int>>>>            elements;
    std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>> elementSets;
    std::map<int, std::vector<std::map<std::string, std::vector<float>>>>   properties;
    std::map<int, std::vector<std::map<std::string, std::vector<float>>>>   pointProperties;

    auto result = read( m_inputPath, parts, nodes, elements, elementSets, properties, pointProperties, m_displacements, m_stepNames );
    if ( !result )
    {
        RiaLogging::error( result.error() );
        return false;
    }

    auto elementType = result.value();

    if ( m_stepNames.empty() ) m_stepNames.push_back( "Geostatic" );

    RiaLogging::debug( QString( "Read FEM parts: %1, steps: %2, element type: %3" )
                           .arg( parts.size() )
                           .arg( m_stepNames.size() )
                           .arg( QString::fromStdString( RigFemTypes::elementTypeText( elementType ) ) )
                           .toStdString() );

    if ( !RigFemTypes::is8NodeElement( elementType ) )
    {
        RiaLogging::error( "Unsupported element type." );
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

        std::vector<int>& elementNodes = m_elementNodes[femPart->elementPartId()];
        for ( int elmIdx = 0; elmIdx < elmCount; ++elmIdx )
        {
            const int* conn = femPart->connectivities( elmIdx );
            elementNodes.insert( elementNodes.end(), conn, conn + RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) ) );
        }

        modelProgress.incrementProgress();
    }

    size_t elementCount = femParts->part( 0 )->elementCount();

    readScalarData( femParts, parts, properties, elementCount );
    readNodalData( femParts, pointProperties );

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
                        std::map<int, std::vector<std::map<std::string, std::vector<float>>>>&   pointProperties,
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
                              .arg( d.timestep )
                              .toStdString() );

        properties[partId].push_back( partProperties );
        pointProperties[partId].push_back( RifVtkImportUtil::readPointProperties( piece ) );
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
    using namespace RifVtkFieldNames;

    std::map<std::string, std::vector<std::string>> retVal;
    if ( m_propertyPartDataElements.contains( std::string( S ) + separator + S11 ) && m_propertyPartDataNodes.contains( por ) )
    {
        retVal[derivedS] = { S11, S22, S33, S12, S13, S23 };
    }
    if ( m_propertyPartDataElements.contains( std::string( E ) + separator + E11 ) )
    {
        retVal[derivedE] = { E11, E22, E33, E12, E13, E23 };
    }
    return retVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifVtkReader::scalarIntegrationPointFieldAndComponentNames()
{
    return scalarElementNodeFieldAndComponentNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readDisplacements( int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements )
{
    CAF_ASSERT( displacements );
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
    CAF_ASSERT( resultValues );

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
    CAF_ASSERT( resultValues );

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
    CAF_ASSERT( resultValues );

    // Stress and strain are cell values; every corner of an element gets its cell's value. S is effective
    // stress, tension positive, from the simulator's total stress (compression positive) and the node pore
    // pressure: S = POR * I - S_VTK. Then ST = -S + POR gives the simulator's stress back.
    using namespace RifVtkFieldNames;

    const auto components = scalarElementNodeFieldAndComponentNames()[fieldName];
    const auto nodesIt    = m_elementNodes.find( partIndex );
    if ( components.empty() || nodesIt == m_elementNodes.end() ) return;
    const std::vector<int>& elementNodes = nodesIt->second;

    const bool                 isStress  = fieldName == derivedS;
    const std::string          cellField = isStress ? S : E;
    const std::vector<double>& porValues = m_propertyPartDataNodes[por][stepIndex][partIndex];
    if ( isStress && porValues.empty() ) return;

    constexpr int nodesPerElement = 8;
    for ( size_t c = 0; c < components.size() && c < resultValues->size(); c++ )
    {
        std::vector<float>* values = ( *resultValues )[c];
        if ( values == nullptr ) continue;

        const std::vector<double>& cellValues = m_propertyPartDataElements[cellField + separator + components[c]][stepIndex][partIndex];
        if ( cellValues.empty() ) continue;

        const bool isNormal = c < 3;
        values->resize( elementNodes.size() );
        for ( size_t i = 0; i < elementNodes.size(); i++ )
        {
            const double cellValue = cellValues[i / nodesPerElement];
            if ( isStress )
                ( *values )[i] = static_cast<float>( ( isNormal ? porValues[elementNodes[i]] : 0.0 ) - cellValue );
            else
                ( *values )[i] = static_cast<float>( cellValue );
        }
    }
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
    readElementNodeField( fieldName, partIndex, stepIndex, frameIndex, resultValues );
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

                        // OPM Flow writes a symmetric 3x3 tensor as three rows "stress[0..2]" of 3 values each.
                        // The upper triangle is read. z is negated on import, so the xz and yz components change sign.
                        struct StressStrainMapping
                        {
                            std::string vtkName;
                            int         column;
                            std::string fieldName;
                            std::string componentName;
                            double      sign;
                        };

                        using namespace RifVtkFieldNames;
                        static const std::vector<StressStrainMapping> mappings = {
                            { "stress[0]", 0, S, S11, 1.0 },
                            { "stress[0]", 1, S, S12, 1.0 },
                            { "stress[0]", 2, S, S13, -1.0 },
                            { "stress[1]", 1, S, S22, 1.0 },
                            { "stress[1]", 2, S, S23, -1.0 },
                            { "stress[2]", 2, S, S33, 1.0 },
                            { "strain[0]", 0, E, E11, 1.0 },
                            { "strain[0]", 1, E, E12, 1.0 },
                            { "strain[0]", 2, E, E13, -1.0 },
                            { "strain[1]", 1, E, E22, 1.0 },
                            { "strain[1]", 2, E, E23, -1.0 },
                            { "strain[2]", 2, E, E33, 1.0 },
                        };

                        for ( const auto& mapping : mappings )
                        {
                            if ( propertyName != mapping.vtkName ) continue;

                            std::string internalKey = mapping.fieldName + separator + mapping.componentName;

                            for ( int partIdx = 0; partIdx < femParts->partCount(); partIdx++ )
                            {
                                size_t              dataSize = femParts->part( partIdx )->elementCount();
                                std::vector<double> compValues( dataSize, 0.0 );
                                for ( size_t i = 0; i < dataSize; i++ )
                                {
                                    compValues[i] = mapping.sign * values[i * 3 + mapping.column];
                                }
                                ( *map )[internalKey][stepId][partIdx] = compValues;
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
/// Scalar node data, e.g. from OPM Flow with --vtk-write-geo-mech-nodal. "pressure" becomes the pore pressure POR.
//--------------------------------------------------------------------------------------------------
void RifVtkReader::readNodalData( RigFemPartCollection*                                                        femParts,
                                  const std::map<int, std::vector<std::map<std::string, std::vector<float>>>>& pointProperties )
{
    for ( const auto& [partId, propertiesPerTimeStep] : pointProperties )
    {
        if ( partId >= femParts->partCount() ) continue;
        const size_t nodeCount = femParts->part( partId )->nodeCount();

        for ( int stepId = 0; stepId < static_cast<int>( propertiesPerTimeStep.size() ); stepId++ )
        {
            for ( const auto& [propertyName, values] : propertiesPerTimeStep[stepId] )
            {
                if ( values.size() != nodeCount ) continue;

                const std::string fieldName = ( propertyName == "pressure" ) ? RifVtkFieldNames::por : propertyName;
                m_propertyPartDataNodes[fieldName][stepId][partId].assign( values.begin(), values.end() );
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
