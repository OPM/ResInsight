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

#include "RimWellLogPlotCurve.h"

#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

class RimCase;
class RimEclipseResultDefinition;
class RimGeoMechResultDefinition;
class RimWellPath;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogExtractionCurve : public RimWellLogPlotCurve
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogExtractionCurve();
    virtual ~RimWellLogExtractionCurve();
    
    virtual void updatePlotData();

protected:
    virtual QString createCurveName();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);

    virtual void initAfterRead();

    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");

    caf::PdmPtrField<RimWellPath*>                  m_wellPath;
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResultDefinition;
    caf::PdmChildField<RimGeoMechResultDefinition*> m_geomResultDefinition;
    caf::PdmField<int>                              m_timeStep;

private:
    static void validCurvePointIntervals(const std::vector<double>& depthValues, const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals);
    static void addValuesFromIntervals(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals, std::vector<double>* filteredValues);
    static void filteredIntervals(const std::vector< std::pair<size_t, size_t> >& intervals, std::vector< std::pair<size_t, size_t> >* fltrIntervals);
};


