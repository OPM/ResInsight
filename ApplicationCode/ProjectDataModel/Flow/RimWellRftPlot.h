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
class RiuWellRftPlot;
class RimWellLogTrack;
class RimTofAccumulatedPhaseFractionsPlot;
class RigSingleWellResultsData;

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
public:
    enum FlowType { ACCUMULATED, INFLOW};

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

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    //std::set<QString>                               findSortedWellNames();

    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual QImage                                  snapshotWindowContent() override;


    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    void                                            calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);
    void                                            calculateValueOptionsForObservedData(QList<caf::PdmOptionItemInfo>& options, int level);
    void                                            calculateValueOptionsForTimeSteps(QList<caf::PdmOptionItemInfo>& options);

    void                                            updateFromWell();
    void                                            updateWidgetTitleWindowTitle();

    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

    cvf::Color3f                                    getTracerColor(const QString& tracerName);

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<QString>                          m_wellName;
    caf::PdmField<std::vector<QString>>             m_selectedSources;
    caf::PdmField<std::vector<QString>>             m_selectedTimeSteps;

    QPointer<RiuWellRftPlot>                        m_wellLogPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_wellLogPlot;
};
