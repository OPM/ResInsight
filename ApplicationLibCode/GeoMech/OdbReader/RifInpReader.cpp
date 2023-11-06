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
void RifInpReader::close()
{
    m_stream.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpReader::openFile( const std::string& fileName, std::string* errorMessage )
{
    m_stream.open( fileName );
    return m_stream.good();
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

    read( m_stream, parts, nodes, elements, elementSets );

    RiaLogging::debug( QString( "Read FEM parts: %1 nodes: %2 elements: %3" ).arg( parts.size() ).arg( nodes.size() ).arg( elements.size() ) );

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

            // TODO: handle more types?
            RigElementType elmType   = RigElementType::HEX8;
            int            nodeCount = RigFemTypes::elementNodeCount( elmType );

            indexBasedConnectivities.resize( nodeCount );
            for ( int lnIdx = 0; lnIdx < nodeCount; ++lnIdx )
            {
                indexBasedConnectivities[lnIdx] = nodeIdToIdxMap[nodesInElement[lnIdx]];
            }

            femPart->appendElement( elmType, elmId, indexBasedConnectivities.data() );
        }

        // read element sets
        auto elementSetsForPart = elementSets[partId];
        for ( auto [setName, elementSet] : elementSetsForPart )
        {
            femPart->addElementSet( setName, elementSet );
        }

        femPart->setElementPartId( femParts->partCount() );
        femParts->addFemPart( femPart );

        modelProgress.incrementProgress();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpReader::read( std::istream&                                                            stream,
                         std::map<int, std::string>&                                              parts,
                         std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>&                  nodes,
                         std::map<int, std::vector<std::pair<int, std::vector<int>>>>&            elements,
                         std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>>& elementSets )
{
    std::string line;

    int         partId = 0;
    std::string partName;

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
                skipComments( stream );
                elements[partId] = readElements( stream );
            }
            else if ( uppercasedLine.starts_with( "*ELSET," ) )
            {
                skipComments( stream );
                std::string setName    = parseLabel( line, "elset" );
                auto        elementSet = readElementSet( stream );
                elementSets[partId].push_back( { setName, elementSet } );
            }

            continue;
        }

        if ( stream.eof() ) break;
    }
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

    // TODO: maybe support more element types
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
            int elementId = RiaStdStringTools::toInt( part ) - 1;
            elementSet.push_back( elementId );
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
    // TODO: read step names from file.
    std::vector<std::string> stepNames;
    stepNames.push_back( "step 1" );

    return stepNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifInpReader::filteredStepNames() const
{
    // TODO: add filtering.
    return RifInpReader::allStepNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifInpReader::frameTimes( int stepIndex ) const
{
    // TODO: get from file?
    std::vector<double> frameValues;
    frameValues.push_back( 1.0 );
    return frameValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifInpReader::frameCount( int stepIndex ) const
{
    if ( shouldReadOnlyLastFrame() ) return 1;

    return frameTimes( stepIndex ).size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifInpReader::elementSetNames( int partIndex, std::string partName )
{
    // TODO: not implemented yet
    if ( partIndex >= m_partElementSetNames.size() ) return {};

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

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifInpReader::scalarNodeFieldAndComponentNames()
{
    return {};
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
void RifInpReader::readNodeField( const std::string&                fieldName,
                                  int                               partIndex,
                                  int                               stepIndex,
                                  int                               frameIndex,
                                  std::vector<std::vector<float>*>* resultValues )
{
    CVF_ASSERT( resultValues );
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
    CVF_ASSERT( resultValues );
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
    CVF_ASSERT( resultValues );
}
