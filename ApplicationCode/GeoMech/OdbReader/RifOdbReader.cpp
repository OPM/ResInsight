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

// Get rid of warnings from compilation of ODB API
#ifdef _MSC_VER
#pragma warning(disable: 4482)
#pragma warning(disable: 4584)
#endif

#include "RifOdbReader.h"

#include "RigFemPartCollection.h"
#include "RigFemPart.h"

#include <odb_API.h>

#include <map>
#include <iostream>
#include <limits>
#include <sstream>


std::map<std::string, RigElementType> initFemTypeMap()
{
    std::map<std::string, RigElementType> typeMap;
    typeMap["C3D8R"] = HEX8;
    typeMap["C3D8"]  = HEX8;
    typeMap["C3D8P"] = HEX8;
    typeMap["CAX4"] = CAX4;

    return typeMap;
}

static std::map<std::string, RigElementType> odbElmTypeToRigElmTypeMap = initFemTypeMap();

bool RifOdbReader::sm_odbAPIInitialized = false;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifOdbReader::RifOdbReader()
{
	m_odb = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifOdbReader::~RifOdbReader()
{
	close();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::initializeOdbAPI()
{
	if (!sm_odbAPIInitialized)
	{
		odb_initializeAPI();
		sm_odbAPIInitialized = true;
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::finalizeOdbAPI()
{
	if (sm_odbAPIInitialized)
	{
		odb_finalizeAPI();
		sm_odbAPIInitialized = false;
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::close()
{
	if (m_odb)
	{
		m_odb->close();
		m_odb = NULL;
	}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::openFile(const std::string& fileName, std::string* errorMessage)
{
	close();
	CVF_ASSERT(m_odb == NULL);

	if (!sm_odbAPIInitialized)
	{
		initializeOdbAPI();
	}

	odb_String path = fileName.c_str();

	try
	{
		m_odb = &openOdb(path, true);
	}

	catch (const nex_Exception& nex) 
    {
        if (errorMessage)
        {
            *errorMessage = nex.UserReport().CStr();
        }

        return false;
    }

    catch (...) 
    {
        if (errorMessage)
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
bool RifOdbReader::buildMetaData()
{
	CVF_ASSERT(m_odb != NULL);

	m_resultsMetaData = resultsMetaData(m_odb);

	return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > scalarFieldAndComponentNames(odb_Odb* odb, odb_Enum::odb_ResultPositionEnum resultPosition)
{
	CVF_ASSERT(odb != NULL);

	std::map<std::string, std::vector<std::string> > resultNamesMap;

	odb_StepRepository stepRepository = odb->steps();
	odb_StepRepositoryIT sIter(stepRepository);
	for (sIter.first(); !sIter.isDone(); sIter.next()) 
	{
		odb_SequenceFrame& stepFrames = stepRepository[sIter.currentKey()].frames();

		int numFrames = stepFrames.size();
		for (int f = 0; f < numFrames; f++) 
		{
			odb_Frame frame = stepFrames.constGet(f);
			
			odb_FieldOutputRepository& fieldCon = frame.fieldOutputs();
			odb_FieldOutputRepositoryIT fieldConIT(fieldCon);
			for (fieldConIT.first(); !fieldConIT.isDone(); fieldConIT.next()) 
			{
				odb_FieldOutput& field = fieldCon[fieldConIT.currentKey()]; 
			
				odb_SequenceFieldLocation fieldLocations = field.locations();
				for (int loc = 0; loc < fieldLocations.size(); loc++)
				{
					const odb_FieldLocation& fieldLocation = fieldLocations.constGet(loc);
					if (fieldLocation.position() == resultPosition)
					{
						std::string fieldName = field.name().CStr();
						odb_SequenceString components = field.componentLabels();

						std::vector<std::string> compVec;

						int numComp = components.size();
						for (int comp = 0; comp < numComp; comp++)
						{
							compVec.push_back(components[comp].CStr());
						}

						resultNamesMap.insert(std::make_pair(fieldName, compVec));

						break;
					}
				}
			}
		}
	}

	return resultNamesMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map< RifOdbReader::ResPos, std::map<std::string, std::vector<std::string> > > RifOdbReader::resultsMetaData(odb_Odb* odb)
{
	CVF_ASSERT(odb != NULL);

    std::map< ResPos, std::map<std::string, std::vector<std::string> > > resultsMap;

	odb_StepRepository stepRepository = odb->steps();
	odb_StepRepositoryIT sIter(stepRepository);
	for (sIter.first(); !sIter.isDone(); sIter.next()) 
	{
		odb_SequenceFrame& stepFrames = stepRepository[sIter.currentKey()].frames();

		int numFrames = stepFrames.size();
		for (int f = 0; f < numFrames; f++) 
		{
			odb_Frame frame = stepFrames.constGet(f);
			
			odb_FieldOutputRepository& fieldCon = frame.fieldOutputs();
			odb_FieldOutputRepositoryIT fieldConIT(fieldCon);
			for (fieldConIT.first(); !fieldConIT.isDone(); fieldConIT.next()) 
			{
				odb_FieldOutput& field = fieldCon[fieldConIT.currentKey()]; 
			
				odb_SequenceFieldLocation fieldLocations = field.locations();
				for (int loc = 0; loc < fieldLocations.size(); loc++)
				{
					const odb_FieldLocation& fieldLocation = fieldLocations.constGet(loc);

					std::string fieldName = field.name().CStr();
					odb_SequenceString components = field.componentLabels();

					std::vector<std::string> compVec;

					int numComp = components.size();
					for (int comp = 0; comp < numComp; comp++)
					{
						compVec.push_back(components[comp].CStr());
					}

					switch (fieldLocation.position())
					{
						case odb_Enum::odb_ResultPositionEnum::NODAL:
                            resultsMap[ResPos::NODAL][fieldName] = compVec;
							break;

						case odb_Enum::odb_ResultPositionEnum::ELEMENT_NODAL:
                            resultsMap[ResPos::ELEMENT_NODAL][fieldName] = compVec;
							break;

						case odb_Enum::odb_ResultPositionEnum::INTEGRATION_POINT:
                            resultsMap[ResPos::INTEGRATION_POINT][fieldName] = compVec;
							break;

						default:
							break;
					}
				}
			}
		}
	}

	return resultsMap;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::readFemParts(RigFemPartCollection* femParts)
{
    CVF_ASSERT(femParts);
	CVF_ASSERT(m_odb != NULL);

    odb_Assembly&  rootAssembly = m_odb->rootAssembly();
    odb_InstanceRepository instanceRepository = m_odb->rootAssembly().instances();

    odb_InstanceRepositoryIT iter(instanceRepository);

    int instanceCount = 0;
    for (iter.first(); !iter.isDone(); iter.next(), instanceCount++)
    {
        odb_Instance& inst = instanceRepository[iter.currentKey()];

        RigFemPart* femPart = new RigFemPart;

        // Extract nodes
        const odb_SequenceNode& odbNodes = inst.nodes();

        std::map<int, int> nodeIdToIdxMap;

        int nodeCount = odbNodes.size();
        femPart->nodes().nodeIds.resize(nodeCount);
        femPart->nodes().coordinates.resize(nodeCount);

        for (int nIdx = 0; nIdx < nodeCount; ++nIdx)
        {
            const odb_Node odbNode = odbNodes.node(nIdx);
            femPart->nodes().nodeIds[nIdx] = odbNode.label();
            const float * pos = odbNode.coordinates();
            femPart->nodes().coordinates[nIdx].set(pos[0], pos[1], pos[2]);
            nodeIdToIdxMap[odbNode.label()] = nIdx;
        }

        // Keep node id to index map per instance
        m_instanceToNodeIdToIdxMap[instanceCount] = nodeIdToIdxMap;

        // Extract elements
        const odb_SequenceElement& elements = inst.elements();

        int elmCount = elements.size();
        femPart->preAllocateElementStorage(elmCount);
        std::map<std::string, RigElementType>::const_iterator it;
        std::vector<int> indexBasedConnectivities;
        std::map<int, int> elementIdToIdxMap;

        for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
        {
            const odb_Element odbElm = elements.element(elmIdx);

            elementIdToIdxMap[odbElm.label()] = elmIdx;

            // Get the type
            it = odbElmTypeToRigElmTypeMap.find(odbElm.type().cStr());

            if (it == odbElmTypeToRigElmTypeMap.end()) 
            {
                #if 0
                std::cout << "Unsupported element type :" << odbElm.type().cStr() << std::endl;
                #endif
                continue; // Unsupported type
             }

            RigElementType elmType = it->second;

            int nodeCount = 0;
            const int* idBasedConnectivities = odbElm.connectivity(nodeCount);
            CVF_TIGHT_ASSERT(nodeCount == RigFemTypes::elmentNodeCount(elmType));

            indexBasedConnectivities.resize(nodeCount);
            for (int lnIdx = 0; lnIdx < nodeCount; ++lnIdx)
            {
                indexBasedConnectivities[lnIdx] = nodeIdToIdxMap[idBasedConnectivities[lnIdx]];
            }

            femPart->appendElement(elmType, odbElm.label(), indexBasedConnectivities.data());
        }

        // Keep element id to index map per instance
        m_instanceToElementIdToIdxMap[instanceCount] = elementIdToIdxMap;

        femPart->setElementPartId(femParts->partCount());
        femParts->addFemPart(femPart);
    }

	return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOdbReader::stepNames()
{
	CVF_ASSERT(m_odb != NULL);

	std::vector<std::string> stepNames;

	odb_StepRepository stepRepository = m_odb->steps();
	odb_StepRepositoryIT sIter(stepRepository);
	for (sIter.first(); !sIter.isDone(); sIter.next()) 
	{
		stepNames.push_back(stepRepository[sIter.currentKey()].name().CStr());
	}

	return stepNames;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RifOdbReader::frameTimes(int stepIndex)
{
	CVF_ASSERT(m_odb != NULL);

	odb_StepRepository& stepRepository = m_odb->steps();

	odb_StepList stepList = stepRepository.stepList();
	odb_Step& step = stepList.Get(stepIndex);
	
	odb_SequenceFrame& stepFrames = step.frames();

	std::vector<double> frameValues;

	int numFrames = stepFrames.size();
	for (int f = 0; f < numFrames; f++) 
	{
		odb_Frame frame = stepFrames.constGet(f);
		frameValues.push_back(frame.frameValue());
	}

	return frameValues;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RifOdbReader::scalarNodeFieldAndComponentNames()
{
    return fieldAndComponentNames(ResPos::NODAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RifOdbReader::scalarElementNodeFieldAndComponentNames()
{
    return fieldAndComponentNames(ResPos::ELEMENT_NODAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RifOdbReader::scalarIntegrationPointFieldAndComponentNames()
{
    return fieldAndComponentNames(ResPos::INTEGRATION_POINT);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
odb_Frame RifOdbReader::stepFrame(int stepIndex, int frameIndex) const
{
	CVF_ASSERT(m_odb);

	odb_StepRepository& stepRepository = m_odb->steps();
	odb_StepList stepList = stepRepository.stepList();
	odb_Step& step = stepList.Get(stepIndex);
	odb_SequenceFrame& stepFrames = step.frames();

	return stepFrames.constGet(frameIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
odb_Instance* RifOdbReader::instance(int instanceIndex)
{
    CVF_ASSERT(m_odb != NULL);

    odb_Assembly& rootAssembly = m_odb->rootAssembly();
    odb_InstanceRepository& instanceRepository = m_odb->rootAssembly().instances();
    odb_InstanceRepositoryIT iter(instanceRepository);

    int instanceCount = 0;
    for (iter.first(); !iter.isDone(); iter.next(), instanceCount++)
    {
        odb_Instance& inst = instanceRepository[iter.currentKey()];

        if (instanceCount == instanceIndex)
        {
            return &inst;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// Get the number of result items (== #nodes or #elements)
//--------------------------------------------------------------------------------------------------
size_t RifOdbReader::resultItemCount(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex)
{
    odb_Instance* partInstance = instance(partIndex);
    CVF_ASSERT(partInstance != NULL);

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset(*partInstance);
	const odb_SequenceFieldBulkData& seqFieldBulkData = instanceFieldOutput.bulkDataBlocks();

	size_t resultItemCount = 0;
	int numBlocks = seqFieldBulkData.size();

	for (int block = 0; block < numBlocks; block++)
	{
		const odb_FieldBulkData& bulkData =	seqFieldBulkData[block];
		resultItemCount += bulkData.length();
	}

	return resultItemCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifOdbReader::componentIndex(ResPos position, const std::string& fieldName, const std::string& componentName) const
{
	std::vector<std::string> compNames = componentNames(position, fieldName);

	for (size_t i = 0; i < compNames.size(); i++)
	{
		if (compNames[i] == componentName)
		{
			return i;
		}
	}

	return 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOdbReader::componentNames(ResPos position, const std::string& fieldName) const
{
	std::vector<std::string> compNames;

  	auto posMapIt = m_resultsMetaData.find(position);
	if (posMapIt != m_resultsMetaData.end())
	{
        auto fieldNameMapIt = posMapIt->second.find(fieldName);
	    if (fieldNameMapIt != posMapIt->second.end())
	    {
            compNames = fieldNameMapIt->second;
        }
	}
	
	return compNames;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RifOdbReader::fieldAndComponentNames(ResPos position)
{
	if (m_resultsMetaData.empty())
	{
		buildMetaData();
	}

    return m_resultsMetaData[position];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readScalarNodeField(const std::string& fieldName, const std::string& componentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues)
{
	CVF_ASSERT(resultValues);

    odb_Instance* partInstance = instance(partIndex);
    CVF_ASSERT(partInstance != NULL);

    auto nodeIdToIdxMapIt = m_instanceToNodeIdToIdxMap.find(partIndex);
    CVF_ASSERT(nodeIdToIdxMapIt != m_instanceToNodeIdToIdxMap.end());

    auto nodeIdToIdxMap = nodeIdToIdxMapIt->second;
    size_t dataSize = nodeIdToIdxMap.size();
    CVF_ASSERT(dataSize > 0);

	resultValues->resize(dataSize);
    resultValues->assign(dataSize, std::numeric_limits<float>::infinity());

	int compIndex = componentIndex(ResPos::NODAL, fieldName, componentName);
	CVF_ASSERT(compIndex >= 0);

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset(*partInstance);
	const odb_FieldOutput& fieldOutput = instanceFieldOutput.getSubset(odb_Enum::NODAL);
	const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

	size_t dataIndex = 0;
	int numBlocks = seqFieldBulkData.size();
	for (int block = 0; block < numBlocks; block++)
	{
		const odb_FieldBulkData& bulkData =	seqFieldBulkData[block];

		int numNodes = bulkData.length();
		int numComp = bulkData.width();
		float* data = bulkData.data();
        int* nodeLabels = bulkData.nodeLabels();

		for (int i = 0; i < numNodes; i++)
		{
            (*resultValues)[nodeIdToIdxMap[nodeLabels[i]]] = data[i*numComp + compIndex];
		}
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readScalarElementNodeField(const std::string& fieldName, const std::string& componentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues)
{
	CVF_ASSERT(resultValues);

    odb_Instance* partInstance = instance(partIndex);
    CVF_ASSERT(partInstance != NULL);

    auto elementIdToIdxMapIt = m_instanceToElementIdToIdxMap.find(partIndex);
    CVF_ASSERT(elementIdToIdxMapIt != m_instanceToElementIdToIdxMap.end());

    auto elementIdToIdxMap = elementIdToIdxMapIt->second;
    CVF_ASSERT(elementIdToIdxMap.size() > 0);

    size_t dataSize = resultItemCount(fieldName, partIndex, stepIndex, frameIndex);
	if (dataSize > 0)
	{
		resultValues->resize(dataSize);
        resultValues->assign(dataSize, std::numeric_limits<float>::infinity());
	}

	int compIndex = componentIndex(ResPos::ELEMENT_NODAL, fieldName, componentName);
	CVF_ASSERT(compIndex >= 0);

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset(*partInstance);
    const odb_FieldOutput& fieldOutput = instanceFieldOutput.getSubset(odb_Enum::ELEMENT_NODAL);
	const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

	size_t dataIndex = 0;
	int numBlocks = seqFieldBulkData.size();
	for (int block = 0; block < numBlocks; block++)
	{
		const odb_FieldBulkData& bulkData =	seqFieldBulkData[block];

		int numValues = bulkData.length();
		int numComp = bulkData.width();
		float* data = bulkData.data();
        int elemCount = bulkData.numberOfElements();
		int elemNodeCount = numValues/elemCount;
		int* elementLabels = bulkData.elementLabels();

        for (int elem = 0; elem < elemCount; elem++)
        {
            int elementIdx = elementIdToIdxMap[elementLabels[elem*elemNodeCount]];

            for (int elemNode = 0; elemNode < elemNodeCount; elemNode++)
            {
                int destIdx = elementIdx*elemNodeCount + elemNode;
                int srcIdx = elem*elemNodeCount*numComp + elemNode*numComp + compIndex;
                (*resultValues)[destIdx] = data[srcIdx];
            }
        }
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readScalarIntegrationPointField(const std::string& fieldName, const std::string& componentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues)
{
	CVF_ASSERT(resultValues);

    odb_Instance* partInstance = instance(partIndex);
    CVF_ASSERT(partInstance != NULL);

    auto elementIdToIdxMapIt = m_instanceToElementIdToIdxMap.find(partIndex);
    CVF_ASSERT(elementIdToIdxMapIt != m_instanceToElementIdToIdxMap.end());

    auto elementIdToIdxMap = elementIdToIdxMapIt->second;
    CVF_ASSERT(elementIdToIdxMap.size() > 0);

   	size_t dataSize = resultItemCount(fieldName, partIndex, stepIndex, frameIndex);
	if (dataSize > 0)
	{
		resultValues->resize(dataSize);
        resultValues->assign(dataSize, std::numeric_limits<float>::infinity());
	}

	int compIndex = componentIndex(ResPos::INTEGRATION_POINT, fieldName, componentName);
	CVF_ASSERT(compIndex >= 0);

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()[fieldName.c_str()].getSubset(*partInstance);
    const odb_FieldOutput& fieldOutput = instanceFieldOutput.getSubset(odb_Enum::INTEGRATION_POINT);
	const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

	size_t dataIndex = 0;
	int numBlocks = seqFieldBulkData.size();
	for (int block = 0; block < numBlocks; block++)
	{
		const odb_FieldBulkData& bulkData =	seqFieldBulkData[block];

		int numValues = bulkData.length();
		int numComp = bulkData.width();
		float* data = bulkData.data();
        int elemCount = bulkData.numberOfElements();
		int ipCount = numValues/elemCount;
		int* elementLabels = bulkData.elementLabels();

        for (int elem = 0; elem < elemCount; elem++)
        {
            int elementIdx = elementIdToIdxMap[elementLabels[elem*ipCount]];

            for (int ip = 0; ip < ipCount; ip++)
            {
                int destIdx = elementIdx*ipCount + ip;
                int srcIdx = elem*ipCount*numComp + ip*numComp + compIndex;
                (*resultValues)[destIdx] = data[srcIdx];
            }
        }
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readDisplacements(int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements)
{
	CVF_ASSERT(displacements);

    odb_Instance* partInstance = instance(partIndex);
    CVF_ASSERT(partInstance != NULL);

	size_t dataSize = resultItemCount("U", partIndex, stepIndex, frameIndex);
	if (dataSize > 0)
	{
		displacements->resize(dataSize);
	}

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& instanceFieldOutput = frame.fieldOutputs()["U"].getSubset(*partInstance);
	const odb_SequenceFieldBulkData& seqFieldBulkData = instanceFieldOutput.bulkDataBlocks();

	size_t dataIndex = 0;
	int numBlocks = seqFieldBulkData.size();
	for (int block = 0; block < numBlocks; block++)
	{
		const odb_FieldBulkData& bulkData =	seqFieldBulkData[block];

		if (bulkData.numberOfNodes() > 0)
		{
			int numNodes = bulkData.length();
			int numComp = bulkData.width();
			float* data = bulkData.data();

			for (int i = 0; i < numNodes; i++)
			{
				(*displacements)[i + dataIndex].set(data[i*numComp], data[i*numComp + 1], data[i*numComp + 2]);
			}

			dataIndex += numNodes*numComp;
		}
	}
}

