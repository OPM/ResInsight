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
#include <map>
#include <string>

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
    ~RifOdbReader() override;

    bool                     openFile( const std::string& fileName, std::string* errorMessage ) override;
    bool                     isOpen() const override;
    bool                     readFemParts( RigFemPartCollection* geoMechCase ) override;
    std::vector<std::string> allStepNames() const override;
    std::vector<std::string> filteredStepNames() const override;
    std::vector<double>      frameTimes( int stepIndex ) const override;

    std::vector<std::string> elementSetNames( int partIndex ) override;
    std::vector<size_t>      elementSet( int partIndex, int setIndex ) override;

    std::map<std::string, std::vector<std::string>> scalarNodeFieldAndComponentNames() override;
    std::map<std::string, std::vector<std::string>> scalarElementNodeFieldAndComponentNames() override;
    std::map<std::string, std::vector<std::string>> scalarIntegrationPointFieldAndComponentNames() override;

    void readDisplacements( int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements ) override;

    void readNodeField( const std::string&                fieldName,
                        int                               partIndex,
                        int                               stepIndex,
                        int                               frameIndex,
                        std::vector<std::vector<float>*>* resultValues ) override;
    void readElementNodeField( const std::string&                fieldName,
                               int                               partIndex,
                               int                               stepIndex,
                               int                               frameIndex,
                               std::vector<std::vector<float>*>* resultValues ) override;
    void readIntegrationPointField( const std::string&                fieldName,
                                    int                               partIndex,
                                    int                               stepIndex,
                                    int                               frameIndex,
                                    std::vector<std::vector<float>*>* resultValues ) override;

private:
    enum ResultPosition
    {
        NODAL,
        ELEMENT_NODAL,
        INTEGRATION_POINT,
        NONE
    };

    class RifOdbResultKey
    {
    public:
        RifOdbResultKey( ResultPosition aResultPostion, const std::string& aFieldName )
            : resultPostion( aResultPostion )
            , fieldName( aFieldName ){};

        ResultPosition resultPostion;
        std::string    fieldName;

        bool operator<( const RifOdbResultKey& other ) const
        {
            if ( resultPostion != other.resultPostion )
            {
                return ( resultPostion < other.resultPostion );
            }

            return ( fieldName < other.fieldName );
        }
    };

    void             assertMetaDataLoaded();
    void             close();
    size_t           resultItemCount( const std::string& fieldName,
                                      int                partIndex,
                                      int                stepIndex,
                                      int                frameIndex,
                                      ResultPosition     resultPosition );
    size_t           componentsCount( const std::string& fieldName, ResultPosition position );
    const odb_Frame& stepFrame( int stepIndex, int frameIndex ) const;
    odb_Instance*    instance( int instanceIndex );

    std::vector<std::string>                            componentNames( const RifOdbResultKey& result );
    std::map<std::string, std::vector<std::string>>     fieldAndComponentNames( ResultPosition position );
    std::map<RifOdbResultKey, std::vector<std::string>> readResultsMetaData( odb_Odb* odb );

private:
    odb_Odb* m_odb;

    std::map<RifOdbResultKey, std::vector<std::string>> m_resultsMetaData;
    std::map<int, std::vector<std::string>>             m_partElementSetNames;
    std::vector<std::map<int, int>>                     m_nodeIdToIdxMaps;
    std::vector<std::map<int, int>>                     m_elementIdToIdxMaps;

    static size_t sm_instanceCount;
};
