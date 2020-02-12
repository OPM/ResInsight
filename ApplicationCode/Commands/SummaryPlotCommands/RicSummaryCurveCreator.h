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

#include "RifEclipseSummaryAddress.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RiuSummaryCurveDefSelectionEditor.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <memory>

#define OBSERVED_DATA_AVALUE_POSTFIX "_OBSDATA"

namespace caf
{
class PdmObject;
};

class RimSummaryCase;
class RimSummaryCurveAutoName;
class RimSummaryPlot;
class RiaSummaryCurveDefinition;
class RimEnsembleCurveSet;

//==================================================================================================
///
///
//==================================================================================================
class RicSummaryCurveCreator : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    static const QString CONFIGURATION_NAME;

private:
    typedef caf::AppEnum<RimSummaryCurveAppearanceCalculator::CurveAppearanceType> AppearanceTypeAppEnum;

public:
    RicSummaryCurveCreator();
    ~RicSummaryCurveCreator() override;

    RimSummaryPlot* previewPlot() const;
    void            updateFromSummaryPlot( RimSummaryPlot*                     targetPlot,
                                           const std::vector<caf::PdmObject*>& defaultSources = std::vector<caf::PdmObject*>() );

    QWidget* addressSelectionWidget( QWidget* parent );

    bool isCloseButtonPressed() const;
    void clearCloseButton();
    void updateCurveNames();

private:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    void syncPreviewCurvesFromUiSelection();
    void updatePreviewCurvesFromCurveDefinitions( const std::set<RiaSummaryCurveDefinition>& allCurveDefsToDisplay,
                                                  const std::set<RiaSummaryCurveDefinition>& curveDefsToAdd,
                                                  const std::set<RimSummaryCurve*>&          curvesToDelete,
                                                  const std::set<RimEnsembleCurveSet*>&      curveSetsToDelete );

    void        populateCurveCreator( const RimSummaryPlot& sourceSummaryPlot );
    void        updateTargetPlot();
    static void copyCurveAndAddToPlot( const RimSummaryCurve* curve, RimSummaryPlot* plot, bool forceVisible = false );
    static void copyEnsembleCurveAndAddToCurveSet( const RimSummaryCurve* curve,
                                                   RimEnsembleCurveSet*   curveSet,
                                                   bool                   forceVisible = false );
    void        setDefaultCurveSelection( const std::vector<caf::PdmObject*>& defaultCases );

    void resetAllFields();
    void initCurveAppearanceCalculator( RimSummaryCurveAppearanceCalculator& curveAppearanceCalc );
    void applyAppearanceToAllPreviewCurves();
    void updateAppearanceEditor();
    void createNewPlot();
    bool isObservedData( RimSummaryCase* sumCase ) const;

    static RimSummaryCase* calculatedSummaryCase();
    void                   selectionEditorFieldChanged();

    void proxyEnablePlotAutoTitle( const bool& enable );
    bool proxyPlotAutoTitle() const;
    void setInitialCurveVisibility( const RimSummaryPlot* targetPlot );

private:
    caf::PdmPtrField<RimSummaryPlot*> m_targetPlot;

    std::unique_ptr<RimSummaryPlot> m_previewPlot;

    caf::PdmProxyValueField<bool> m_useAutoPlotTitleProxy;

    caf::PdmField<bool>                  m_useAutoAppearanceAssignment;
    caf::PdmField<bool>                  m_appearanceApplyButton;
    caf::PdmField<AppearanceTypeAppEnum> m_caseAppearanceType;
    caf::PdmField<AppearanceTypeAppEnum> m_variableAppearanceType;
    caf::PdmField<AppearanceTypeAppEnum> m_wellAppearanceType;
    caf::PdmField<AppearanceTypeAppEnum> m_groupAppearanceType;
    caf::PdmField<AppearanceTypeAppEnum> m_regionAppearanceType;

    caf::PdmChildField<RimSummaryCurveAutoName*> m_curveNameConfig;

    caf::PdmField<bool> m_okButtonField;
    caf::PdmField<bool> m_applyButtonField;
    caf::PdmField<bool> m_closeButtonField;

    std::unique_ptr<RiuSummaryCurveDefSelectionEditor> m_summaryCurveSelectionEditor;
};
