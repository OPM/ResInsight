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

#include "RigGeoMechCaseData.h"
#include "RigFemPart.h"

#include <odb_API.h>

#include <map>

std::map<std::string, RigElementType> initFemTypeMap()
{
    std::map<std::string, RigElementType> typeMap;
    typeMap["C3D8R"] = HEX8;
    typeMap["CAX4"] = CAX4;

    return typeMap;
}

static std::map<std::string, RigElementType> odbElmTypeToRigElmTypeMap = initFemTypeMap();

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifOdbReader::RifOdbReader()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifOdbReader::~RifOdbReader()
{

}

void readOdbFile(const std::string& fileName, RigGeoMechCaseData* geoMechCase)
{
    CVF_ASSERT(geoMechCase);

    odb_String path = fileName.c_str();

    odb_Odb& odb = openOdb(path);

    odb_Assembly&  rootAssembly = odb.rootAssembly();
    odb_InstanceRepository instanceRepository = odb.rootAssembly().instances();

    odb_InstanceRepositoryIT iter(instanceRepository);

    for (iter.first(); !iter.isDone(); iter.next())
    {
        odb_Instance& inst = instanceRepository[iter.currentKey()];

        RigFemPart* femPart = new RigFemPart;

        const odb_SequenceNode& odbNodes = inst.nodes();

        int nodeCount = odbNodes.size();
        femPart->nodes().nodeIds.resize(nodeCount);
        femPart->nodes().coordinates.resize(nodeCount);

        for (int nIdx = 0; nIdx < nodeCount; ++nIdx)
        {
            const odb_Node odbNode = odbNodes.node(nIdx);
            femPart->nodes().nodeIds[nIdx] = odbNode.label();
            const float * pos = odbNode.coordinates();
            femPart->nodes().coordinates[nIdx].set(pos[0], pos[1], pos[2]);
        }

        const odb_SequenceElement& elements = inst.elements();

        int elmCount = elements.size();
        femPart->preAllocateElementStorage(elmCount);
        std::map<std::string, RigElementType>::const_iterator it;
        for (int elmIdx = 0; elmIdx < elmCount; ++elmIdx)
        {
            const odb_Element odbElm = elements.element(elmIdx);
            it = odbElmTypeToRigElmTypeMap.find(odbElm.type().cStr());

            if (it == odbElmTypeToRigElmTypeMap.end()) continue;

            RigElementType elmType = it->second;
            int nodeCount = 0;
            femPart->appendElement(elmType, odbElm.label(), odbElm.connectivity(nodeCount));
        }

        femPart->setElementPartId(geoMechCase->partCount());
        geoMechCase->addFemPart(femPart);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifOdbReader::open(const std::string& fileName, RigGeoMechCaseData* geoMechCase)
{
    odb_initializeAPI();

    int status = 0;

    try 
    {
        readOdbFile(fileName, geoMechCase);
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

    odb_finalizeAPI();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifOdbReader::close()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RifOdbReader::timeSteps()
{
    return std::vector<double>();
}
