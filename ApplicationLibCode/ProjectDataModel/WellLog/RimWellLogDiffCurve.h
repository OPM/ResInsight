/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimWellLogCurve.h"

#include "cafPdmPtrField.h"
#include "cvfObject.h"

class RimCase;
class RimWellPath;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogDiffCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogDiffCurve();
    ~RimWellLogDiffCurve() override;

    void setWellLogCurves( RimWellLogCurve* wellLogCurveA, RimWellLogCurve* wellLogCurveB );

    // Inherited via RimWellLogCurve
    virtual QString wellName() const override;
    virtual QString wellLogChannelUiName() const override;
    virtual QString wellLogChannelUnits() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    // Inherited via RimWellLogCurve
    virtual QString createCurveAutoName() override;
    virtual void    onLoadDataAndUpdate( bool updateParentPlot ) override;

private:
    caf::PdmPtrField<RimCase*> m_case;

    caf::PdmPtrField<RimWellLogCurve*> m_wellLogCurveA;
    caf::PdmPtrField<RimWellLogCurve*> m_wellLogCurveB;
};
