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

#ifdef _MSC_VER
// Get rid of warnings from compilation of ODB API
#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4482 )
#pragma warning( disable : 4584 )
#pragma warning( disable : 4800 )
#endif
#include <odb_API.h>
#include <odb_Enum.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "RifOdbReader.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"

#include "cafProgressInfo.h"
#include <QString>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>

//==================================================================================================
//
// Helper class to ensure that ODB bulk data are returned as float. Converting if necessary.
//
//==================================================================================================
class RifOdbBulkDataGetter
{
public:
    explicit RifOdbBulkDataGetter( const odb_FieldBulkData& bulkData )
        : m_bulkData( bulkData ){};
    virtual ~RifOdbBulkDataGetter(){};

    float* data()
    {
        odb_Enum::odb_PrecisionEnum precision = m_bulkData.precision();
        if ( precision == odb_Enum::SINGLE_PRECISION )
        {
            return m_bulkData.data();
        }
        else if ( precision == odb_Enum::DOUBLE_PRECISION )
        {
            if ( m_data.size() < 1 )
            {
                int dataSize = m_bulkData.length() * m_bulkData.width();
                m_data.resize( dataSize );

                double* doublePtr = m_bulkData.dataDouble();
                CVF_ASSERT( doublePtr );

                float* dataPtr = m_data.data();
                for ( int i = 0; i < dataSize; i++ )
                {
                    dataPtr[i] = (float)doublePtr[i];
                }
            }

            return m_data.data();
        }

        // Should never end up here
        CVF_ASSERT( 0 );
        return NULL;
    }

private:
    const odb_FieldBulkData& m_bulkData;
    std::vector<float>       m_data;
};

size_t RifOdbReader::sm_instanceCount = 0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

