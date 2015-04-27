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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cvfObject.h"

class RimGeoMechView;
class RigGeoMechCaseData;
//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechCase : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase(void);
    virtual ~RimGeoMechCase(void);
    
    void setFileName(const QString& fileName) {m_caseFileName = fileName;}
    bool openGeoMechCase();

    RigGeoMechCaseData* geoMechData() { return m_geoMechCaseData.p(); }
    const RigGeoMechCaseData* geoMechData() const { return m_geoMechCaseData.p(); }

    RimGeoMechView* createAndAddReservoirView();

    virtual caf::PdmFieldHandle* userDescriptionField();

     // Fields:                                        
    caf::PdmField<QString>                      caseUserDescription;
    caf::PdmPointersField<RimGeoMechView*>      geoMechViews;

private:
    cvf::ref<RigGeoMechCaseData>                m_geoMechCaseData;
    caf::PdmField<QString>                      m_caseFileName;
};
