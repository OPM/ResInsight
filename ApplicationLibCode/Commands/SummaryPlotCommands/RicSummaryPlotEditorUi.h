/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuSummaryVectorSelectionWidgetCreator.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <memory>

#define OBSERVED_DATA_AVALUE_POSTFIX "_OBSDATA"

namespace caf
{
class PdmObject;
};

class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryMultiPlot;
class RimSummaryPlot;
class RiaSummaryCurveDefinition;
class RimEnsembleCurveSet;
class RimCalculatedSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RicSummaryPlotEditorUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    static const QString CONFIGURATION_NAME;

public:
    RicSummaryPlotEditorUi();
    ~RicSummaryPlotEditorUi() override;

    RimSummaryPlot* previewPlot() const;
    void            updateFromSummaryPlot( RimSummaryPlot*                     targetPlot,
                                           const std::vector<caf::PdmObject*>& defaultSources = std::vector<caf::PdmObject*>() );

    void updateFromSummaryMultiPlot( RimSummaryMultiPlot*                summaryMultiPlot,
                                     const std::vector<caf::PdmObject*>& defaultSources = std::vector<caf::PdmObject*>() );

    QWidget* addressSelectionWidget( QWidget* parent );

    bool isCloseButtonPressed() const;
    void clearCloseButton();
    void updateCurveNames();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void syncPreviewCurvesFromUiSelection();
    void updatePreviewCurvesFromCurveDefinitions( const std::set<RiaSummaryCurveDefinition>& allCurveDefsToDisplay,
                                                  const std::set<RiaSummaryCurveDefinition>& curveDefsToAdd,
                                                  const std::set<RimSummaryCurve*>&          curvesToDelete,
                                                  const std::set<RimEnsembleCurveSet*>&      curveSetsToDelete );

    void        populateCurveCreator( const RimSummaryPlot& sourceSummaryPlot );
    void        updateTargetPlot();
    static void copyCurveAndAddToPlot( const RimSummaryCurve* curve, RimSummaryPlot* plot, bool forceVisible = false );
    void        setDefaultCurveSelection( const std::vector<caf::PdmObject*>& defaultCases );

    void resetAllFields();
    void applyAppearanceToAllPreviewCurves();
    void createNewPlot();
    bool isObservedData( RimSummaryCase* sumCase ) const;

    void selectionEditorFieldChanged();
    void setInitialCurveVisibility( const RimSummaryPlot* targetPlot );

private:
    caf::PdmPtrField<RimSummaryPlot*> m_targetPlot;

    std::unique_ptr<RimSummaryPlot> m_previewPlot;

    caf::PdmField<bool> m_okButtonField;
    caf::PdmField<bool> m_applyButtonField;
    caf::PdmField<bool> m_closeButtonField;

    std::unique_ptr<RiuSummaryVectorSelectionWidgetCreator> m_summaryCurveSelectionEditor;

    caf::PdmPointer<RimSummaryMultiPlot> m_plotContainer;
};
