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
#include "RigFlowDiagResultAddress.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cvfCollection.h"
#include "RimPlotCurve.h"

#include "RifWellRftAddress.h"
#include "RifWellRftAddressQMetaType.h"

#include <QPointer>
#include <QDate>
#include <QMetaType>
#include <set>
#include <map>


class RimEclipseCase;
class RimEclipseResultCase;
class RimWellLogCurve;
class RimWellLogFileChannel;
class RimWellLogPlot;
class RimWellPath;
class RiuWellPltPlot;
class RimWellLogTrack;


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
class RimWellPltPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

    enum FlowType { FLOW_TYPE_TOTAL, FLOW_TYPE_PHASE_SPLIT };
    enum FlowPhase { PHASE_NONE, PHASE_OIL, PHASE_GAS, PHASE_WATER, PHASE_TOTAL };

    static const QString OIL_CHANNEL_NAME;
    static const QString GAS_CHANNEL_NAME;
    static const QString WATER_CHANNEL_NAME;
    static const QString TOTAL_CHANNEL_NAME;

    static const std::set<QString> FLOW_DATA_NAMES;
    static const char PLOT_NAME_QFORMAT_STRING[];

public:
    RimWellPltPlot();
    virtual ~RimWellPltPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    virtual void                                    loadDataAndUpdate() override;

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;

    RimWellLogPlot*                                 wellLogPlot() const;

    void                                            setCurrentWellName(const QString& currWellName);
    QString                                         currentWellName() const;

    static bool                                     hasFlowData(const RimWellLogFile* wellLogFile);
    static bool                                     isFlowChannel(RimWellLogFileChannel* channel);
    static bool                                     hasFlowData(RimEclipseResultCase* gridCase);
    static bool                                     hasFlowData(RimWellPath* wellPath);
    static const char*                              plotNameFormatString();

    //void                                            applyInitialSelections();

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual QImage                                  snapshotWindowContent() override;


    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

private:
    void                                            addTimeStepToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                                                     const std::pair<QDateTime, std::set<RifWellRftAddress>>& timeStepToAdd);
    void                                            addTimeStepsToMap(std::map<QDateTime, std::set<RifWellRftAddress>>& destMap,
                                                                      const std::map<QDateTime, std::set<RifWellRftAddress>>& timeStepsToAdd);
    void                                            calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);
    void                                            calculateValueOptionsForTimeSteps(const QString& wellName, QList<caf::PdmOptionItemInfo>& options);

    void                                            updateEditorsFromCurves();
    void                                            updateWidgetTitleWindowTitle();

    void                                            syncCurvesFromUiSelection();

    std::vector<RimWellLogFile*>                    wellLogFilesContainingFlow(const QString& wellName) const;
    std::vector<RimWellLogFileChannel*>             getFlowChannelsFromWellFile(const RimWellLogFile* wellLogFile) const;

    RimWellPath*                                    wellPathFromWellLogFile(const RimWellLogFile* wellLogFile) const;
    
    //std::vector<std::tuple<RimEclipseResultCase*, bool, bool>> eclipseCasesForWell(const QString& wellName) const;
    //std::vector<RimEclipseResultCase*>              gridCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const;
    //std::vector<RimEclipseResultCase*>              rftCasesFromEclipseCases(const std::vector<std::tuple<RimEclipseResultCase*, bool, bool>>& eclipseCasesTuple) const;
    //std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsFromRftCase(RimEclipseResultCase* gridCase) const;
    //std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsFromGridCase(RimEclipseCase* gridCase) const;
    std::map<QDateTime, std::set<RifWellRftAddress>> timeStepsFromWellLogFile(RimWellLogFile* wellLogFile) const;
    std::map<QDateTime, std::set<RifWellRftAddress>> adjacentTimeSteps(const std::vector<std::pair<QDateTime, std::set<RifWellRftAddress>>>& allTimeSteps, 
                                                                       const std::pair<QDateTime, std::set<RifWellRftAddress>>& searchTimeStepPair);
    static bool                                      mapContainsTimeStep(const std::map<QDateTime, std::set<RifWellRftAddress>>& map, const QDateTime& timeStep);

    std::set<std::pair<RifWellRftAddress, QDateTime>> selectedCurveDefs() const;
    std::set<std::pair<RifWellRftAddress, QDateTime>> curveDefsFromCurves() const;
    std::pair<RifWellRftAddress, QDateTime>         curveDefFromCurve(const RimWellLogCurve* curve) const;
    void                                            updateCurvesInPlot(const std::set<std::pair<RifWellRftAddress, QDateTime>>& curveDefs);
    void                                            addStackedCurve(const QString& tracerName,
                                                                    const std::vector<double>& depthValues,
                                                                    const std::vector<double>& accFlow,
                                                                    RimWellLogTrack* plotTrack);

    bool                                            isOnlyGridSourcesSelected() const;
    bool                                            isAnySourceAddressSelected(const std::set<RifWellRftAddress>& addresses) const;
    std::vector<RifWellRftAddress>                  selectedSources() const;

    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

    //void                                            applyCurveAppearance(RimWellLogCurve* newCurve);
    void                                            updateSelectedTimeStepsFromSelectedSources();
    static FlowPhase                                flowPhaseFromChannelName(const QString& channelName);
    void                                            setPlotXAxisTitles(RimWellLogTrack* plotTrack);
    std::vector<RimEclipseCase*>                    eclipseCases() const;

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<int>                              m_branchIndex;
    caf::PdmField<std::vector<RifWellRftAddress>>   m_selectedSources;
    
    caf::PdmField<std::vector<QDateTime>>           m_selectedTimeSteps;

    QPointer<RiuWellPltPlot>                        m_wellLogPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_wellLogPlot;

    std::map<QDateTime, std::set<RifWellRftAddress>>    m_timeStepsToAddresses;

    caf::PdmField<caf::AppEnum<FlowType>>               m_phaseSelectionMode;
    caf::PdmField<std::vector<caf::AppEnum<FlowPhase>>> m_phases;
};
