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

#include "RiaSummaryCurveAnalyzer.h"
#include "RifEclipseSummaryAddress.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QString>

#include <set>

class RimSummaryCase;
class RifSummaryReaderInterface;

//==================================================================================================
///
//==================================================================================================
class RimSummaryPlotSourceStepping : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum SourceSteppingType
    {
        Y_AXIS,
        X_AXIS,
        UNION_X_Y_AXIS
    };

public:
    RimSummaryPlotSourceStepping();

    void setSourceSteppingType(SourceSteppingType sourceSteppingType);

    void applyNextCase();
    void applyPrevCase();

    void applyNextQuantity();
    void applyPrevQuantity();

    void applyNextOtherIdentifier();
    void applyPrevOtherIdentifier();

    std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();

private:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                bool*                      useOptionsOnly) override;

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue,
                                  const QVariant& newValue) override;

    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName,
                                       caf::PdmUiEditorAttribute* attribute) override;

private:
    RifSummaryReaderInterface* summaryReader() const;
    RimSummaryCase*            singleSummaryCase() const;
    void                       updateUiFromCurves();
    caf::PdmValueField*        fieldToModify();

    std::set<RifEclipseSummaryAddress> allAddressesUsedInCurveCollection() const;
    std::set<RimSummaryCase*>          allSummaryCasesUsedInCurveCollection() const;

    bool isXAxisStepping() const;
    bool isYAxisStepping() const;

    RiaSummaryCurveAnalyzer* analyzerForReader(RifSummaryReaderInterface* reader);

    void modifyCurrentIndex(caf::PdmValueField* valueField, int indexOffset);

private:
    caf::PdmPtrField<RimSummaryCase*> m_summaryCase;
    caf::PdmField<QString>            m_wellName;
    caf::PdmField<QString>            m_wellGroupName;
    caf::PdmField<int>                m_region;
    caf::PdmField<QString>            m_quantity;
    caf::PdmField<QString>            m_placeholderForLabel;
    SourceSteppingType                m_sourceSteppingType;

    std::pair<RifSummaryReaderInterface*, RiaSummaryCurveAnalyzer> m_curveAnalyzerForReader;
};
