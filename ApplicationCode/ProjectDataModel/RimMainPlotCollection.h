/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildField.h"

#include <QPointer>
#include <QDockWidget>


class RimWellLogPlotCollection;
class RimSummaryPlotCollection;
class RimSummaryPlot;
class RifReaderEclipseSummary;
class RimEclipseResultCase;


//==================================================================================================
///  
///  
//==================================================================================================
class RimMainPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimMainPlotCollection();
    virtual ~RimMainPlotCollection();

    RimWellLogPlotCollection* wellLogPlotCollection();
    RimSummaryPlotCollection* summaryPlotCollection();

    #if 0
    // Separate Window stuff 
    void showPlotWindow();
    void hidePlotWindow();

    void redrawAllPlots();
    void createDockWindowsForAllPlots();
    QMainWindow*            windowWithGraphPlots();
private:

    QDockWidget* dockWidgetFromPlot(RimSummaryPlot* graphPlot);
    void createPlotDockWidget(RimSummaryPlot* graphPlot);
    void eraseDockWidget(RimSummaryPlot* graphPlot);
    
private:
    QMainWindow*              m_plotManagerMainWindow; // Outer main Window
    QMainWindow*              m_plotMainWindow; // Inner main window

    std::vector<QPointer<QDockWidget> > m_plotViewDockWidgets; // ChildPlotWidgets
    #endif
protected:

    // Overridden PDM methods
    virtual caf::PdmFieldHandle* objectToggleField();
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    //virtual void initAfterRead();


    caf::PdmChildField<RimWellLogPlotCollection*> m_wellLogPlotCollection;
    caf::PdmChildField<RimSummaryPlotCollection*> m_summaryPlotCollection;

    caf::PdmField<bool> show;
};
