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

class odb_Odb;
class odb_Frame;

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

    virtual bool                                             readFemParts(const std::string& fileName, RigFemPartCollection* geoMechCase);
    virtual std::vector<std::string>                         stepNames();
    virtual std::vector<double>                              frameTimes(int stepIndex);
    
    virtual std::map<std::string, std::vector<std::string> > scalarNodeFieldAndComponentNames() const; 
    virtual std::map<std::string, std::vector<std::string> > scalarElementNodeFieldAndComponentNames() const; 
    virtual std::map<std::string, std::vector<std::string> > scalarIntegrationPointFieldAndComponentNames() const; 
	
    virtual void                                             readScalarNodeField(const std::string& fieldName, const std::string& componmentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues);
    virtual void                                             readScalarElementNodeField(const std::string& fieldName, const std::string& componmentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues);
    virtual void                                             readScalarIntegrationPointField(const std::string& fieldName, const std::string& componmentName, int partIndex, int stepIndex, int frameIndex, std::vector<float>* resultValues);
    virtual void                                             readDisplacements(int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements);

	bool                                                     openFile(const std::string& fileName);

private:
    void                                                     close();
    size_t                                                   resultItemCount(const std::string& fieldName, int stepIndex, int frameIndex) const;
    odb_Frame                                                stepFrame(int stepIndex, int frameIndex) const;
    int                                                      componentIndex(const std::string& fieldName, const std::string& componentName) const;

    static void initializeOdbAPI();
    static void finalizeOdbAPI();

private:
    odb_Odb*     m_odb;
    static bool  sm_odbAPIInitialized;
};
