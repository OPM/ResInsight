/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RifInpReader.h"

#include "RifInpIncludeReader.h"

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
RifInpReader::RifInpReader()
    : m_enableIncludes( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifInpReader::~RifInpReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpReader::enableIncludes( bool enable )
{
    m_enableIncludes = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpReader::close()
{
    m_stream.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpReader::populateDerivedResultNames() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpReader::openFile( const std::string& fileName, std::string* errorMessage )
{
    m_stream.open( fileName );
    bool bOK = m_stream.good();

    if ( bOK )
    {
        m_inputPath = std::filesystem::path( fileName ).parent_path();
    }

    return bOK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpReader::isOpen() const
{
    return m_stream.is_open();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpReader::readFemParts( RigFemPartCollection* femParts )
{
    CVF_ASSERT( femParts );

    // The key in the maps is the part ID
    std::map<int, std::string>                                              parts;
    std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>                  nodes;
    std::map<int, std::vector<std::pair<int, std::vector<int>>>>            elements;
    std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>> elementSets;

    auto elementType = read( m_stream, parts, nodes, elements, elementSets, m_stepNames, m_enableIncludes, m_includeEntries );

    for ( int i = 0; i < (int)m_includeEntries.size(); i++ )
    {
        m_includeEntries[i].fileName = ( m_inputPath / m_includeEntries[i].fileName ).string();
    }

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

        auto nodeCount = partNodes.size();
        femPart->nodes().nodeIds.resize( nodeCount );
        femPart->nodes().coordinates.resize( nodeCount );

        for ( int nIdx = 0; nIdx < static_cast<int>( nodeCount ); ++nIdx )
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

        auto elmCount = static_cast<int>( partElements.size() );
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

    readScalarData( femParts, parts, m_includeEntries );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigElementType RifInpReader::read( std::istream&                                                            stream,
                                   std::map<int, std::string>&                                              parts,
                                   std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>&                  nodes,
                                   std::map<int, std::vector<std::pair<int, std::vector<int>>>>&            elements,
                                   std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>>& elementSets,
                                   std::vector<std::string>&                                                stepNames,
                                   bool                                                                     enableIncludes,
                                   std::vector<RifInpIncludeEntry>&                                         includeEntries )
{
    std::string line;
    std::string prevline;

    RigElementType elementType = RigElementType::UNKNOWN_ELM_TYPE;

    int         partId = 0;
    std::string partName;
    int         stepId = -1;
    std::string stepName;
    int         timeSteps = 0;

    while ( true )
    {
        std::getline( stream, line );

        if ( stream )
        {
            std::string uppercasedLine = RiaStdStringTools::toUpper( line );

            // "Part" section.
            if ( uppercasedLine.starts_with( "*PART" ) )
            {
                partName = parseLabel( line, "name" );
            }
            // "End Part" section.
            else if ( uppercasedLine.starts_with( "*END PART" ) )
            {
                parts[partId] = partName;

                partName = "";
                partId++;
            }
            // "Node" section.
            else if ( uppercasedLine.starts_with( "*NODE" ) )
            {
                skipComments( stream );
                nodes[partId] = readNodes( stream );
            }
            // "Element" section.
            else if ( uppercasedLine.starts_with( "*ELEMENT," ) )
            {
                auto nodeType = parseLabel( line, "type" );
                elementType   = RigFemTypes::toRigElementType( nodeType );
                skipComments( stream );

                if ( !elements.contains( partId ) ) elements[partId] = {};

                auto newElements = readElements( stream );
                for ( const auto& e : newElements )
                {
                    elements[partId].push_back( e );
                }
            }
            else if ( uppercasedLine.starts_with( "*ELSET," ) )
            {
                bool isGenerateSet = uppercasedLine.find( "GENERATE" ) != std::string::npos;
                skipComments( stream );
                std::string setName    = parseLabel( line, "elset" );
                auto        elementSet = isGenerateSet ? readElementSetGenerate( stream ) : readElementSet( stream );
                elementSets[partId].push_back( { setName, elementSet } );
            }
            else if ( uppercasedLine.starts_with( "*STEP" ) )
            {
                stepName = parseLabel( line, "name" );
                stepNames.push_back( stepName );
                stepId = static_cast<int>( stepNames.size() - 1 );
            }
            else if ( uppercasedLine.starts_with( "*END STEP" ) )
            {
                stepId   = -1;
                stepName = "";
            }
            else if ( enableIncludes && uppercasedLine.starts_with( "*INCLUDE" ) )
            {
                auto                filename   = parseLabel( line, "input" );
                RigFemResultPosEnum resultType = RigFemResultPosEnum::RIG_ELEMENT;
                std::string         propertyName( "" );
                int                 columnIndex = 1;

                if ( prevline.starts_with( "*BOUNDARY" ) )
                {
                    propertyName = "POR";
                    resultType   = RigFemResultPosEnum::RIG_NODAL;
                    columnIndex  = 3;
                }
                else if ( prevline.starts_with( "*TEMPERATURE" ) )
                {
                    propertyName = "TEMP";
                    resultType   = RigFemResultPosEnum::RIG_NODAL;
                }
                else if ( prevline.starts_with( "*INITIAL" ) )
                {
                    auto label = parseLabel( prevline, "type" );
                    if ( label == "RATIO" )
                    {
                        propertyName = "VOIDR";
                        resultType   = RigFemResultPosEnum::RIG_NODAL;
                    }
                    else if ( label == "STRESS" )
                    {
                        includeEntries.push_back( RifInpIncludeEntry( "STRESS_TOP", RigFemResultPosEnum::RIG_ELEMENT, stepId, filename, 1 ) );
                        includeEntries.push_back( RifInpIncludeEntry( "DEPTH_TOP", RigFemResultPosEnum::RIG_ELEMENT, stepId, filename, 2 ) );
                        includeEntries.push_back( RifInpIncludeEntry( "STRESS_BOTTOM", RigFemResultPosEnum::RIG_ELEMENT, stepId, filename, 3 ) );
                        includeEntries.push_back( RifInpIncludeEntry( "DEPTH_BOTTOM", RigFemResultPosEnum::RIG_ELEMENT, stepId, filename, 4 ) );
                        includeEntries.push_back(
                            RifInpIncludeEntry( "LATERAL_STRESS_X", RigFemResultPosEnum::RIG_ELEMENT, stepId, filename, 5 ) );

                        propertyName = "LATERAL_STRESS_Y";
                        columnIndex  = 6;
                    }
                }
                if ( propertyName.empty() )
                {
                    std::string uppercasedFilename = RiaStdStringTools::toUpper( filename );

                    if ( uppercasedFilename.find( "DENSITY" ) != std::string::npos )
                    {
                        propertyName = "DENSITY";
                    }
                    else if ( uppercasedFilename.find( "ELASTICS" ) != std::string::npos )
                    {
                        includeEntries.push_back( RifInpIncludeEntry( "MODULUS", RigFemResultPosEnum::RIG_ELEMENT, stepId, filename, 1 ) );

                        propertyName = "RATIO";
                        columnIndex  = 2;
                    }
                }
                if ( !propertyName.empty() )
                {
                    includeEntries.push_back( RifInpIncludeEntry( propertyName, resultType, stepId, filename, columnIndex ) );
                }
            }
            prevline = uppercasedLine;
            continue;
        }

        if ( stream.eof() ) break;
    }

    return elementType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, cvf::Vec3d>> RifInpReader::readNodes( std::istream& stream )
{
    std::vector<std::pair<int, cvf::Vec3d>> nodes;
    while ( stream.peek() != '*' && stream.peek() != EOF )
    {
        std::string line;
        std::getline( stream, line );

        std::stringstream ss( RiaStdStringTools::removeWhitespace( line ) );

        int    nodeId = 0;
        double x      = 0.0;
        double y      = 0.0;
        double z      = 0.0;
        char   comma;

        ss >> nodeId >> comma >> x >> comma >> y >> comma >> z;

        nodes.push_back( { nodeId, cvf::Vec3d( x, y, z ) } );
    }

    return nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, std::vector<int>>> RifInpReader::readElements( std::istream& stream )
{
    std::vector<std::pair<int, std::vector<int>>> partElements;

    // only support C3D8* element type
    unsigned numNodesPerElement = 8;

    // Read until we find a new section (which should start with a '*').
    while ( stream.peek() != '*' && stream.peek() != EOF )
    {
        // First read the element id (and consume the comma)
        int  elementId = 0;
        char comma;
        stream >> elementId >> comma;

        unsigned         nodeCount = 0;
        std::vector<int> els;

        // Read line-by-line
        while ( nodeCount < numNodesPerElement )
        {
            // Read entire line of comma-separated values
            std::string line;
            std::getline( stream, line );

            // Process the comma-separated values
            auto parts = RiaStdStringTools::splitString( line, ',' );
            for ( auto part : parts )
            {
                int nodeId = RiaStdStringTools::toInt( part );
                if ( nodeId > 0 )
                {
                    els.push_back( nodeId );
                    nodeCount++;
                }
            }
        }

        partElements.push_back( { elementId, els } );
    }

    return partElements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifInpReader::readElementSet( std::istream& stream )
{
    std::vector<size_t> elementSet;

    // Read until we find a new section (which should start with a '*').
    while ( stream.peek() != '*' && stream.peek() != EOF )
    {
        // Read entire line of comma-separated values
        std::string line;
        std::getline( stream, line );

        // Process the comma-separated values
        auto parts = RiaStdStringTools::splitString( line, ',' );
        for ( auto part : parts )
        {
            auto trimmedPart = RiaStdStringTools::trimString( part );

            if ( !trimmedPart.empty() )
            {
                int elementId = RiaStdStringTools::toInt( trimmedPart ) - 1;
                elementSet.push_back( elementId );
            }
        }
    }

    return elementSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifInpReader::readElementSetGenerate( std::istream& stream )
{
    std::vector<size_t> elementSet;

    // Read until we find a new section (which should start with a '*').
    while ( stream.peek() != '*' && stream.peek() != EOF )
    {
        // Read entire line of comma-separated values
        std::string line;
        std::getline( stream, line );

        // Process the comma-separated values
        auto parts = RiaStdStringTools::splitString( line, ',' );
        if ( parts.size() >= 3 )
        {
            int firstElement = RiaStdStringTools::toInt( parts[0] );
            int lastElement  = RiaStdStringTools::toInt( parts[1] );
            int increment    = RiaStdStringTools::toInt( parts[2] );
            if ( lastElement < firstElement || increment <= 0 )
            {
                RiaLogging::error( "Encountered illegal set definition (using GENERATE keyword)." );
                return elementSet;
            }

            for ( int i = firstElement; i < lastElement; i += increment )
            {
                int elementId = i - 1;
                elementSet.push_back( elementId );
            }
        }
    }

    return elementSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifInpReader::parseLabel( const std::string& line, const std::string& labelName )
{
    std::string cleaned        = RiaStdStringTools::removeWhitespace( line );
    std::string upperLine      = RiaStdStringTools::toUpper( cleaned );
    std::string upperLabelName = RiaStdStringTools::toUpper( labelName );

    // Get index of start of "label="
    size_t labelIndex = upperLine.find( upperLabelName + "=" );
    if ( labelIndex != std::string::npos )
    {
        // Location of the first comma following "label="
        size_t commaIndex = upperLine.find( ",", labelIndex );

        // Extract the label substring
        size_t subStringStart = labelName.size() + 1 + labelIndex;
        size_t subStringEnd   = ( commaIndex == std::string::npos ) ? cleaned.size() : commaIndex;
        return cleaned.substr( subStringStart, subStringEnd - subStringStart );
    }

    return std::string();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpReader::skipComments( std::istream& stream )
{
    // Comments start with two stars.
    while ( true )
    {
        if ( stream.peek() != '*' )
        {
            // First character is not a star: line cannot be a comment
            return;
        }

        // Consume the first star.
        stream.get();

        if ( stream.peek() != '*' )
        {
            // The second char is not a star: put the first star back
            stream.unget();
            return;
        }

        // Found second star: this a comment.
        // Skip rest of the line
        std::string dummy;
        std::getline( stream, dummy );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifInpReader::allStepNames() const
{
    return m_stepNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifInpReader::filteredStepNames() const
{
    // no filter supported
    return RifInpReader::allStepNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifInpReader::frameTimes( int stepIndex ) const
{
    // only one frame from INP file
    std::vector<double> frameValues( { 1.0 } );
    return frameValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifInpReader::frameCount( int stepIndex ) const
{
    return static_cast<int>( frameTimes( stepIndex ).size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifInpReader::elementSetNames( int partIndex, std::string partName )
{
    if ( partIndex >= (int)m_partElementSetNames.size() ) return {};
    return m_partElementSetNames.at( partIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifInpReader::elementSet( int partIndex, std::string partName, int setIndex )
{
    // TODO: not implemented yet
    std::vector<size_t> elementIndexes;
    return elementIndexes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifInpReader::scalarNodeFieldAndComponentNames()
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
std::map<std::string, std::vector<std::string>> RifInpReader::scalarElementFieldAndComponentNames()
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
std::map<std::string, std::vector<std::string>> RifInpReader::scalarElementNodeFieldAndComponentNames()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifInpReader::scalarIntegrationPointFieldAndComponentNames()
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpReader::readDisplacements( int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements )
{
    CVF_ASSERT( displacements );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpReader::readField( RigFemResultPosEnum               resultType,
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
void RifInpReader::readNodeField( const std::string&                fieldName,
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
void RifInpReader::readElementField( const std::string&                fieldName,
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
void RifInpReader::readElementNodeField( const std::string&                fieldName,
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
void RifInpReader::readIntegrationPointField( const std::string&                fieldName,
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
void RifInpReader::readScalarData( RigFemPartCollection*            femParts,
                                   std::map<int, std::string>&      parts,
                                   std::vector<RifInpIncludeEntry>& includeEntries )
{
    for ( auto& entry : includeEntries )
    {
        auto map = propertyDataMap( entry.resultType );
        if ( map == nullptr ) continue;
        if ( map->count( entry.propertyName ) == 0 )
        {
            ( *map )[entry.propertyName] = {};
        }

        int stepId = entry.stepId;

        if ( ( *map )[entry.propertyName].count( stepId ) == 0 )
        {
            ( *map )[entry.propertyName][stepId] = {};
        }

        for ( int partId = 0; partId < femParts->partCount(); partId++ )
        {
            ( *map )[entry.propertyName][stepId][partId] = {};
            size_t dataSize                              = 0;
            if ( entry.resultType == RigFemResultPosEnum::RIG_NODAL )
            {
                dataSize = femParts->part( partId )->nodeCount();
            }
            if ( entry.resultType == RigFemResultPosEnum::RIG_ELEMENT )
            {
                dataSize = femParts->part( partId )->elementCount();
            }
            ( *map )[entry.propertyName][stepId][partId].resize( dataSize, 0.0 );
        }
        RifInpIncludeReader reader;
        if ( !reader.openFile( entry.fileName ) ) continue;
        reader.readData( entry.columnIndex, parts, ( *map )[entry.propertyName][stepId] );
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///  Map keys: result name / time step / part id
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::map<int, std::map<int, std::vector<double>>>>* RifInpReader::propertyDataMap( RigFemResultPosEnum resultType )
{
    if ( resultType == RigFemResultPosEnum::RIG_ELEMENT ) return &m_propertyPartDataElements;
    if ( resultType == RigFemResultPosEnum::RIG_NODAL ) return &m_propertyPartDataNodes;

    return nullptr;
}
