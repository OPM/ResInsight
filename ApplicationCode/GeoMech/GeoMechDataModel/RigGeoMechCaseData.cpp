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

#include <stdlib.h>

#include "RifElementPropertyReader.h"
#include "RifGeoMechReaderInterface.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#ifdef USE_ODB_API 
#include "RifOdbReader.h"
#endif

#include "RigFemScalarResultFrames.h"
#include "RigStatisticsDataCache.h"

#include "cafProgressInfo.h"
#include "cvfBoundingBox.h"

#include <QString>
#include <cmath>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::RigGeoMechCaseData(const std::string& fileName)
    : m_geoMechCaseFileName(fileName)
{

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
const RigFemPartResultsCollection* RigGeoMechCaseData::femPartResults() const
{
    return m_femPartResultsColl.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartResultsCollection* RigGeoMechCaseData::femPartResults()
{
    return m_femPartResultsColl.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::openAndReadFemParts(std::string* errorMessage)
{

#ifdef USE_ODB_API    
    m_readerInterface = new RifOdbReader;
#endif

    if (m_readerInterface.notNull() && m_readerInterface->openFile(m_geoMechCaseFileName, errorMessage))
    {
        m_femParts = new RigFemPartCollection();

        caf::ProgressInfo progress(10, ""); // Here because the next call uses progress
        progress.setNextProgressIncrement(9);
        if (m_readerInterface->readFemParts(m_femParts.p()))
        {
            progress.incrementProgress();
            progress.setProgressDescription("Calculating element neighbors");

            m_elementPropertyReader = new RifElementPropertyReader(m_femParts->part(0)->elementIdxToId());
            // Initialize results containers
            m_femPartResultsColl = new RigFemPartResultsCollection(m_readerInterface.p(), m_elementPropertyReader.p(), m_femParts.p());

            // Calculate derived Fem data
            for (int pIdx = 0; pIdx < m_femParts->partCount(); ++pIdx)
            {
                m_femParts->part(pIdx)->assertNodeToElmIndicesIsCalculated();
                m_femParts->part(pIdx)->assertElmNeighborsIsCalculated();
            }
            return true;
        }
    }
    return false;
}
