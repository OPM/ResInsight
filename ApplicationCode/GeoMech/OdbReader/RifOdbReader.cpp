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

    RigFemPart* part = new RigFemPart;

    const odb_SequenceNode& nodes = rootAssembly.nodes();

    const odb_SequenceElement& elements = rootAssembly.elements();

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
