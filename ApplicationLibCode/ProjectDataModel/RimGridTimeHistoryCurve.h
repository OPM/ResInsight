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

#include "RimStackablePlotCurve.h"

#include "RiaDefines.h"

#include "RimGeometrySelectionItem.h"

#include "cafPdmChildField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <memory>

class RigMainGrid;
class RimEclipseCase;
class RimEclipseResultDefinition;
class RimEclipseGeometrySelectionItem;
class RimGeoMechResultDefinition;
class RimGeoMechGeometrySelectionItem;
class RiuFemTimeHistoryResultAccessor;
class RiuSelectionItem;
class RigEclipseResultAddress;
class RimCase;
class RimSummaryPlot;

//==================================================================================================
///
///
//==================================================================================================
class RimGridTimeHistoryCurve : public RimStackablePlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridTimeHistoryCurve();
    ~RimGridTimeHistoryCurve() override;

    static void createCurveFromSelectionItem( const RiuSelectionItem* selectionItem, RimSummaryPlot* plot );

    void setFromSelectionItem( const RiuSelectionItem* selectionItem, bool updateResultDefinition );
    void setFromEclipseCellAndResult( RimEclipseCase* eclCase, size_t gridIdx, size_t i, size_t j, size_t k, const RigEclipseResultAddress& resAddr );
    RiuPlotAxis yAxis() const;
    void        setYAxis( RiaDefines::PlotAxis plotAxis );

    std::vector<double> yValues() const;
    std::vector<time_t> timeStepValues() const;
    std::vector<double> daysSinceSimulationStart() const;

    QString  quantityName() const;
    QString  caseName() const;
    RimCase* gridCase() const;

    void setLocked( bool locked );
    bool isLocked() const;

    RiaDefines::PhaseType phaseType() const override;

protected:
    QString createCurveAutoName() override;
    void    updateZoomInParentPlot() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    RigMainGrid*                     mainGrid();
    RimEclipseGeometrySelectionItem* eclipseGeomSelectionItem() const;
    RimGeoMechGeometrySelectionItem* geoMechGeomSelectionItem() const;
    void                             updateResultDefinitionFromCase();
    QString                          geometrySelectionText() const;
    void                             updateQwtPlotAxis();

    void onPadlockClicked( const SignalEmitter* emitter, size_t index );

    std::unique_ptr<RiuFemTimeHistoryResultAccessor> femTimeHistoryResultAccessor() const;

private:
    caf::PdmField<bool> m_isLocked;

    caf::PdmProxyValueField<QString> m_geometrySelectionText;

    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geoMechResultDefinition;

    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis;

    caf::PdmChildField<RimEclipseGeometrySelectionItem*> m_eclipseDataSource;
    caf::PdmChildField<RimGeoMechGeometrySelectionItem*> m_geoMechDataSource;

    caf::PdmChildField<RimGeometrySelectionItem_OBSOLETE*> m_geometrySelectionItem_OBSOLETE;
};