std::map<std::string, RigElementType> initFemTypeMap()
{
    std::map<std::string, RigElementType> typeMap;
    typeMap["C3D8R"] = HEX8;
    typeMap["C3D8"]  = HEX8;
    typeMap["C3D8P"] = HEX8P;
    typeMap["CAX4"]  = CAX4;

    return typeMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigElementType toRigElementType( const odb_String& odbTypeName )
{
    static std::map<std::string, RigElementType> odbElmTypeToRigElmTypeMap = initFemTypeMap();

    std::map<std::string, RigElementType>::iterator it = odbElmTypeToRigElmTypeMap.find( odbTypeName.cStr() );

    if ( it == odbElmTypeToRigElmTypeMap.end() )
    {
#if 0
        std::cout << "Unsupported element type :" << odbElm.type().cStr() << std::endl;
#endif
        return UNKNOWN_ELM_TYPE;
    }

    return it->second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const int* localElmNodeToIntegrationPointMapping( RigElementType elmType )
{
    static const int HEX8_Mapping[8] = {0, 1, 3, 2, 4, 5, 7, 6};

    switch ( elmType )
    {
        case HEX8:
        case HEX8P:
            return HEX8_Mapping;
            break;
        case CAX4:
            return HEX8_Mapping; // First four is identical to HEX8
            break;
        default:
            // assert(false); // Element type not supported
            break;
    }
    return NULL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOdbReader::RifOdbReader()
{
    if ( ++sm_instanceCount == 1 )
    {
        odb_initializeAPI();
    }

    m_odb = NULL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOdbReader::~RifOdbReader()
{
    close();

    if ( --sm_instanceCount == 0 )
    {
        odb_finalizeAPI();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOdbReader::close()
{
    if ( m_odb )
    {
        m_odb->close();
        m_odb = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::openFile( const std::string& fileName, std::string* errorMessage )
{
    close();
    CVF_ASSERT( m_odb == NULL );

    odb_String path = fileName.c_str();

    try
    {
        m_odb = &openOdb( path, true );
    }

    catch ( const nex_Exception& nex )
    {
        if ( errorMessage )
        {
            *errorMessage = nex.UserReport().CStr();
        }

        return false;
    }

    catch ( ... )
    {
        if ( errorMessage )
        {
            std::stringstream errStr;
            errStr << "Unable to open file '" << fileName << "'.";

            *errorMessage = errStr.str();
        }

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::isOpen() const
{
    return m_odb != NULL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOdbReader::assertMetaDataLoaded()
{
    CVF_ASSERT( m_odb != NULL );

    if ( m_resultsMetaData.empty() )
    {
        m_resultsMetaData = readResultsMetaData( m_odb );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RifOdbReader::RifOdbResultKey, std::vector<std::string>> RifOdbReader::readResultsMetaData( odb_Odb* odb )
{
    CVF_ASSERT( odb != NULL );

    std::map<RifOdbResultKey, std::vector<std::string>> resultsMap;

    const odb_StepRepository& stepRepository = odb->steps();
    odb_StepRepositoryIT      stepIt( stepRepository );
    stepIt.first();

    const odb_Step&          step       = stepRepository.constGet( stepIt.currentKey() );
    const odb_SequenceFrame& stepFrames = step.frames();

    if ( stepFrames.size() > 1 )
    {
        // Optimization: Get results metadata for the second frame of the first step only
        const odb_Frame& frame = stepFrames.constGet( 1 );

        const odb_FieldOutputRepository& fieldCon = frame.fieldOutputs();
        odb_FieldOutputRepositoryIT      fieldConIT( fieldCon );

        for ( fieldConIT.first(); !fieldConIT.isDone(); fieldConIT.next() )
        {
            const odb_FieldOutput& field = fieldCon[fieldConIT.currentKey()];

            const odb_SequenceFieldLocation& fieldLocations = field.locations();
            for ( int loc = 0; loc < fieldLocations.size(); loc++ )
            {
                const odb_FieldLocation& fieldLocation = fieldLocations.constGet( loc );

                std::string        fieldName  = field.name().CStr();
                odb_SequenceString components = field.componentLabels();

                std::vector<std::string> compVec;

                int numComp = components.size();
                for ( int comp = 0; comp < numComp; comp++ )
                {
                    compVec.push_back( components[comp].CStr() );
                }

                switch ( fieldLocation.position() )
                {
                    case odb_Enum::NODAL:
                        resultsMap[RifOdbResultKey( NODAL, fieldName )] = compVec;
                        break;

                    case odb_Enum::ELEMENT_NODAL:
                        resultsMap[RifOdbResultKey( ELEMENT_NODAL, fieldName )] = compVec;
                        break;

                    case odb_Enum::INTEGRATION_POINT:
                        resultsMap[RifOdbResultKey( INTEGRATION_POINT, fieldName )] = compVec;
                        resultsMap[RifOdbResultKey( ELEMENT_NODAL, fieldName )]     = compVec;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    stepFrames.release();

    return resultsMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::readFemParts( RigFemPartCollection* femParts )
{
    CVF_ASSERT( femParts );
    CVF_ASSERT( m_odb != NULL );

    odb_InstanceRepository   instanceRepository = m_odb->rootAssembly().instances();
    odb_InstanceRepositoryIT iter( instanceRepository );

    caf::ProgressInfo modelProgress( instanceRepository.size() * ( 2 + 4 ), "Reading Odb Parts" );

    int instanceCount = 0;
    for ( iter.first(); !iter.isDone(); iter.next(), instanceCount++ )
    {
        modelProgress.setProgressDescription( QString( iter.currentKey().cStr() ) + ": Reading Nodes" );
        m_nodeIdToIdxMaps.push_back( std::map<int, int>() );

        odb_Instance& inst = instanceRepository[iter.currentKey()];

        RigFemPart* femPart = new RigFemPart;

        // Extract nodes
        const odb_SequenceNode& odbNodes = inst.nodes();

        std::map<int, int>& nodeIdToIdxMap = m_nodeIdToIdxMaps.back();

        int nodeCount = odbNodes.size();
        femPart->nodes().nodeIds.resize( nodeCount );
        femPart->nodes().coordinates.resize( nodeCount );

        for ( int nIdx = 0; nIdx < nodeCount; ++nIdx )
        {
            const odb_Node odbNode         = odbNodes.node( nIdx );
            femPart->nodes().nodeIds[nIdx] = odbNode.label();
            const float* pos               = odbNode.coordinates();
            femPart->nodes().coordinates[nIdx].set( pos[0], pos[1], pos[2] );
            nodeIdToIdxMap[odbNode.label()] = nIdx;

            // Progress reporting
            if ( nIdx == nodeCount / 2 )
            {
                modelProgress.incrementProgress();
            }
        }

        modelProgress.incrementProgress();
        modelProgress.setProgressDescription( QString( iter.currentKey().cStr() ) + ": Reading Elements" );

        // Extract elements
        const odb_SequenceElement& elements = inst.elements();

        int elmCount = elements.size();
        femPart->preAllocateElementStorage( elmCount );

        std::vector<int> indexBasedConnectivities;

        m_elementIdToIdxMaps.push_back( std::map<int, int>() );
        std::map<int, int>& elementIdToIdxMap = m_elementIdToIdxMaps.back();

        for ( int elmIdx = 0; elmIdx < elmCount; ++elmIdx )
        {
            const odb_Element odbElm = elements.element( elmIdx );

            elementIdToIdxMap[odbElm.label()] = elmIdx;

            RigElementType elmType = toRigElementType( odbElm.type() );
            if ( elmType == UNKNOWN_ELM_TYPE ) continue;

            int        nodeCount             = 0;
            const int* idBasedConnectivities = odbElm.connectivity( nodeCount );
            CVF_TIGHT_ASSERT( nodeCount == RigFemTypes::elementNodeCount( elmType ) );

            indexBasedConnectivities.resize( nodeCount );
            for ( int lnIdx = 0; lnIdx < nodeCount; ++lnIdx )
            {
                indexBasedConnectivities[lnIdx] = nodeIdToIdxMap[idBasedConnectivities[lnIdx]];
            }

            femPart->appendElement( elmType, odbElm.label(), indexBasedConnectivities.data() );

            // Progress reporting
            if ( elmIdx == elmCount / 4 || elmIdx == elmCount / 2 || elmIdx == 3 * elmCount / 4 )
            {
                modelProgress.incrementProgress();
            }
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
std::vector<std::string> RifOdbReader::allStepNames() const
{
    CVF_ASSERT( m_odb != NULL );

    std::vector<std::string> stepNames;

    odb_StepRepository   stepRepository = m_odb->steps();
    odb_StepRepositoryIT sIter( stepRepository );
    for ( sIter.first(); !sIter.isDone(); sIter.next() )
    {
        std::string stepName( sIter.currentValue().name().CStr() );
        stepNames.push_back( stepName );
    }

    return stepNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOdbReader::filteredStepNames() const
{
    CVF_ASSERT( m_odb != NULL );

    std::vector<std::string> stepNames;

    odb_StepRepository   stepRepository = m_odb->steps();
    odb_StepRepositoryIT sIter( stepRepository );
    int                  stepIndex = 0;
    for ( sIter.first(); !sIter.isDone(); sIter.next() )
    {
        std::string stepName( sIter.currentValue().name().CStr() );
        if ( this->isTimeStepIncludedByFilter( stepIndex++ ) )
        {
            stepNames.push_back( stepName );
        }
    }

    return stepNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifOdbReader::frameTimes( int stepIndex ) const
{
    CVF_ASSERT( m_odb != NULL );

    odb_StepRepository& stepRepository = m_odb->steps();

    odb_StepList stepList = stepRepository.stepList();

    int stepFileIndex = this->timeStepIndexOnFile( stepIndex );

    odb_Step& step = stepList.Get( stepFileIndex );

    odb_SequenceFrame& stepFrames = step.frames();

    std::vector<double> frameValues;

    int numFrames = stepFrames.size();
    for ( int f = 0; f < numFrames; f++ )
    {
        odb_Frame frame = stepFrames.constGet( f );
        frameValues.push_back( frame.frameValue() );
    }

    return frameValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOdbReader::elementSetNames( int partIndex )
{
    CVF_ASSERT( m_odb != NULL );

    std::map<int, std::vector<std::string>>::const_iterator mapIt = m_partElementSetNames.find( partIndex );
    if ( mapIt == m_partElementSetNames.end() )
    {
        std::vector<std::string> setNames;

        const odb_Assembly&           rootAssembly = m_odb->constRootAssembly();
        const odb_InstanceRepository& instances    = rootAssembly.instances();

        int                      currentInstance = 0;
        odb_InstanceRepositoryIT instIt( instances );
        for ( instIt.first(); !instIt.isDone(); instIt.next(), currentInstance++ )
        {
            const odb_Instance& instance = instIt.currentValue();

            if ( currentInstance == partIndex )
            {
                const odb_SetRepository& sets = rootAssembly.elementSets();

                odb_SetRepositoryIT setIt( sets );
                for ( setIt.first(); !setIt.isDone(); setIt.next() )
                {
                    const odb_Set& set = setIt.currentValue();
                    setNames.push_back( set.name().CStr() );
                }

                break;
            }
        }

        m_partElementSetNames[partIndex] = setNames;
    }

    return m_partElementSetNames.at( partIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifOdbReader::elementSet( int partIndex, int setIndex )
{
    CVF_ASSERT( m_odb != NULL );

    std::vector<std::string> setNames     = elementSetNames( partIndex );
    const odb_Assembly&      rootAssembly = m_odb->constRootAssembly();

    const odb_Set&     set           = rootAssembly.elementSets()[odb_String( setNames[setIndex].c_str() )];
    odb_SequenceString instanceNames = set.instanceNames();

    const odb_SequenceElement& setElements  = set.elements( instanceNames[partIndex] );
    int                        elementCount = setElements.size();

    std::vector<size_t> elementIndexes;
    elementIndexes.resize( elementCount );

    for ( int i = 0; i < elementCount; i++ )
    {
        elementIndexes[i] = setElements.element( i ).index();
    }

    return elementIndexes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifOdbReader::scalarNodeFieldAndComponentNames()
{
    return fieldAndComponentNames( NODAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifOdbReader::scalarElementNodeFieldAndComponentNames()
{
    return fieldAndComponentNames( ELEMENT_NODAL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifOdbReader::scalarIntegrationPointFieldAndComponentNames()
{
    return fieldAndComponentNames( INTEGRATION_POINT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const odb_Frame& RifOdbReader::stepFrame( int stepIndex, int frameIndex ) const
{
    CVF_ASSERT( m_odb );

    const odb_StepRepository& stepRepository = m_odb->steps();
    const odb_StepList&       stepList       = stepRepository.stepList();

    int stepFileIndex = this->timeStepIndexOnFile( stepIndex );

    const odb_Step&          step       = stepList.ConstGet( stepFileIndex );
    const odb_SequenceFrame& stepFrames = step.frames();

    return stepFrames.constGet( frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
odb_Instance* RifOdbReader::instance( int instanceIndex )
{
    CVF_ASSERT( m_odb != NULL );

    odb_InstanceRepository&  instanceRepository = m_odb->rootAssembly().instances();
    odb_InstanceRepositoryIT iter( instanceRepository );

    int instanceCount = 0;
    for ( iter.first(); !iter.isDone(); iter.next(), instanceCount++ )
    {
        odb_Instance& inst = instanceRepository[iter.currentKey()];

        if ( instanceCount == instanceIndex )
        {
            return &inst;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// Get the number of result items (== #nodes or #elements)
//--------------------------------------------------------------------------------------------------
size_t RifOdbReader::resultItemCount( const std::string& fieldName, int partIndex, int stepIndex, int frameIndex )
{
    odb_Instance* partInstance = instance( partIndex );
    CVF_ASSERT( partInstance != NULL );

    const odb_Frame&       frame               = stepFrame( stepIndex, frameIndex );
    const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset( *partInstance );
    const odb_SequenceFieldBulkData& seqFieldBulkData = instanceFieldOutput.bulkDataBlocks();

    size_t resultItemCount = 0;
    int    numBlocks       = seqFieldBulkData.size();

    for ( int block = 0; block < numBlocks; block++ )
    {
        const odb_FieldBulkData& bulkData = seqFieldBulkData[block];
        resultItemCount += bulkData.length();
    }

    return resultItemCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifOdbReader::componentsCount( const std::string& fieldName, ResultPosition position )
{
    std::vector<std::string> compNames = componentNames( RifOdbResultKey( position, fieldName ) );
    return compNames.size() > 0 ? compNames.size() : 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOdbReader::componentNames( const RifOdbResultKey& result )
{
    assertMetaDataLoaded();

    std::map<RifOdbResultKey, std::vector<std::string>>::const_iterator resMapIt = m_resultsMetaData.find( result );
    if ( resMapIt != m_resultsMetaData.end() )
    {
        std::vector<std::string> compNames;
        compNames = resMapIt->second;
        return compNames;
    }

    CVF_ASSERT( false );
    return std::vector<std::string>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>> RifOdbReader::fieldAndComponentNames( ResultPosition position )
{
    assertMetaDataLoaded();

    std::map<std::string, std::vector<std::string>> fieldsAndComponents;

    std::map<RifOdbResultKey, std::vector<std::string>>::const_iterator resMapIt;
    for ( resMapIt = m_resultsMetaData.begin(); resMapIt != m_resultsMetaData.end(); ++resMapIt )
    {
        if ( resMapIt->first.resultPostion == position )
        {
            fieldsAndComponents[resMapIt->first.fieldName] = resMapIt->second;
        }
    }

    return fieldsAndComponents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readDisplacements( int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements )
{
    CVF_ASSERT( displacements );

    odb_Instance* partInstance = instance( partIndex );
    CVF_ASSERT( partInstance != NULL );

    size_t dataSize = resultItemCount( "U", partIndex, stepIndex, frameIndex );
    if ( dataSize > 0 )
    {
        displacements->resize( dataSize );
    }

    const odb_Frame&                 frame               = stepFrame( stepIndex, frameIndex );
    const odb_FieldOutput&           instanceFieldOutput = frame.fieldOutputs()["U"].getSubset( *partInstance );
    const odb_SequenceFieldBulkData& seqFieldBulkData    = instanceFieldOutput.bulkDataBlocks();

    size_t dataIndex = 0;
    int    numBlocks = seqFieldBulkData.size();
    for ( int block = 0; block < numBlocks; block++ )
    {
        const odb_FieldBulkData& bulkData = seqFieldBulkData[block];
        RifOdbBulkDataGetter     bulkDataGetter( bulkData );

        if ( bulkData.numberOfNodes() > 0 )
        {
            int    numNodes = bulkData.length();
            int    numComp  = bulkData.width();
            float* data     = bulkDataGetter.data();

            for ( int i = 0; i < numNodes; i++ )
            {
                ( *displacements )[i + dataIndex].set( data[i * numComp], data[i * numComp + 1], data[i * numComp + 2] );
            }

            dataIndex += numNodes * numComp;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readNodeField( const std::string&                fieldName,
                                  int                               partIndex,
                                  int                               stepIndex,
                                  int                               frameIndex,
                                  std::vector<std::vector<float>*>* resultValues )
{
    CVF_ASSERT( resultValues );

    odb_Instance* partInstance = instance( partIndex );
    CVF_ASSERT( partInstance != NULL );

    size_t compCount = componentsCount( fieldName, NODAL );
    CVF_ASSERT( compCount == resultValues->size() );

    std::map<int, int>& nodeIdToIdxMap = m_nodeIdToIdxMaps[partIndex];

    size_t dataSize = nodeIdToIdxMap.size();

    if ( dataSize > 0 )
    {
        for ( int comp = 0; comp < compCount; comp++ )
        {
            CVF_ASSERT( ( *resultValues )[comp] );

            ( *resultValues )[comp]->resize( dataSize, std::numeric_limits<float>::infinity() );
        }
    }

    const odb_Frame&       frame               = stepFrame( stepIndex, frameIndex );
    const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset( *partInstance );
    const odb_FieldOutput& fieldOutput         = instanceFieldOutput.getSubset( odb_Enum::NODAL );
    const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

    int numBlocks = seqFieldBulkData.size();
    for ( int block = 0; block < numBlocks; block++ )
    {
        const odb_FieldBulkData& bulkData = seqFieldBulkData[block];
        RifOdbBulkDataGetter     bulkDataGetter( bulkData );

        int    numNodes   = bulkData.length();
        int    numComp    = bulkData.width();
        int*   nodeLabels = bulkData.nodeLabels();
        float* data       = bulkDataGetter.data();

        for ( int nIdx = 0; nIdx < numNodes; nIdx++ )
        {
            for ( int comp = 0; comp < numComp; comp++ )
            {
                std::vector<float>* singleComponentValues                    = ( *resultValues )[comp];
                ( *singleComponentValues )[nodeIdToIdxMap[nodeLabels[nIdx]]] = data[nIdx * numComp + comp];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readElementNodeField( const std::string&                fieldName,
                                         int                               partIndex,
                                         int                               stepIndex,
                                         int                               frameIndex,
                                         std::vector<std::vector<float>*>* resultValues )
{
    CVF_ASSERT( resultValues );

    odb_Instance* partInstance = instance( partIndex );
    CVF_ASSERT( partInstance != NULL );

    size_t compCount = componentsCount( fieldName, ELEMENT_NODAL );
    CVF_ASSERT( compCount == resultValues->size() );

    size_t dataSize = resultItemCount( fieldName, partIndex, stepIndex, frameIndex );
    if ( dataSize > 0 )
    {
        for ( int comp = 0; comp < compCount; comp++ )
        {
            CVF_ASSERT( ( *resultValues )[comp] );

            ( *resultValues )[comp]->resize( dataSize, std::numeric_limits<float>::infinity() );
        }
    }

    const odb_Frame&       frame               = stepFrame( stepIndex, frameIndex );
    const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset( *partInstance );
    const odb_FieldOutput& fieldOutput         = instanceFieldOutput.getSubset( odb_Enum::ELEMENT_NODAL );
    const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

    std::map<int, int>& elementIdToIdxMap = m_elementIdToIdxMaps[partIndex];
    CVF_ASSERT( elementIdToIdxMap.size() > 0 );

    int numBlocks = seqFieldBulkData.size();
    for ( int block = 0; block < numBlocks; block++ )
    {
        const odb_FieldBulkData& bulkData = seqFieldBulkData[block];
        RifOdbBulkDataGetter     bulkDataGetter( bulkData );

        int    numValues     = bulkData.length();
        int    numComp       = bulkData.width();
        int    elemCount     = bulkData.numberOfElements();
        int    elemNodeCount = numValues / elemCount;
        int*   elementLabels = bulkData.elementLabels();
        float* data          = bulkDataGetter.data();

        for ( int elem = 0; elem < elemCount; elem++ )
        {
            int elementIdx                  = elementIdToIdxMap[elementLabels[elem * elemNodeCount]];
            int elementResultStartDestIdx   = elementIdx * elemNodeCount; // Ikke generellt riktig !
            int elementResultStartSourceIdx = elem * elemNodeCount * numComp;

            for ( int elemNode = 0; elemNode < elemNodeCount; elemNode++ )
            {
                int destIdx = elementResultStartDestIdx + elemNode;
                int srcIdx  = elementResultStartSourceIdx + elemNode * numComp;

                for ( int comp = 0; comp < numComp; comp++ )
                {
                    ( *( *resultValues )[comp] )[destIdx] = data[srcIdx + comp];
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readIntegrationPointField( const std::string&                fieldName,
                                              int                               partIndex,
                                              int                               stepIndex,
                                              int                               frameIndex,
                                              std::vector<std::vector<float>*>* resultValues )
{
    CVF_ASSERT( resultValues );

    odb_Instance* partInstance = instance( partIndex );
    CVF_ASSERT( partInstance != NULL );

    size_t compCount = componentsCount( fieldName, INTEGRATION_POINT );
    CVF_ASSERT( compCount == resultValues->size() );

    size_t dataSize = resultItemCount( fieldName, partIndex, stepIndex, frameIndex );
    if ( dataSize > 0 )
    {
        for ( int comp = 0; comp < compCount; comp++ )
        {
            CVF_ASSERT( ( *resultValues )[comp] );

            ( *resultValues )[comp]->resize( dataSize, std::numeric_limits<float>::infinity() );
        }
    }

    const odb_Frame&       frame               = stepFrame( stepIndex, frameIndex );
    const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset( *partInstance );
    const odb_FieldOutput& fieldOutput         = instanceFieldOutput.getSubset( odb_Enum::INTEGRATION_POINT );
    const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

    std::map<int, int>& elementIdToIdxMap = m_elementIdToIdxMaps[partIndex];
    CVF_ASSERT( elementIdToIdxMap.size() > 0 );

    int numBlocks = seqFieldBulkData.size();
    for ( int block = 0; block < numBlocks; block++ )
    {
        const odb_FieldBulkData& bulkData = seqFieldBulkData[block];
        RifOdbBulkDataGetter     bulkDataGetter( bulkData );

        int    numValues     = bulkData.length();
        int    numComp       = bulkData.width();
        int    elemCount     = bulkData.numberOfElements();
        int    ipCount       = numValues / elemCount;
        int*   elementLabels = bulkData.elementLabels();
        float* data          = bulkDataGetter.data();

        RigElementType eType = toRigElementType( bulkData.baseElementType() );
        const int*     elmNodeToIpResultMapping =
            localElmNodeToIntegrationPointMapping( eType ); // Todo: Use the one in RigFemTypes.h, but we need to guard
                                                            // against unknown element types first.
        if ( !elmNodeToIpResultMapping ) continue;

        for ( int elem = 0; elem < elemCount; elem++ )
        {
            int elementIdx                  = elementIdToIdxMap[elementLabels[elem * ipCount]];
            int elementResultStartDestIdx   = elementIdx * ipCount; // Ikke generellt riktig !
            int elementResultStartSourceIdx = elem * ipCount * numComp;

            for ( int ipIdx = 0; ipIdx < ipCount; ipIdx++ )
            {
                int resultIpIdx = elmNodeToIpResultMapping[ipIdx];
                int destIdx     = elementResultStartDestIdx + ipIdx;
                int srcIdx      = elementResultStartSourceIdx + resultIpIdx * numComp;

                for ( int comp = 0; comp < numComp; comp++ )
                {
                    ( *( *resultValues )[comp] )[destIdx] = data[srcIdx + comp];
                }
            }
        }
    }
}
