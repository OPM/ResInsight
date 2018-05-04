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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <vector>

class RimSummaryCase;

class RimSummaryCaseCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCaseCollection();
    virtual ~RimSummaryCaseCollection();

    void                            removeCase(RimSummaryCase* summaryCase);
    void                            addCase(RimSummaryCase* summaryCase);
    std::vector<RimSummaryCase*>    allSummaryCases();
    void                            setName(const QString& name);
    QString                         name() const;

private:
    caf::PdmFieldHandle*            userDescriptionField() override;
    void                            updateReferringCurveSets() const;
    QString                         nameAndItemCount() const;

private:
    caf::PdmChildArrayField<RimSummaryCase*> m_cases;
    caf::PdmField<QString>                   m_name;
    caf::PdmProxyValueField<QString>         m_nameAndItemCount;
};
