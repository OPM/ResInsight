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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cvfCollection.h"
#include "RimWellRftAddress.h"
#include "RimPlotCurve.h"
#include <QPointer>
#include <QDate>
#include <QMetaType>

class RimEclipseResultCase;
class RimEclipseCase;
class RimEclipseWell;
class RimFlowDiagSolution;
class RimTotalWellAllocationPlot;
class RimWellLogPlot;
class RiuWellRftPlot;
class RimWellLogTrack;
class RimTofAccumulatedPhaseFractionsPlot;
class RigSingleWellResultsData;
class RimWellLogFileChannel;
class RimWellPath;
class RimWellLogCurve;
class RigWellPath;

namespace cvf {
    class Color3f;
}

namespace caf {
    class PdmOptionItemInfo;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellRftPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

    static const char PRESSURE_DATA_NAME[];
    static const char PLOT_NAME_QFORMAT_STRING[];

public:
    RimWellRftPlot();
    virtual ~RimWellRftPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    virtual void                                    loadDataAndUpdate() override;

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;

    RimWellLogPlot*                                 wellLogPlot() const;

    void                                            setCurrentWellName(const QString& currWellName);
    QString                                         currentWellName() const;

    static bool                                     hasPressureData(RimWellLogFile* wellLogFile);
    static bool                                     hasPressureData(RimWellLogFileChannel* channel);
    static bool                                     hasPressureData(RimEclipseResultCase* gridCase);
    static const char*                              plotNameFormatString();

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual QImage                                  snapshotWindowContent() override;


    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    void                                            calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);
    void                                            calculateValueOptionsForTimeSteps(const QString& wellName, QList<caf::PdmOptionItemInfo>& options);

    void                                            updateEditorsFromCurves();
    void                                            updateWidgetTitleWindowTitle();

    void                                            syncCurvesFromUiSelection();

    std::vector<RimWellPath*>                       wellPathsContainingPressure(const QString& wellName) const;
    std::vector<RimWellLogFileChannel*>             getPressureChannelsFromWellPath(const RimWellPath* wellPath) const;
    RimEclipseCase*                                 eclipseCaseFromCaseId(int caseId);

    RimWellPath*                                    wellPathForObservedData(const QString& wellName, const QDateTime& date) const;

    std::vector<std::tuple<RimEclipseResultCase*, bool, bool>> eclipseCasesContainingPressure(const QString& wellName) const;
    std::vector<RimEclipseResultCase*>              gridCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const;
    std::vector<RimEclipseResultCase*>              rftCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const;
    std::vector<QDateTime>                          timeStepsFromRftCase(RimEclipseResultCase* gridCase) const;
    std::vector<QDateTime>                          timeStepsFromGridCase(const RimEclipseCase* gridCase) const; 
    std::vector<QDateTime>                          timeStepsFromWellPaths(const std::vector<RimWellPath*> wellPaths) const;

    std::set<std::pair<RimWellRftAddress, QDateTime>> selectedCurveDefs() const;
    std::set<std::pair<RimWellRftAddress, QDateTime>> curveDefsFromCurves() const;
    std::pair<RimWellRftAddress, QDateTime>         curveDefFromCurve(const RimWellLogCurve* curve) const;
    void                                            updateCurvesInPlot(const std::set<std::pair<RimWellRftAddress, QDateTime>>& allCurveDefs,
                                                                       const std::set<std::pair<RimWellRftAddress, QDateTime>>& curveDefsToAdd,
                                                                       const std::set<RimWellLogCurve*>& curvesToDelete);
    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

    void                                            applyCurveAppearance(RimWellLogCurve* newCurve);

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<int>                              m_branchIndex;
    caf::PdmField<std::vector<RimWellRftAddress>>   m_selectedSources;
    
    caf::PdmField<std::vector<QDateTime>>           m_selectedTimeSteps;

    QPointer<RiuWellRftPlot>                        m_wellLogPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_wellLogPlot;
};
