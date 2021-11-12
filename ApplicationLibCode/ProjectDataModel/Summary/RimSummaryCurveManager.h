////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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
#include "cafPdmPtrField.h"
#include "cafSelectionChangedReceiver.h"

class RimSummaryPlot;
class RifEclipseSummaryAddress;

class RimSummaryCurveManager : public QObject, public caf::PdmObject, public caf::SelectionChangedReceiver

{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveManager();

private:
    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void updateCurveCandidates();

    void insertFilteredAddressesInSet( const QStringList&                        curveFilters,
                                       const std::set<RifEclipseSummaryAddress>& allAddressesInCase,
                                       std::set<RifEclipseSummaryAddress>*       setToInsertFilteredAddressesIn,
                                       std::vector<bool>*                        usedFilters );

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void appendText();
    void replaceText();
    void clearText();

    bool eventFilter( QObject* obj, QEvent* event ) override;

private:
    void                                      updateFromSelection();
    static std::set<RifEclipseSummaryAddress> addressesForSource( caf::PdmObject* summarySource );

private:
    caf::PdmPtrField<RimSummaryPlot*> m_summaryPlot;

    caf::PdmField<QString>              m_curveFilterText;
    caf::PdmField<std::vector<QString>> m_curveCandidates;

    caf::PdmField<bool> m_includeHistoryCurves;
    caf::PdmField<bool> m_includeDiffCurves;

    caf::PdmField<bool> m_pushButtonReplace;
    caf::PdmField<bool> m_pushButtonClear;
    caf::PdmField<bool> m_pushButtonAppend;
};
