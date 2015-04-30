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

#include "RifOdbReader.h"

#include "RigFemPartCollection.h"
#include "RigFemPart.h"

#include <odb_API.h>

#include <map>
#include <iostream>

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


void readOdbFile(const std::string& fileName, RigFemPartCollection* femParts)
{
    CVF_ASSERT(femParts);

	RifOdbReader::initializeOdbAPI();

    odb_String path = fileName.c_str();

    odb_Odb& odb = openOdb(path);

    odb_Assembly&  rootAssembly = odb.rootAssembly();
    odb_InstanceRepository instanceRepository = odb.rootAssembly().instances();

    odb_InstanceRepositoryIT iter(instanceRepository);

    for (iter.first(); !iter.isDone(); iter.next())
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

        // Extract elements
        const odb_SequenceElement& elements = inst.elements();

        int elmCount = elements.size();
        femPart->preAllocateElementStorage(elmCount);
        std::map<std::string, RigElementType>::const_iterator it;
        std::vector<int> indexBasedConnectivities;

        for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
        {
            const odb_Element odbElm = elements.element(elmIdx);

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

        femPart->setElementPartId(femParts->partCount());
        femParts->addFemPart(femPart);
    }
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
					if (fieldLocation.position() == resultPosition || resultPosition == odb_Enum::odb_ResultPositionEnum::UNDEFINED_POSITION)
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
bool RifOdbReader::readFemParts(const std::string& fileName, RigFemPartCollection* femParts)
{
    int status = 0;

    try 
    {
        readOdbFile(fileName, femParts);
    }

    catch (const nex_Exception& nex) 
    {
        status = 1;
        fprintf(stderr, "%s\n", nex.UserReport().CStr());
        fprintf(stderr, "ODB Application exited with error(s)\n");
    }

    catch (...) 
    {
        status = 1;
        fprintf(stderr, "ODB Application exited with error(s)\n");
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
std::map<std::string, std::vector<std::string> > RifOdbReader::scalarNodeFieldAndComponentNames() const
{
	return scalarFieldAndComponentNames(m_odb, odb_Enum::odb_ResultPositionEnum::NODAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RifOdbReader::scalarElementNodeFieldAndComponentNames() const
{
	return scalarFieldAndComponentNames(m_odb, odb_Enum::odb_ResultPositionEnum::ELEMENT_NODAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RifOdbReader::scalarIntegrationPointFieldAndComponentNames() const
{
	return scalarFieldAndComponentNames(m_odb, odb_Enum::odb_ResultPositionEnum::INTEGRATION_POINT);
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
/// Get the number of result items (== #nodes or #elements)
//--------------------------------------------------------------------------------------------------
size_t RifOdbReader::resultItemCount(const std::string& fieldName, int stepIndex, int frameIndex) const
{
	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutputRepository& fieldOutputRepo = frame.fieldOutputs();
	
	odb_String fieldNameStr = fieldName.c_str();
	CVF_ASSERT(fieldOutputRepo.isMember(fieldNameStr));

	const odb_FieldOutput& fieldOutput = fieldOutputRepo[fieldNameStr];
	const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

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
int RifOdbReader::componentIndex(const std::string& fieldName, const std::string& componentName) const
{
	std::map<std::string, std::vector<std::string> > resultCompMap = scalarFieldAndComponentNames(m_odb, odb_Enum::odb_ResultPositionEnum::UNDEFINED_POSITION);

	std::vector<std::string> comps;

	auto mapIt = resultCompMap.find(fieldName);
	if (mapIt != resultCompMap.end())
	{
		comps = mapIt->second;
	}
	
	for (size_t i = 0; i < comps.size(); i++)
	{
		if (comps[i] == componentName)
		{
			return i;
		}
	}

	return -1;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readScalarNodeField(const std::string& fieldName, const std::string& componentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues)
{
	CVF_ASSERT(resultValues);

	size_t dataSize = resultItemCount(fieldName, stepIndex, frameIndex);
	if (dataSize > 0)
	{
		resultValues->resize(dataSize);
	}

	int compIndex = componentIndex(fieldName, componentName);
	CVF_ASSERT(compIndex >= 0);

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& fieldOutput = frame.fieldOutputs()[fieldName.c_str()];
	const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

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
				(*resultValues)[dataIndex++] = data[i*numComp + compIndex];
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readScalarElementNodeField(const std::string& fieldName, const std::string& componentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues)
{
	CVF_ASSERT(resultValues);

	// TODO:
	// Need example files containing element node results for testing
	// Or, consider reporting integration point results as element node results too, and extrapolate when requested
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readScalarIntegrationPointField(const std::string& fieldName, const std::string& componentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues)
{
	CVF_ASSERT(resultValues);

	// TODO
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::readDisplacements(int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements)
{
	CVF_ASSERT(displacements);

	size_t dataSize = resultItemCount("U", stepIndex, frameIndex);
	if (dataSize > 0)
	{
		displacements->resize(dataSize);
	}

	const odb_Frame& frame = stepFrame(stepIndex, frameIndex);
	const odb_FieldOutput& fieldOutput = frame.fieldOutputs()["U"];
	const odb_SequenceFieldBulkData& seqFieldBulkData = fieldOutput.bulkDataBlocks();

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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::openFile(const std::string& fileName)
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
		m_odb = &openOdb(path);
	}

	catch (const nex_Exception& nex) 
    {
        fprintf(stderr, "%s\n", nex.UserReport().CStr());
        fprintf(stderr, "ODB Application exited with error(s)\n");
    }

    catch (...) 
    {
        fprintf(stderr, "ODB Application exited with error(s)\n");
    }

	return true;
}
