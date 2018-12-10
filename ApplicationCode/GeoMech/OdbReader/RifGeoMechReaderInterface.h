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
#include "cvfVector3.h"

#include <vector>
#include <map>
#include <string>

class RigFemPartCollection;


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

    virtual bool                                             openFile(const std::string& fileName, std::string* errorMessage) = 0;
    virtual bool                                             isOpen() const = 0;
    virtual bool                                             readFemParts(RigFemPartCollection* geoMechCase) = 0;
    virtual std::vector<std::string>                         allStepNames() const = 0;
    virtual std::vector<std::string>                         filteredStepNames() const = 0;
    virtual std::vector<double>                              frameTimes(int stepIndex) const = 0;
  
    virtual std::vector<std::string>                         elementSetNames(int partIndex) = 0;
    virtual std::vector<size_t>                              elementSet(int partIndex, int setIndex) = 0;

    virtual std::map<std::string, std::vector<std::string> > scalarNodeFieldAndComponentNames() = 0; 
    virtual std::map<std::string, std::vector<std::string> > scalarElementNodeFieldAndComponentNames() = 0; 
    virtual std::map<std::string, std::vector<std::string> > scalarIntegrationPointFieldAndComponentNames() = 0; 

    virtual void                                             readDisplacements(int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements) = 0;

    virtual void                                             readNodeField(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex, std::vector<std::vector<float>*>* resultValues) = 0;
    virtual void                                             readElementNodeField(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex, std::vector<std::vector<float>*>* resultValues) = 0;
    virtual void                                             readIntegrationPointField(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex, std::vector<std::vector<float>*>* resultValues) = 0;

    void                                                     setTimeStepFilter(const std::vector<size_t>& fileTimeStepIndices);
    bool                                                     isTimeStepIncludedByFilter(int timeStepIndex) const;
    int                                                      timeStepIndexOnFile(int timeStepIndex) const;
private:
    std::vector<int>                                         m_fileTimeStepIndices;
};
