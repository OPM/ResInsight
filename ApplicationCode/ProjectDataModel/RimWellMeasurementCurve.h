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

#include "RimWellLogCurve.h"

#include "RiuQwtSymbol.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <vector>

class RimWellPath;
class RimWellMeasurement;

//==================================================================================================
///
//==================================================================================================
class RimWellMeasurementCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementCurve();
    ~RimWellMeasurementCurve() override;

    void         setWellPath( RimWellPath* wellPath );
    RimWellPath* wellPath() const;
    void         setMeasurementKind( const QString& measurementKind );
    QString      measurementKind() const;

    // Overrides from RimWellLogPlotCurve
    QString wellName() const override;
    QString wellLogChannelUiName() const override;
    QString wellLogChannelUnits() const override;

protected:
    // Overrides from RimWellLogCurve
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    // Pdm overrrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    RiuQwtSymbol::PointSymbolEnum getSymbolForMeasurementKind( const QString& measurementKind );
    cvf::Color3f                  getColorForMeasurementKind( const QString& measurementKind );

protected:
    caf::PdmPtrField<RimWellPath*> m_wellPath;
    caf::PdmField<QString>         m_measurementKind;
};
