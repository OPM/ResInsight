/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RimEclipseResultCase;
class RimFlowDiagSolution;
class RimRegularLegendConfig;
class RiuMatrixPlotWidget;

//==================================================================================================
///
//==================================================================================================
class RimWellConnectivityTable : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellConnectivityTable();
    ~RimWellConnectivityTable() override;

private:
    void cleanupBeforeClose();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    // Inherited via RimPlotWindow
    virtual QString description() const override;
    virtual void    doRenderWindowContent( QPaintDevice* paintDevice ) override;

    // Inherited via RimViewWindow
    virtual QWidget* viewWidget() override;
    virtual QImage   snapshotWindowContent() override;
    virtual void     zoomAll() override;
    virtual QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    virtual void     deleteViewWidget() override;

    int axisTitleFontSize() const;
    int axisLabelFontSize() const;
    int valueLabelFontSize() const;

private:
    caf::PdmPtrField<RimEclipseResultCase*> m_case;
    caf::PdmPtrField<RimFlowDiagSolution*>  m_flowDiagSolution;

    caf::PdmField<int> m_timeStep; // TODO: Replace with time step logic from WellAllocationOverTimePlot?

    QPointer<RiuMatrixPlotWidget> m_matrixPlotWidget; // Matrix plot to plot table data
    QString                       m_tableTitle = "This is a test title";

    caf::PdmField<int>                          m_rowCount;
    caf::PdmField<int>                          m_colCount;
    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisLabelFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_valueLabelFontSize;
};
