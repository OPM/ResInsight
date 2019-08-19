/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiaQDateTimeTools.h"
#include "RiuInterfaceToViewWindow.h"
#include "RiuQwtPlot.h"

#include "cafPdmPointer.h"

#include <QPointer>

class RimEnsembleCurveSet;
class RiuCvfOverlayItemWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuSummaryQwtPlot : public RiuQwtPlot
{
    Q_OBJECT;

public:
    RiuSummaryQwtPlot(RimViewWindow* ownerViewWindow, QWidget* parent = nullptr);

    void useDateBasedTimeAxis(const QString& dateFormat, const QString& timeFormat);
    void useTimeBasedTimeAxis();

    void addOrUpdateEnsembleCurveSetLegend(RimEnsembleCurveSet* curveSetToShowLegendFor);
    void removeEnsembleCurveSetLegend(RimEnsembleCurveSet* curveSetToShowLegendFor);

protected:
    void keyPressEvent(QKeyEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void setDefaults();
    void updateLayout() override;
private:
    void updateLegendLayout();
    std::map<caf::PdmPointer<RimEnsembleCurveSet>, QPointer<RiuCvfOverlayItemWidget>> m_ensembleLegendWidgets;

};
