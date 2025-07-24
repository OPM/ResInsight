/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimWellLogPlot.h"

#include "RifDataSourceForRftPltQMetaType.h"

#include "RiuPlotCurveSymbol.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <map>
#include <set>
#include <utility>
#include <variant>

class RimEclipseCase;
class RimEclipseResultCase;
class RimRegularLegendConfig;
class RimWellLogCurve;
class RimWellLogChannel;
class RimWellPath;
class RimWellPathCollection;
class RiuWellRftPlot;
class RigEclipseCaseData;
class RiaRftPltCurveDefinition;
class RifDataSourceForRftPlt;
class RifEclipseRftAddress;
class RiuDraggableOverlayFrame;
class RimDataSourceForRftPlt;
class RiuPlotCurve;
class RimWellRftEnsembleCurveSet;

namespace cvf
{
class Color3f;
}

namespace caf
{
class PdmOptionItemInfo;
}

//==================================================================================================
///
///
//==================================================================================================
class RimWellRftPlot : public RimWellLogPlot
{
    CAF_PDM_HEADER_INIT;

    static const std::set<QString> PRESSURE_DATA_NAMES;
    static const char              PLOT_NAME_QFORMAT_STRING[];

public:
    RimWellRftPlot();
    ~RimWellRftPlot() override;

    const QString& simWellOrWellPathName() const;
    void           setSimWellOrWellPathName( const QString& currWellName );

    int branchIndex() const;

    std::variant<RimSummaryCase*, RimSummaryEnsemble*> dataSource() const;
    void applyInitialSelections( std::variant<RimSummaryCase*, RimSummaryEnsemble*> dataSource );

    static const char* plotNameFormatString();

    void deleteCurvesAssosicatedWithObservedData( const RimObservedFmuRftData* observedFmuRftData );

    bool showErrorBarsForObservedData() const;
    void onLegendDefinitionChanged();

    RimWellRftEnsembleCurveSet* findEnsembleCurveSet( RimSummaryEnsemble* ensemble ) const;

    void rebuildCurves();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptionsForSources() const;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;
    void setupBeforeSave() override;
    void initAfterRead() override;
    bool useUndoRedoForFieldChanged() override;
    void deleteViewWidget() override;

    std::map<QString, QStringList> findWellSources();
    void                           updateEditorsFromPreviousSelection();
    void                           setSelectedSourcesFromCurves();
    void                           syncCurvesFromUiSelection();
    void                           assignWellPathToExtractionCurves();
    void                           syncSourcesIoFieldFromGuiField();

    RimObservedFmuRftData* findObservedFmuData( const QString& wellPathName, const QDateTime& timeStep ) const;

    std::set<RiaRftPltCurveDefinition> selectedCurveDefs() const;
    std::set<RiaRftPltCurveDefinition> curveDefsFromCurves() const;

    void updateCurvesInPlot( const std::set<RiaRftPltCurveDefinition>& allCurveDefs,
                             const std::set<RiaRftPltCurveDefinition>& curveDefsToAdd,
                             const std::set<RimWellLogCurve*>&         curvesToDelete );

    std::vector<RifDataSourceForRftPlt> selectedSourcesExpanded() const;

    // RimViewWindow overrides

    void applyCurveAppearance( RimWellLogCurve* curve );
    void applyCurveColor( RimWellLogCurve* curve );

    void    updateFormationsOnPlot() const;
    QString associatedSimWellName() const;

    static RiuPlotCurveSymbol::PointSymbolEnum statisticsCurveSymbolFromAddress( const RifEclipseRftAddress& address );
    static RiuPlotCurveSymbol::LabelPosition   statisticsLabelPosFromAddress( const RifEclipseRftAddress& address );

    cvf::Color3f findCurveColor( RimWellLogCurve* curve );
    void         defineCurveColorsAndSymbols( const std::set<RiaRftPltCurveDefinition>& allCurveDefs );

    std::vector<RimSummaryEnsemble*> selectedEnsembles() const;
    void                             createEnsembleCurveSets();

    void detachAndDeleteLegendCurves();

private:
    friend class RimWellRftEnsembleCurveSet;

    caf::PdmField<QString> m_wellPathNameOrSimWellName;
    caf::PdmField<int>     m_branchIndex;
    caf::PdmField<bool>    m_branchDetection;
    caf::PdmField<bool>    m_showStatisticsCurves;
    caf::PdmField<bool>    m_showEnsembleCurves;
    caf::PdmField<bool>    m_showErrorInObservedData;

    caf::PdmField<std::vector<RifDataSourceForRftPlt>> m_selectedSources;
    caf::PdmChildArrayField<RimDataSourceForRftPlt*>   m_selectedSourcesForIo;
    caf::PdmField<std::vector<QDateTime>>              m_selectedTimeSteps;

    caf::PdmChildArrayField<RimWellRftEnsembleCurveSet*> m_ensembleCurveSets;
    caf::PdmPtrField<RimEclipseCase*>                    m_ensembleCurveSetEclipseCase;

    std::map<RimWellRftEnsembleCurveSet*, QPointer<RiuDraggableOverlayFrame>> m_ensembleLegendFrames;

    std::map<RifDataSourceForRftPlt, cvf::Color3f>           m_dataSourceColors;
    std::map<QDateTime, RiuPlotCurveSymbol::PointSymbolEnum> m_timeStepSymbols;
    bool                                                     m_isOnLoad;

    std::vector<RiuPlotCurve*> m_legendPlotCurves;

    caf::PdmChildField<RimWellLogPlot*> m_wellLogPlot_OBSOLETE;
    bool                                m_isInitialized = false;
};
