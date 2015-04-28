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

#include "RifGeoMechReaderInterface.h"
#include <string>
#include <map>

class RigFemPartCollection;

//==================================================================================================
//
// Data interface base class
//
//==================================================================================================
class RifOdbReader : public RifGeoMechReaderInterface
{
   
public:
    RifOdbReader();
    virtual ~RifOdbReader();

    virtual bool                 readFemParts(const std::string& fileName, RigFemPartCollection* femParts);
   
    virtual std::vector<double>  timeSteps(); 
    virtual std::vector<std::string> scalarNodeResultNames() {return std::vector<std::string> ();};
    virtual void readScalarNodeResult(const std::string& resultName, int partIndex, int stepIndex, std::vector<float>* resultValues ) {};

private:
};
