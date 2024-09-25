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
class RimSummaryEnsemble;
class RimEnsembleCurveSet;
class RimSummaryCurve;
class RimMultiPlot;
class RimPlot;

class RimSummaryPlotManager : public QObject, public caf::PdmObject, public caf::SelectionChangedReceiver

{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlotManager();

    void setFocusToFilterText();
    void resetDataSourceSelection();
    void onSummaryDataSourceHasChanged( const caf::SignalEmitter* emitter );

private:
    void appendCurves();
    void replaceCurves();
    void createNewPlot();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    // Override eventFilter to be able to track key events from QWidget
    bool eventFilter( QObject* obj, QEvent* event ) override;

    void updateCurveCandidates();

    std::vector<std::pair<QString, caf::PdmObject*>> findDataSourceCandidates() const;
    std::vector<QString>                             dataSourceDisplayNames() const;

    std::set<RifEclipseSummaryAddress> filteredAddresses();

    void updateUiFromSelection();
    void appendCurvesToPlot( RimSummaryPlot* destinationPlot );
    void updateFilterTextHistory();
    void updateProjectTreeAndRefresUi();
    void updateSelectionFromUiChange();

    QStringList extractDataSourceFilters() const;

    void findFilteredSummaryCasesAndEnsembles( std::vector<RimSummaryCase*>& summaryCases, std::vector<RimSummaryEnsemble*>& ensembles ) const;

    static void    setFocusToEditorWidget( caf::PdmUiFieldHandle* uiFieldHandle );
    static QString curveFilterRecentlyUsedRegistryKey();

private:
    caf::PdmPtrField<RimSummaryPlot*> m_summaryPlot;

    caf::PdmField<QString>              m_filterText;
    caf::PdmField<std::vector<QString>> m_addressCandidates;
    caf::PdmField<std::vector<QString>> m_selectedDataSources;

    caf::PdmField<bool> m_includeDiffCurves;

    caf::PdmField<bool> m_pushButtonReplace;
    caf::PdmField<bool> m_pushButtonNewPlot;
    caf::PdmField<bool> m_pushButtonAppend;

    caf::PdmField<bool> m_individualPlotPerVector;
    caf::PdmField<bool> m_individualPlotPerDataSource;
    caf::PdmField<bool> m_individualPlotPerObject;
    caf::PdmField<bool> m_createMultiPlot;

    caf::PdmField<QString> m_labelA;
    caf::PdmField<QString> m_labelB;

    QStringList m_previousDataSourceText;
};
