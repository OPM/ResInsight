/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimCheckableNamedObject.h"
#include "RimNameConfig.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimCase;
class RimGridCrossPlotCurve;
class RimEclipseResultDefinition;
class QwtPlot;
class QwtPlotCurve;

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotCurveSet : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotCurveSet();
    ~RimGridCrossPlotCurveSet() = default;

    void    loadDataAndUpdate(bool updateParentPlot);
    void    setParentQwtPlotNoReplot(QwtPlot* parent);
    QString xAxisName() const;
    QString yAxisName() const;
protected:
    void initAfterRead() override;
    void onLoadDataAndUpdate(bool updateParentPlot);
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;
    void updateName();
    void triggerReplotAndTreeRebuild();

private:
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmChildField<RimEclipseResultDefinition*> m_xAxisProperty;
    caf::PdmChildField<RimEclipseResultDefinition*> m_yAxisProperty;

    caf::PdmChildArrayField<RimGridCrossPlotCurve*> m_crossPlotCurves;
};
