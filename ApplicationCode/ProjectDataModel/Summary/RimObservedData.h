/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimSummaryCase.h"

#include "RifEclipseSummaryAddress.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfObject.h"


//==================================================================================================
//
//==================================================================================================
class RimObservedData : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;
public:
    RimObservedData();

    QString         caseName() const override;
    void            updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) override;

    QString                                      identifierName() const;
    RifEclipseSummaryAddress::SummaryVarCategory summaryCategory() const;

    void                    updateMetaData();

protected:
    QString                 customWellName() const;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory> >  m_summaryCategory;
    caf::PdmField<QString>                                                      m_identifierName;
    caf::PdmField<bool>                                                         m_useCustomIdentifier;

    caf::PdmField<QString>                                                      m_importedSummaryData;
};
