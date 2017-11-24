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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimSummaryCase;
class RifSummaryReaderInterface;

//==================================================================================================
///
//==================================================================================================
class RimSummaryCurvesModifier : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurvesModifier();

    void applyNextIdentifier();
    void applyPreviousIdentifier();

private:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue,
                                  const QVariant& newValue) override;

private:
    RifSummaryReaderInterface* summaryReader() const;
    RimSummaryCase*            singleSummaryCase() const;
    QString                    wellName() const;
    void                       setWellName(const QString& wellName);
    void                       updateUiFromCurves();
    caf::PdmFieldHandle*       fieldToModify();

private:
    caf::PdmPtrField<RimSummaryCase*> m_summaryCase;
    caf::PdmField<QString>            m_wellName;
    caf::PdmField<QString>            m_wellGroupName;
    caf::PdmField<int>                m_region;
    caf::PdmField<QString>            m_quantity;

    caf::PdmProxyValueField<QString> m_wellNameProxy; // TODO: This is a test field for a list editor
};
