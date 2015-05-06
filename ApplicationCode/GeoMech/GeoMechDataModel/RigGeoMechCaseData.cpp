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

#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RifGeoMechReaderInterface.h"

#ifdef USE_ODB_API 
#include "RifOdbReader.h"
#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::RigGeoMechCaseData(const std::string& fileName)
{
    m_geoMechCaseFileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::~RigGeoMechCaseData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartCollection* RigGeoMechCaseData::femParts()
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFemPartCollection* RigGeoMechCaseData::femParts() const
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::openAndReadFemParts()
{

#ifdef USE_ODB_API    
    m_readerInterface = new RifOdbReader;
#endif

    if (m_readerInterface.notNull() && m_readerInterface->openFile(m_geoMechCaseFileName))
    {
        m_femParts = new RigFemPartCollection();

        if (m_readerInterface->readFemParts(m_femParts.p()))
        {
            // Initialize results containers
            m_femPartResults.resize(m_femParts->partCount());
            std::vector<std::string> stepNames = m_readerInterface->stepNames();
            for (int pIdx = 0; pIdx < m_femPartResults.size(); ++pIdx)
            {
                m_femPartResults[pIdx]->initResultStages(stepNames);
            }
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RigGeoMechCaseData::scalarFieldAndComponentNames(RigFemResultPosEnum resPos)
{
    std::map<std::string, std::vector<std::string> >  fieldCompNames;

    if (m_readerInterface.notNull())
    {
        if (resPos == RIG_NODAL)
        {
            fieldCompNames = m_readerInterface->scalarNodeFieldAndComponentNames();
        }
        else if (resPos == RIG_ELEMENT_NODAL)
        {
            fieldCompNames = m_readerInterface->scalarElementNodeFieldAndComponentNames();
        }
        else if (resPos == RIG_INTEGRATION_POINT)
        {
            fieldCompNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();
        }
    }

    return fieldCompNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigGeoMechCaseData::findOrLoadScalarResult(size_t partIndex, size_t stepIndex, 
                                                                      RigFemResultPosEnum resultPosType, 
                                                                      const std::string& fieldName, 
                                                                      const std::string& componentName)
{
    CVF_ASSERT(partIndex < m_femParts->partCount());

    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult(stepIndex, resultPosType, fieldName, componentName);
    if (frames) return frames;

    std::vector<double > frameTimes = m_readerInterface->frameTimes(stepIndex);
    frames = m_femPartResults[partIndex]->createScalarResult( stepIndex, resultPosType, fieldName, componentName, frameTimes);

    for (size_t fIdx = 0; fIdx < frameTimes.size(); ++fIdx)
    {
        std::vector<float>* frameData = &(frames->frameData(fIdx));
        switch (resultPosType)
        {
            case RIG_NODAL:
                m_readerInterface->readScalarNodeField(fieldName, componentName, partIndex, stepIndex, fIdx, frameData);
            break;
            case RIG_ELEMENT_NODAL:
                //m_readerInterface->readScalarElementNodeField(fieldName, componentName, partIndex, stepIndex, fIdx, frameData);
            break;
            case RIG_INTEGRATION_POINT:
                 //m_readerInterface->readScalarIntegrationPointField(fieldName, componentName, partIndex, stepIndex, fIdx, frameData);
           break;
        }
    }
}
