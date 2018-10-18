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

#include <QPointer>

class RigSimWellData;
class RimEclipseResultCase;
class RimFlowDiagSolution;
class RimSimWellInView;
class RimTofAccumulatedPhaseFractionsPlot;
class RimTotalWellAllocationPlot;
class RimWellAllocationPlotLegend;
class RimWellLogPlot;
class RimWellLogTrack;
class RiuWellAllocationPlot;

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
class RimWellAllocationPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;
public:
    enum FlowType { ACCUMULATED, INFLOW};

public:
    RimWellAllocationPlot();
    ~RimWellAllocationPlot() override;

    void                                            setFromSimulationWell(RimSimWellInView* simWell);

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    QWidget*                                viewWidget() override;
    void                                    zoomAll() override;

    RimWellLogPlot*                                 accumulatedWellFlowPlot();
    RimTotalWellAllocationPlot*                     totalWellFlowPlot();
    RimTofAccumulatedPhaseFractionsPlot*            tofAccumulatedPhaseFractionsPlot();
    caf::PdmObject*                                 plotLegend();
    RimEclipseResultCase*                           rimCase();
    int                                             timeStep();

    QString                                         wellName() const;

    void                                            removeFromMdiAreaAndDeleteViewWidget();

    void                                            showPlotLegend(bool doShow);
protected:
    // Overridden PDM methods
    caf::PdmFieldHandle*                    userDescriptionField() override { return &m_userName; }
    void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    std::set<QString>                               findSortedWellNames();

    QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    QImage                                  snapshotWindowContent() override;


    void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                                    onLoadDataAndUpdate() override;

private:
    void                                            updateFromWell();

    std::map<QString, const std::vector<double> *>  findRelevantTracerCellFractions(const RigSimWellData* simWellData);

    void                                            updateWellFlowPlotXAxisTitle(RimWellLogTrack* plotTrack);

    void                                            addStackedCurve(const QString& tracerName, 
                                                                    const std::vector<double>& depthValues, 
                                                                    const std::vector<double>& accFlow, 
                                                                    RimWellLogTrack* plotTrack);
    
    void                                            updateWidgetTitleWindowTitle();
    static QString                                  wellStatusTextForTimeStep(const QString& wellName, const RimEclipseResultCase* eclipseResultCase, size_t timeStep);

    // RimViewWindow overrides

    QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    void                                    deleteViewWidget() override; 

    cvf::Color3f                                    getTracerColor(const QString& tracerName);

    void                                            updateFormationNamesData() const;

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<bool>                             m_branchDetection;

    caf::PdmPtrField<RimEclipseResultCase*>         m_case;
    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmPtrField<RimFlowDiagSolution*>          m_flowDiagSolution;
    caf::PdmField<bool>                             m_groupSmallContributions;
    caf::PdmField<double>                           m_smallContributionsThreshold;
    caf::PdmField<caf::AppEnum<FlowType> >          m_flowType;

    QPointer<RiuWellAllocationPlot>                 m_wellAllocationPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_accumulatedWellFlowPlot;
    caf::PdmChildField<RimTotalWellAllocationPlot*> m_totalWellAllocationPlot;
    caf::PdmChildField<RimWellAllocationPlotLegend*> m_wellAllocationPlotLegend;
    caf::PdmChildField<RimTofAccumulatedPhaseFractionsPlot*> m_tofAccumulatedPhaseFractionsPlot;
};
