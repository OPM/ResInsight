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
    
    virtual QString     summaryHeaderFilename() const = 0; 
    virtual QString     caseName() = 0; 
    QString             shortName() const;

    void                updateAutoShortName();
    void                updateOptionSensitivity();

    void                loadCase();
    void                reloadCase();
    RigSummaryCaseData* caseData();

    virtual void        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) = 0;

    bool                isObservedData();

protected:
    void                updateTreeItemName();

    caf::PdmField<QString>          m_shortName;
    caf::PdmField<bool>             m_useAutoShortName;
    caf::PdmField<QString>          m_summaryHeaderFilename;
    cvf::ref<RigSummaryCaseData>    m_summaryCaseData;
    bool                            m_isObservedData;

private:
    // Overridden PDM methods
    virtual void        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void        initAfterRead() override;
};
