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

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RifOdbReader.h"
#include "RigGeomechCaseData.h"
#include <QFile>

CAF_PDM_SOURCE_INIT(RimGeoMechCase, "ResInsightGeoMechCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::RimGeoMechCase(void)
{
    CAF_PDM_InitObject("Geomechanical Case", ":/GeoMechCase48x48.png", "", "");

    CAF_PDM_InitField(&caseUserDescription, "CaseUserDescription",  QString(), "Case name", "", "" ,"");
    CAF_PDM_InitField(&m_caseFileName, "CaseFileName", QString(), "Case file name", "", "", "");
    m_caseFileName.setUiReadOnly(true);
    CAF_PDM_InitFieldNoDefault(&geoMechViews, "GeoMechViews", "",  "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::~RimGeoMechCase(void)
{
    geoMechViews.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechCase::createAndAddReservoirView()
{
    RimGeoMechView* gmv = new RimGeoMechView();
    size_t i = geoMechViews().size();
    gmv->name = QString("View %1").arg(i + 1);
    
    gmv->setGeoMechCase(this);

    geoMechViews.push_back(gmv);
    return gmv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGeoMechCase::userDescriptionField()
{
    return &caseUserDescription;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimGeoMechCase::openGeoMechCase()
{
    // If read already, return

    if (this->m_geoMechCaseData.notNull()) return true;


    if (!QFile::exists(m_caseFileName()))
    {
        return false;
    }


#ifdef USE_ODB_API    
    m_readerInterface = new RifOdbReader;

    m_geoMechCaseData = new RigGeoMechCaseData;
    if (m_readerInterface->readFemParts(m_caseFileName().toStdString(), m_geoMechCaseData->femParts()))
    {
    
        // Todo: Default Results stuff, if needed

        return true;
    }
#endif

    return false;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifGeoMechReaderInterface* RimGeoMechCase::readerInterface()
{
    return m_readerInterface.p();
}

