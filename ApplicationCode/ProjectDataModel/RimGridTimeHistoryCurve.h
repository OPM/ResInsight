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

#include "RimPlotCurve.h"

#include "RiaDefines.h"

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
class RimGeometrySelectionItem;
class RiuFemTimeHistoryResultAccessor;
class RiuSelectionItem;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGridTimeHistoryCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;
public:

public:
    RimGridTimeHistoryCurve();
    virtual ~RimGridTimeHistoryCurve();

    void                    setFromSelectionItem(const RiuSelectionItem* selectionItem);
    RiaDefines::PlotAxis    yAxis() const;
    void                    setYAxis(RiaDefines::PlotAxis plotAxis);

    std::vector<double>     yValues() const;
    std::vector<time_t>     timeStepValues() const;
    std::vector<double>     daysSinceSimulationStart() const;

    QString                 quantityName() const;
    QString                 caseName() const;

protected:
    virtual QString createCurveAutoName() override;
    virtual void    updateZoomInParentPlot() override;
    virtual void    onLoadDataAndUpdate() override;


    virtual void    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void    initAfterRead() override;
    virtual void    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    RigMainGrid*            mainGrid();
    RimEclipseGeometrySelectionItem* eclipseGeomSelectionItem() const;
    RimGeoMechGeometrySelectionItem* geoMechGeomSelectionItem() const;
    void                    updateResultDefinitionFromCase();
    QString                 geometrySelectionText() const;
    void                    updateQwtPlotAxis();

    std::unique_ptr<RiuFemTimeHistoryResultAccessor> femTimeHistoryResultAccessor() const;

private:
    caf::PdmProxyValueField<QString>                m_geometrySelectionText;
    
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geoMechResultDefinition;

    caf::PdmChildField<RimGeometrySelectionItem*>     m_geometrySelectionItem;
    caf::PdmField< caf::AppEnum< RiaDefines::PlotAxis > > m_plotAxis;
};

