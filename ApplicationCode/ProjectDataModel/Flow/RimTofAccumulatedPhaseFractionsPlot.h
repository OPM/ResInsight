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

#include <vector>

class RimEclipseResultCase;
class RimWellLogPlot;
class RiuTofAccumulatedPhaseFractionsPlot;
class RiuWellAllocationPlot;

namespace caf {
    class PdmOptionItemInfo;
}

namespace cvf {
    class Color3f;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RimTofAccumulatedPhaseFractionsPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimTofAccumulatedPhaseFractionsPlot();
    virtual ~RimTofAccumulatedPhaseFractionsPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    // RimViewWindow overrides

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;
    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

    void                                            reloadFromWell();

    RimEclipseResultCase*                           resultCase();
    QString                                         tracerName();
    size_t                                          timeStep();

protected:
    // RimViewWindow overrides

    virtual void                                    onLoadDataAndUpdate() override;
    virtual QImage                                  snapshotWindowContent() override;

    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;
    caf::PdmField<int>                              m_maxTof;

    QPointer<RiuTofAccumulatedPhaseFractionsPlot>   m_tofAccumulatedPhaseFractionsPlotWidget;
};
