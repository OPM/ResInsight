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

class RiuWellAllocationPlot;
class RimEclipseWell;


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

    void                                            setSimulationWell(RimEclipseWell* simWell);

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    void                                            handleViewerDeletion();

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    objectToggleField() { return &m_showWindow; }
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                    setupBeforeSave() override;

    virtual QImage                                  snapshotWindowContent() override;

private:
    void                                            updateViewerWidget();
    void                                            updateViewerWidgetWindowTitle();
    void                                            deletePlotWidget();

private:
    caf::PdmField<bool>                             m_showWindow;

    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmPtrField<RimEclipseWell*>               m_simulationWell;

    QPointer<RiuWellAllocationPlot>                 m_wellAllocationPlot;
};
