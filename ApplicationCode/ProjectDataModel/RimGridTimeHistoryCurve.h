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

#include "RimDefines.h"

#include "cafPdmChildField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RigMainGrid;
class RimEclipseCase;
class RimEclipseResultDefinition;
class RimEclipseTopologyItem;
class RimPickingTopologyItem;
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
    RimDefines::PlotAxis    yAxis() const;
    void                    setYAxis(RimDefines::PlotAxis plotAxis);

    std::vector<double>     yValues() const;
    std::vector<time_t>     timeStepValues() const;

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
    RimEclipseTopologyItem* eclipseTopologyItem() const;
    void                    updateResultDefinitionFromCase();
    QString                 topologyText() const;
    void                    updateQwtPlotAxis();

private:
    caf::PdmProxyValueField<QString>                m_topologyText;
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimPickingTopologyItem*>     m_pickingTopologyItem;
    caf::PdmField< caf::AppEnum< RimDefines::PlotAxis > > m_plotAxis;
};

