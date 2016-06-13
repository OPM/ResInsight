/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"

#include <QPointer>

#include "RimViewWindow.h"

class RiuSummaryQwtPlot;
class RimSummaryCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryPlot();
    virtual ~RimSummaryPlot();

    void                                        setDescription(const QString& description);
    void                                        addCurve(RimSummaryCurve* curve);
    void                                        loadDataAndUpdate();
    void                                        handleViewerDeletion();

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                objectToggleField()    { return &m_showWindow; }
    virtual caf::PdmFieldHandle*                userDescriptionField() { return &m_userName; }
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                setupBeforeSave() override;

private:
    void                                        updateViewerWidget();
    void                                        detachAllCurves();
    void                                        deletePlotWidget();

    caf::PdmField<bool>                         m_showWindow;
    caf::PdmField<QString>                      m_userName;
    caf::PdmChildArrayField<RimSummaryCurve*>   m_curves;

    QPointer<RiuSummaryQwtPlot>                  m_qwtPlot;
};
