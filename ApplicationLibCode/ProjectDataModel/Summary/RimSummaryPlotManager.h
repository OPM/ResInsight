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

class RifEclipseSummaryAddress;
class RimSummaryPlot;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimEnsembleCurveSet;
class RimSummaryCurve;

class RimSummaryPlotManager : public QObject, public caf::PdmObject, public caf::SelectionChangedReceiver

{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotManager();

    void setFocusToFilterText();

private:
    void appendCurves();
    void replaceCurves();
    void createNewPlot();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    // Override eventFilter to be able to track key events from QDockWidget
    bool eventFilter( QObject* obj, QEvent* event ) override;

    void updateCurveCandidates();
    void updateDataSourceCandidates();

    std::vector<std::pair<QString, caf::PdmObject*>> findDataSourceCandidates() const;

    std::set<RifEclipseSummaryAddress> computeFilteredAddresses( const QStringList&                        textFilters,
                                                                 const std::set<RifEclipseSummaryAddress>& sourceAddresses );

    std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryCaseCollection*>> allDataSourcesInProject() const;

    void                               updateUiFromSelection();
    std::set<RifEclipseSummaryAddress> filteredAddresses();
    void                               appendCurvesToPlot( RimSummaryPlot* destinationPlot );
    void                               updateFilterTextHistory();

    // Static helper functions
    static std::set<RifEclipseSummaryAddress> addressesForSource( caf::PdmObject* summarySource );

    static RimEnsembleCurveSet* createCurveSet( RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& addr );
    static RimSummaryCurve*     createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr );

    static void appendCurvesToPlot( RimSummaryPlot*                               summaryPlot,
                                    const std::set<RifEclipseSummaryAddress>&     addresses,
                                    const std::vector<RimSummaryCase*>&           summaryCases,
                                    const std::vector<RimSummaryCaseCollection*>& ensembles );

    static void setFocusToEditorWidget( caf::PdmUiFieldHandle* uiFieldHandle );

    void splitIntoAddressAndDataSourceFilters( QStringList& addressFilters, QStringList& dataSourceFilters ) const;
    void findFilteredSummaryCasesAndEnsembles( std::vector<RimSummaryCase*>&           summaryCases,
                                               std::vector<RimSummaryCaseCollection*>& ensembles ) const;

    static QString curveFilterRecentlyUsedRegistryKey();

private:
    caf::PdmPtrField<RimSummaryPlot*> m_summaryPlot;

    caf::PdmField<QString>              m_filterText;
    caf::PdmField<std::vector<QString>> m_addressCandidates;
    caf::PdmField<std::vector<QString>> m_dataSourceCandidates;

    caf::PdmField<bool> m_includeDiffCurves;

    caf::PdmField<bool> m_pushButtonReplace;
    caf::PdmField<bool> m_pushButtonNewPlot;
    caf::PdmField<bool> m_pushButtonAppend;

    caf::PdmField<QString> m_labelA;
    caf::PdmField<QString> m_labelB;
};
