/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cvfObject.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RigSummaryCaseData;

//==================================================================================================
//
// 
//
//==================================================================================================

class RimSummaryCase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimSummaryCase();
    virtual ~RimSummaryCase();
    
    QString caseName(); 

    virtual QString summaryHeaderFilename() const = 0; 
    void loadCase();

    RigSummaryCaseData* caseData() { return m_summaryCaseData.p(); }

    caf::PdmField<QString>  curveDisplayName;
    caf::PdmField<bool>     autoCurveDisplayName;

    void updateOptionSensitivity();
    
protected:
    cvf::ref<RigSummaryCaseData> m_summaryCaseData;

private:
    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

};
