/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiaDefines.h"
#include "RimPlotWindow.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"

#include <QPointer>

#include <QtCharts/QChartView>

class RimPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimGridStatisticsPlot : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridStatisticsPlot();
    ~RimGridStatisticsPlot() override;

    QWidget* viewWidget() override;
    QWidget* createPlotWidget( QWidget* mainWindowParent = nullptr );
    QString  description() const override;

    void zoomAll() override;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    int subTitleFontSize() const;
    int axisTitleFontSize() const;
    int axisValueFontSize() const;

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;
    void     recreatePlotWidgets();

    // Overridden PDM methods
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void                 initAfterRead() override;
    void                 onLoadDataAndUpdate() override;
    void                 updatePlots();
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    void cleanupBeforeClose();
    void onPlotAdditionOrRemoval();
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;
    void doUpdateLayout() override;

protected:
    QPointer<QtCharts::QChartView> m_viewer; 

    caf::PdmField<QString>                          m_plotWindowTitle;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_subTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisValueFontSize;
};
