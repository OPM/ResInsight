/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include <vector>

class RigGeoMechCaseData;


//==================================================================================================
//
// Data interface base class
//
//==================================================================================================
class RifGeoMechReaderInterface : public cvf::Object
{
public:
    RifGeoMechReaderInterface();
    virtual ~RifGeoMechReaderInterface();

    virtual bool                readFemParts(const std::string& fileName, RigGeoMechCaseData* geoMechCase) = 0;
    virtual void                close() = 0;
 
    virtual std::vector<double>  timeSteps() = 0;
    virtual std::vector<std::string> scalarNodeResultNames() = 0; 
    virtual void readScalarNodeResult(const std::string& resultName, int partIndex, int stepIndex, std::vector<float>* resultValues ) = 0;

private:
};
