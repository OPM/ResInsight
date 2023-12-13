/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RigFemResultPosEnum.h"
#include "RigFemTypes.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <utility>

class RigFemPartCollection;

struct RifInpIncludeEntry
{
public:
    RifInpIncludeEntry( std::string propertyName, RigFemResultPosEnum resultType, int stepId, std::string fileName, int columnIndex )
    {
        this->propertyName = propertyName;
        this->stepId       = stepId;
        this->fileName     = fileName;
        this->resultType   = resultType;
        this->columnIndex  = columnIndex;
    }

public:
    std::string         propertyName;
    int                 stepId;
    std::string         fileName;
    RigFemResultPosEnum resultType;
    int                 columnIndex;
};

//==================================================================================================
//
//==================================================================================================
class RifInpReader : public RifGeoMechReaderInterface
{
public:
    RifInpReader();
    ~RifInpReader() override;

    void enableIncludes( bool enable );

    bool                     openFile( const std::string& fileName, std::string* errorMessage ) override;
    bool                     isOpen() const override;
    bool                     readFemParts( RigFemPartCollection* geoMechCase ) override;
    std::vector<std::string> allStepNames() const override;
    std::vector<std::string> filteredStepNames() const override;
    std::vector<double>      frameTimes( int stepIndex ) const override;
    int                      frameCount( int stepIndex ) const override;

    std::vector<std::string> elementSetNames( int partIndex, std::string partName ) override;
    std::vector<size_t>      elementSet( int partIndex, std::string partName, int setIndex ) override;

    std::map<std::string, std::vector<std::string>> scalarNodeFieldAndComponentNames() override;
    std::map<std::string, std::vector<std::string>> scalarElementFieldAndComponentNames() override;
    std::map<std::string, std::vector<std::string>> scalarElementNodeFieldAndComponentNames() override;
    std::map<std::string, std::vector<std::string>> scalarIntegrationPointFieldAndComponentNames() override;

    void readDisplacements( int partIndex, int stepIndex, int frameIndex, std::vector<cvf::Vec3f>* displacements ) override;

    void readNodeField( const std::string&                fieldName,
                        int                               partIndex,
                        int                               stepIndex,
                        int                               frameIndex,
                        std::vector<std::vector<float>*>* resultValues ) override;
    void readElementField( const std::string&                fieldName,
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

    bool populateDerivedResultNames() const override;

private:
    void close();

    void readField( RigFemResultPosEnum               resultType,
                    const std::string&                fieldName,
                    int                               partIndex,
                    int                               stepIndex,
                    std::vector<std::vector<float>*>* resultValues );

    void readScalarData( RigFemPartCollection* femParts, std::map<int, std::string>& parts, std::vector<RifInpIncludeEntry>& includeEntries );

    std::map<std::string, std::map<int, std::map<int, std::vector<double>>>>* propertyDataMap( RigFemResultPosEnum resultType );

    static void                                          skipComments( std::istream& stream );
    static std::string                                   parseLabel( const std::string& line, const std::string& labelName );
    static std::vector<std::pair<int, cvf::Vec3d>>       readNodes( std::istream& stream );
    static std::vector<std::pair<int, std::vector<int>>> readElements( std::istream& stream );
    static std::vector<size_t>                           readElementSet( std::istream& stream );
    static std::vector<size_t>                           readElementSetGenerate( std::istream& stream );

    static RigElementType read( std::istream&                                                            stream,
                                std::map<int, std::string>&                                              parts,
                                std::map<int, std::vector<std::pair<int, cvf::Vec3d>>>&                  nodes,
                                std::map<int, std::vector<std::pair<int, std::vector<int>>>>&            elements,
                                std::map<int, std::vector<std::pair<std::string, std::vector<size_t>>>>& elementSets,
                                std::vector<std::string>&                                                stepNames,
                                bool                                                                     enableIncludes,
                                std::vector<RifInpIncludeEntry>&                                         includeEntries );

private:
    bool                                                                     m_enableIncludes;
    std::map<int, std::vector<std::string>>                                  m_partElementSetNames;
    std::vector<std::string>                                                 m_stepNames;
    std::vector<RifInpIncludeEntry>                                          m_includeEntries;
    std::ifstream                                                            m_stream;
    std::filesystem::path                                                    m_inputPath;
    std::map<std::string, std::map<int, std::map<int, std::vector<double>>>> m_propertyPartDataNodes;
    std::map<std::string, std::map<int, std::map<int, std::vector<double>>>> m_propertyPartDataElements;
};
