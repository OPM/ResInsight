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
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimEnsembleCurveSet;

class RimSummaryPlotManager : public QObject, public caf::PdmObject, public caf::SelectionChangedReceiver

{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotManager();

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
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void appendCurves();
    void replaceCurves();
    void createNewPlot();

    bool eventFilter( QObject* obj, QEvent* event ) override;

    std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryCaseCollection*>> dataSources() const;

private:
    void                                      updateFromSelection();
    std::set<RifEclipseSummaryAddress>        filteredAddresses();
    static std::set<RifEclipseSummaryAddress> addressesForSource( caf::PdmObject* summarySource );

    static RimEnsembleCurveSet* createCurveSet( RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& addr );
    static void                 appendCurvesToPlot( RimSummaryPlot*                               summaryPlot,
                                                    const std::set<RifEclipseSummaryAddress>&     allAddressesInCase,
                                                    const std::vector<RimSummaryCase*>&           summaryCases,
                                                    const std::vector<RimSummaryCaseCollection*>& ensembles );

    static void setFocusToEditorWidget( caf::PdmUiFieldHandle* uiFieldHandle );

    void appendCurvesToPlot( RimSummaryPlot* destinationPlot );

private:
    caf::PdmPtrField<RimSummaryPlot*> m_summaryPlot;

    caf::PdmField<QString>              m_curveFilterText;
    caf::PdmField<std::vector<QString>> m_curveCandidates;

    caf::PdmField<bool> m_includeHistoryCurves;
    caf::PdmField<bool> m_includeDiffCurves;

    caf::PdmField<bool> m_pushButtonReplace;
    caf::PdmField<bool> m_pushButtonNew;
    caf::PdmField<bool> m_pushButtonAppend;

    caf::PdmField<QString> m_labelA;
    caf::PdmField<QString> m_labelB;
};
