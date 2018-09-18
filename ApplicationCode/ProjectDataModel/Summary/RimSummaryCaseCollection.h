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

#include "RifEclipseSummaryAddress.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <vector>

class RimSummaryCase;

//==================================================================================================
///  
//==================================================================================================
class EnsembleParameter
{
public:
    enum Type { TYPE_NONE, TYPE_NUMERIC, TYPE_TEXT };

    QString                 name;
    Type                    type;
    std::vector<QVariant>   values;
    double                  minValue;
    double                  maxValue;

    EnsembleParameter() :
        type(TYPE_NONE),
        minValue(std::numeric_limits<double>::infinity()),
        maxValue(-std::numeric_limits<double>::infinity()) { }

    bool isValid() const { return !name.isEmpty() && type != TYPE_NONE; }
    bool isNumeric() const { return type == TYPE_NUMERIC; }
    bool isText() const { return type == TYPE_TEXT; }
};

//==================================================================================================
///  
//==================================================================================================
class RimSummaryCaseCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCaseCollection();
    virtual ~RimSummaryCaseCollection();

    void                            removeCase(RimSummaryCase* summaryCase);
    void                            deleteAllCases();
    void                            addCase(RimSummaryCase* summaryCase, bool updateCurveSets = true);
    virtual std::vector<RimSummaryCase*> allSummaryCases() const;
    void                            setName(const QString& name);
    QString                         name() const;
    bool                            isEnsemble() const;
    void                            setAsEnsemble(bool isEnsemble);
    virtual std::set<RifEclipseSummaryAddress> calculateUnionOfSummaryAddresses() const;
    EnsembleParameter               ensembleParameter(const QString& paramName) const;
    void                            calculateEnsembleParametersIntersectionHash();
    void                            clearEnsembleParametersHashes();

    void                            loadDataAndUpdate();

    static bool                     validateEnsembleCases(const std::vector<RimSummaryCase*> cases);
    bool                            operator<(const RimSummaryCaseCollection& rhs) const;
private:
    caf::PdmFieldHandle*            userDescriptionField() override;
    QString                         nameAndItemCount() const;
    void                            updateIcon();

    virtual void                    initAfterRead() override;
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

protected:
    virtual void                    onLoadDataAndUpdate();
    void                            updateReferringCurveSets();
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                            setNameAsReadOnly();

    caf::PdmChildArrayField<RimSummaryCase*> m_cases;

private:
    caf::PdmField<QString>                   m_name;
    caf::PdmProxyValueField<QString>         m_nameAndItemCount;
    caf::PdmField<bool>                      m_isEnsemble;

    size_t                                   m_commonAddressCount;      // if different address count among cases, set to 0
};
