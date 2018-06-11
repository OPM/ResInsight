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
class odb_Instance;
class odb_SequenceFieldBulkData;

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

    virtual bool                                                openFile(const std::string& fileName, std::string* errorMessage);
    virtual bool                                                isOpen() const;
    virtual bool                                                readFemParts(RigFemPartCollection* geoMechCase);
    virtual std::vector<std::string>                            allStepNames() const override;
    virtual std::vector<std::string>                            filteredStepNames() const override;
    virtual std::vector<double>                                 frameTimes(int stepIndex) const override;

    virtual std::vector<std::string>                            elementSetNames(int partIndex);
    virtual std::vector<size_t>                                 elementSet(int partIndex, int setIndex);
    
    virtual std::map<std::string, std::vector<std::string> >    scalarNodeFieldAndComponentNames(); 
    virtual std::map<std::string, std::vector<std::string> >    scalarElementNodeFieldAndComponentNames(); 
    virtual std::map<std::string, std::vector<std::string> >    scalarIntegrationPointFieldAndComponentNames(); 

    virtual void                                                readDisplacements(int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements);

    virtual void                                                readNodeField(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex, std::vector<std::vector<float>*>* resultValues);
    virtual void                                                readElementNodeField(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex, std::vector<std::vector<float>*>* resultValues);
    virtual void                                                readIntegrationPointField(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex, std::vector<std::vector<float>*>* resultValues);

private:
    enum ResultPosition
    {
        NODAL,
        ELEMENT_NODAL,
        INTEGRATION_POINT
    };

    class RifOdbResultKey
    {
    public:
        RifOdbResultKey(ResultPosition aResultPostion, const std::string& aFieldName)
                        : resultPostion(aResultPostion), fieldName(aFieldName) {};

        ResultPosition  resultPostion;
        std::string     fieldName;

        bool operator< (const RifOdbResultKey& other) const
        {
            if (resultPostion != other.resultPostion)
            {
                return (resultPostion < other.resultPostion);
            }

            return (fieldName < other.fieldName);
       }
    };

    void                                                    assertMetaDataLoaded();
    void                                                    close();
    size_t                                                  resultItemCount(const std::string& fieldName, int partIndex, int stepIndex, int frameIndex);
    size_t                                                  componentsCount(const std::string& fieldName, ResultPosition position);
    const odb_Frame&                                        stepFrame(int stepIndex, int frameIndex) const;
    odb_Instance*                                           instance(int instanceIndex);

    int                                                     componentIndex(const RifOdbResultKey& result, const std::string& componentName);
    std::vector<std::string>                                componentNames(const RifOdbResultKey& result);
    std::map< std::string, std::vector<std::string> >       fieldAndComponentNames(ResultPosition position); 
    std::map< RifOdbResultKey, std::vector<std::string> >   readResultsMetaData(odb_Odb* odb);
 
private:
    odb_Odb*                                                m_odb;

    std::map< RifOdbResultKey, std::vector<std::string> >   m_resultsMetaData;
    std::map< int, std::vector<std::string> >               m_partElementSetNames;
    std::vector< std::map<int, int> >                       m_nodeIdToIdxMaps;
    std::vector< std::map<int, int> >                       m_elementIdToIdxMaps;

    static size_t                                           sm_instanceCount;
};
