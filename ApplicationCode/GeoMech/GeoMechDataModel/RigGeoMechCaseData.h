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

#include "RigFemPartResults.h"
#include "RigFemResultPosEnum.h"

#include "cvfObject.h"

#include <string>
#include <vector>

class RifGeoMechReaderInterface;
class RigFemPartCollection;
class RigFemPartResultsCollection;
class RifElementPropertyReader;

class RigGeoMechCaseData : public cvf::Object
{
public:
    explicit RigGeoMechCaseData( const std::string& fileName );
    ~RigGeoMechCaseData() override;

    bool open( std::string* errorMessage );
    bool readTimeSteps( std::string* errorMessage, std::vector<std::string>* stepNames );
    bool readFemParts( std::string* errorMessage, const std::vector<size_t>& timeStepFilter = std::vector<size_t>() );
    RigFemPartCollection*       femParts();
    const RigFemPartCollection* femParts() const;

    RigFemPartResultsCollection*       femPartResults();
    const RigFemPartResultsCollection* femPartResults() const;

private:
    std::string                           m_geoMechCaseFileName;
    cvf::ref<RigFemPartCollection>        m_femParts;
    cvf::ref<RigFemPartResultsCollection> m_femPartResultsColl;
    cvf::ref<RifGeoMechReaderInterface>   m_readerInterface;
    cvf::ref<RifElementPropertyReader>    m_elementPropertyReader;
};
