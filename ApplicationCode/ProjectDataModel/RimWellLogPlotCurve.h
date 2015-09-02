/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"

#include <vector>

class RiuWellLogTracePlot;
class QwtPlotCurve;
class QString;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlotCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogPlotCurve();
    virtual ~RimWellLogPlotCurve();

    void    setPlot(RiuWellLogTracePlot* plot);
    bool    depthRange(double* minimumDepth, double* maximumDepth);
    virtual void                 updatePlotData();

protected:

    // Overridden PDM methods
    virtual void                 fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle* objectToggleField();
    virtual caf::PdmFieldHandle* userDescriptionField();


    caf::PdmField<bool>     m_showCurve;
    caf::PdmField<QString>  m_userName;
    // caf::PdmField<QColor> m_curveColor;
    // caf::PdmField<Linestyle> m_lineStyle;
    // caf::PdmField<int>       m_lineWidth;

    RiuWellLogTracePlot*    m_plot;
    QwtPlotCurve*           m_plotCurve;
};

#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

class RimWellPath;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class RimCase;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogEclipseCurve : public RimWellLogPlotCurve
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogEclipseCurve();
    virtual ~RimWellLogEclipseCurve();
    virtual void updatePlotData();

protected:

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);

    caf::PdmPtrField<RimWellPath*>                  m_wellPath;
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;
};
