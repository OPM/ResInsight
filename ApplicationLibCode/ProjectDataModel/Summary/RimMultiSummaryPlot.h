/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimPlotWindow.h"
#include "cafPdmChildField.h"

/*
#include "RiaDefines.h"
#include "RiaQDateTimeTools.h"

#include "RifEclipseSummaryAddress.h"

#include "RimPlot.h"

#include "qwt_plot_textlabel.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <memory>
#include <set>

class PdmUiTreeOrdering;
class RimAsciiDataCurve;
class RimGridTimeHistoryCurve;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryCurveCollection;
class RimEnsembleCurveSet;
class RimEnsembleCurveSetCollection;
class RimSummaryCurveFilter_OBSOLETE;
class RimSummaryTimeAxisProperties;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RiuSummaryQwtPlot;
class RimSummaryPlotNameHelper;
class RimPlotTemplateFileItem;
class RimSummaryPlotFilterTextCurveSetEditor;
class RimSummaryPlotSourceStepping;
class RiaSummaryCurveDefinition;

class QwtInterval;
class QwtPlotCurve;

class QKeyEvent;
*/

class RimMultiPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimMultiSummaryPlot : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimMultiSummaryPlot();

    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    QString  description() const override;

private:
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;

    void doRenderWindowContent( QPaintDevice* paintDevice ) override;

private:
    caf::PdmChildField<RimMultiPlot*> m_multiPlot;
};
