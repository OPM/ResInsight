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

#include "RimRftAddress.h"
#include "RifDataSourceForRftPltQMetaType.h"
#include "RimPlotCurve.h"
#include "RimWellPlotTools.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafPdmChildArrayField.h"
#include "cvfCollection.h"

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

    static const char PLOT_NAME_QFORMAT_STRING[];

public:
    RimWellPltPlot();
    virtual ~RimWellPltPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;

    RimWellLogPlot*                                 wellLogPlot() const;

    void                                            setCurrentWellName(const QString& currWellName);
    QString                                         currentWellName() const;
    
    static const char*                              plotNameFormatString();

    //void                                            applyInitialSelections();

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual QImage                                  snapshotWindowContent() override;


    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);
    virtual void                                    onLoadDataAndUpdate() override;

    virtual void                                    initAfterRead() override;
    virtual void                                    setupBeforeSave() override;
    void                                            initAfterLoad();
    void                                            syncSourcesIoFieldFromGuiField();

private:
    void                                            updateTimeStepsToAddresses(const std::vector<RifDataSourceForRftPlt>& addressesToKeep);
    void                                            calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);
    void                                            calculateValueOptionsForTimeSteps(const QString& wellPathNameOrSimWellName, QList<caf::PdmOptionItemInfo>& options);

    void                                            updateWidgetTitleWindowTitle();

    void                                            syncCurvesFromUiSelection();

    std::set<std::pair<RifDataSourceForRftPlt, QDateTime>> selectedCurveDefs() const;
    std::set<std::pair<RifDataSourceForRftPlt, QDateTime>> curveDefsFromCurves() const;
    std::pair<RifDataSourceForRftPlt, QDateTime>         curveDefFromCurve(const RimWellLogCurve* curve) const;
    void                                            addStackedCurve(const QString& tracerName,
                                                                    const std::vector<double>& depthValues,
                                                                    const std::vector<double>& accFlow,
                                                                    RimWellLogTrack* plotTrack,
                                                                    cvf::Color3f color,
                                                                    int curveGroupId,
                                                                    bool doFillCurve);

    bool                                            isOnlyGridSourcesSelected() const;
    bool                                            isAnySourceAddressSelected(const std::set<RifDataSourceForRftPlt>& addresses) const;
    std::vector<RifDataSourceForRftPlt>                  selectedSources() const;
    std::vector<RifDataSourceForRftPlt>                  selectedSourcesAndTimeSteps() const;

    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

    //void                                            applyCurveAppearance(RimWellLogCurve* newCurve);
    void                                            updateSelectedTimeStepsFromSelectedSources();
    void                                            setPlotXAxisTitles(RimWellLogTrack* plotTrack);
    std::vector<RimEclipseCase*>                    eclipseCases() const;

    void                                            updateFormationsOnPlot() const;

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<QString>                          m_wellPathName;

    caf::PdmField<std::vector<RifDataSourceForRftPlt>>   m_selectedSources;
    caf::PdmChildArrayField<RimRftAddress*>         m_selectedSourcesForIo;

    caf::PdmField<std::vector<QDateTime>>           m_selectedTimeSteps;

    QPointer<RiuWellPltPlot>                        m_wellLogPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_wellLogPlot;

    std::map<QDateTime, std::set<RifDataSourceForRftPlt>>    m_timeStepsToAddresses;

    caf::PdmField<caf::AppEnum<FlowType>>               m_phaseSelectionMode;
    caf::PdmField<std::vector<caf::AppEnum<FlowPhase>>> m_phases;

    bool                                            m_doInitAfterLoad;
    bool                                            m_isOnLoad;
};
