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

#include "RimDataSourceForRftPlt.h"
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
#include "RifEclipseRftAddress.h"

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
    ~RimWellPltPlot() override;

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    QWidget*                                viewWidget() override;
    void                                    zoomAll() override;

    RimWellLogPlot*                                 wellLogPlot() const;

    void                                            setCurrentWellName(const QString& currWellName);
    
    static const char*                              plotNameFormatString();


protected:
    // Overridden PDM methods
    caf::PdmFieldHandle*                    userDescriptionField() override { return &m_userName; }
    void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

    QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    void                                            calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);

    QImage                                  snapshotWindowContent() override;

    void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    void                                    onLoadDataAndUpdate() override;

    void                                    initAfterRead() override;
    void                                    setupBeforeSave() override;
    void                                            initAfterLoad();

private:

    void                                            syncSourcesIoFieldFromGuiField();
    void                                            syncCurvesFromUiSelection();

    std::set<RiaRftPltCurveDefinition>              selectedCurveDefs() const;
    void                                            addStackedCurve(const QString& tracerName,
                                                                    const std::vector<double>& depthValues,
                                                                    const std::vector<double>& accFlow,
                                                                    RimWellLogTrack* plotTrack,
                                                                    cvf::Color3f color,
                                                                    int curveGroupId,
                                                                    bool doFillCurve);

    std::vector<RifDataSourceForRftPlt>             selectedSourcesExpanded() const;

    // RimViewWindow overrides

    void                                            updateWidgetTitleWindowTitle();
    QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    void                                    deleteViewWidget() override; 

    void                                            setPlotXAxisTitles(RimWellLogTrack* plotTrack);

    void                                            updateFormationsOnPlot() const;

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<QString>                          m_wellPathName;

    caf::PdmField<std::vector<RifDataSourceForRftPlt>>   m_selectedSources;
    caf::PdmChildArrayField<RimDataSourceForRftPlt*>         m_selectedSourcesForIo;

    caf::PdmField<std::vector<QDateTime>>           m_selectedTimeSteps;

    QPointer<RiuWellPltPlot>                        m_wellLogPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_wellLogPlot;

    caf::PdmField<bool>                  m_useStandardConditionCurves;
    caf::PdmField<bool>                  m_useReservoirConditionCurves;
    caf::PdmField<std::vector<caf::AppEnum<FlowPhase>>> m_phases;

    bool                                            m_doInitAfterLoad;
    bool                                            m_isOnLoad;
};
