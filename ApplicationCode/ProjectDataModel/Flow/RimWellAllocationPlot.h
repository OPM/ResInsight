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

class RimEclipseResultCase;
class RimEclipseWell;
class RimFlowDiagSolution;
class RimTotalWellAllocationPlot;
class RimWellLogPlot;
class RiuWellAllocationPlot;
class RimWellLogTrack;

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
    RimWellAllocationPlot();
    virtual ~RimWellAllocationPlot();

    void                                            setFromSimulationWell(RimEclipseWell* simWell);

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    void                                            loadDataAndUpdate();

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;

    RimWellLogPlot*                                 accumulatedWellFlowPlot();
    RimTotalWellAllocationPlot*                     totalWellFlowPlot();


    QString                                         wellName() const;

    void                                            removeFromMdiAreaAndDeleteViewWidget();

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual QImage                                  snapshotWindowContent() override;

private:
    void                                            updateFromWell();

    void                                            addStackedCurve(const QString& tracerName, 
                                                                    const std::vector<double>& connNumbers, 
                                                                    const std::vector<double>& accFlow, 
                                                                    RimWellLogTrack* plotTrack);
    
    void                                            updateWidgetTitleWindowTitle();

    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmPtrField<RimEclipseResultCase*>         m_case;
    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmPtrField<RimFlowDiagSolution*>          m_flowDiagSolution;

    QPointer<RiuWellAllocationPlot>                 m_wellAllocationPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_accumulatedWellFlowPlot;
    caf::PdmChildField<RimTotalWellAllocationPlot*> m_totalWellAllocationPlot;
};
