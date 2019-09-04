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

#include "RimViewWindow.h"
#include "RimWellLogPlot.h"

#include "RifDataSourceForRftPltQMetaType.h"
#include "RiuQwtSymbol.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cvfCollection.h"

#include <QDate>
#include <QMetaType>
#include <QPointer>

#include <map>
#include <set>
#include <utility>

class RimEclipseCase;
class RimEclipseResultCase;
class RimWellLogCurve;
class RimWellLogFileChannel;
class RimWellPath;
class RiuWellRftPlot;
class RigEclipseCaseData;
class RiaRftPltCurveDefinition;
class RifDataSourceForRftPlt;
class RifEclipseRftAddress;

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
    void           setSimWellOrWellPathName(const QString& currWellName);

    int branchIndex() const;

    void applyInitialSelections();

    static const char* plotNameFormatString();

	void deleteCurvesAssosicatedWithObservedData(const RimObservedFmuRftData* observedFmuRftData);

protected:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;

    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void onLoadDataAndUpdate() override;
    void initAfterRead() override;

private:
    void calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);
    void updateEditorsFromCurves();
    void syncCurvesFromUiSelection();
    void assignWellPathToExtractionCurves();

    RimObservedFmuRftData* findObservedFmuData(const QString& wellPathName, const QDateTime& timeStep) const;

    std::set<RiaRftPltCurveDefinition> selectedCurveDefs() const;
    std::set<RiaRftPltCurveDefinition> curveDefsFromCurves() const;

    void updateCurvesInPlot(const std::set<RiaRftPltCurveDefinition>& allCurveDefs,
                            const std::set<RiaRftPltCurveDefinition>& curveDefsToAdd,
                            const std::set<RimWellLogCurve*>&         curvesToDelete);

    std::vector<RifDataSourceForRftPlt> selectedSourcesExpanded() const;

    // RimViewWindow overrides

    QWidget* createViewWidget(QWidget* mainWindowParent) override;

    void applyCurveAppearance(RimWellLogCurve* newCurve);

    void    updateFormationsOnPlot() const;
    QString associatedSimWellName() const;

    static RiuQwtSymbol::PointSymbolEnum statisticsCurveSymbolFromAddress(const RifEclipseRftAddress& address);
    static RiuQwtSymbol::LabelPosition   statisticsLabelPosFromAddress(const RifEclipseRftAddress& address);

    void defineCurveColorsAndSymbols(const std::set<RiaRftPltCurveDefinition>& allCurveDefs);

    void onDepthTypeChanged() override;
private:
    caf::PdmField<QString> m_wellPathNameOrSimWellName;
    caf::PdmField<int>     m_branchIndex;
    caf::PdmField<bool>    m_branchDetection;
    caf::PdmField<bool>    m_showStatisticsCurves;
    caf::PdmField<bool>    m_showEnsembleCurves;

    caf::PdmField<std::vector<RifDataSourceForRftPlt>> m_selectedSources;

    caf::PdmField<std::vector<QDateTime>> m_selectedTimeSteps;

    caf::PdmField<bool>                 m_showPlotTitle_OBSOLETE;
    caf::PdmChildField<RimWellLogPlot*> m_wellLogPlot_OBSOLETE;

	std::map<RifDataSourceForRftPlt, cvf::Color3f>     m_dataSourceColors;
    std::map<QDateTime, RiuQwtSymbol::PointSymbolEnum> m_timeStepSymbols;
    bool m_isOnLoad;
};
