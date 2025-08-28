/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RiuPlotAnnotationTool.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "RimHistogramPlot.h"
#include "RimStackablePlotCurve.h"

class RimPlotAxisPropertiesInterface;
class RimHistogramDataSource;

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramCurve : public RimStackablePlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimHistogramCurve();
    ~RimHistogramCurve() override;

    // Y Axis functions
    std::string                 unitNameY() const;
    virtual std::vector<double> valuesY() const;
    void                        setLeftOrRightAxisY( RiuPlotAxis plotAxis );
    RiuPlotAxis                 axisY() const;

    // X Axis functions
    std::string                 unitNameX() const;
    virtual std::vector<double> valuesX() const;
    void                        setTopOrBottomAxisX( RiuPlotAxis plotAxis );
    RiuPlotAxis                 axisX() const;

    void updatePlotAxis();
    void updateLegendEntryVisibilityNoPlotUpdate() override;

    void setDataSource( RimHistogramDataSource* dataSource );
    void setAppearanceFromGraphType( RimHistogramPlot::GraphType graphType );

    RimHistogramDataSource* dataSource() const;

protected:
    // RimPlotCurve overrides
    QString createCurveAutoName() override;
    void    updateZoomInParentPlot() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    void loadAndUpdateDataAndPlot();

    void updateLegendsInPlot() override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    bool canCurveBeAttached() const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void hideXAxisGroup();

    void onDataSourceChanged( const caf::SignalEmitter* emitter );

    void initAfterRead() override;

private:
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_yPlotAxisProperties;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_xPlotAxisProperties;

    caf::PdmChildField<RimHistogramDataSource*> m_dataSource;

    caf::PdmField<bool> m_showP10Curve;
    caf::PdmField<bool> m_showP90Curve;
    caf::PdmField<bool> m_showMeanCurve;
    caf::PdmField<bool> m_showValue;

    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
};
