/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cafAppEnum.h"

class RimEclipseWell;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFlowDiagSolution : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;
public:
    RimFlowDiagSolution(void);
    virtual ~RimFlowDiagSolution(void);

    QString userDescription() { return m_userDescription();} 

    std::set<QString> tracerNames();

    enum TracerStatusType
    {
        PRODUCER, 
        INJECTOR, 
        VARYING, 
        UNDEFINED
    };

    TracerStatusType tracerStatus(QString tracerName);

protected:
    //virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

private:
    virtual caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmField<QString> m_userDescription;

    //caf::PdmPtrArrayField<RimEclipseWell*> m_selectedWells;
};
