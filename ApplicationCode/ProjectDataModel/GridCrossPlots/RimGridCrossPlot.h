/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"

#include "RimNameConfig.h"
#include "RimViewWindow.h"

#include <QPointer>

class RimGridCrossPlotCurveSet;
class RiuGridCrossQwtPlot;

class RimGridCrossPlotNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;
public:
    RimGridCrossPlotNameConfig(RimNameConfigHolderInterface* holder = nullptr);
public:
    caf::PdmField<bool> addDataSetNames;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

};

class RimGridCrossPlot : public RimViewWindow, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;
public:
    RimGridCrossPlot();
    ~RimGridCrossPlot();

    RimGridCrossPlotCurveSet* createCurveSet();
    int                       indexOfCurveSet(const RimGridCrossPlotCurveSet* curveSet) const;

    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    void     calculateZoomRangeAndUpdateQwt();
    void     reattachCurvesToQwtAndReplot();
    QString  createAutoName() const override;
    
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 detachAllCurves();

protected:
    QWidget* createViewWidget(QWidget* mainWindowParent) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void     fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;

    void performAutoNameUpdate() override;
    void updatePlot();
    QString xAxisParameterString() const;
    QString yAxisParameterString() const;

private:
    caf::PdmField<bool>                                m_showLegend;
    caf::PdmField<int>                                 m_legendFontSize;
    caf::PdmChildField<RimGridCrossPlotNameConfig*>    m_nameConfig;

    caf::PdmChildArrayField<RimGridCrossPlotCurveSet*> m_crossPlotCurveSets;

    QPointer<RiuGridCrossQwtPlot>                      m_qwtPlot;
    
};



