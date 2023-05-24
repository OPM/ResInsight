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
class RimWellLogCalculatedCurve : public RimWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    enum class Operators
    {
        ADD,
        SUBTRACT,
        DIVIDE,
        MULTIPLY
    };

public:
    RimWellLogCalculatedCurve();
    ~RimWellLogCalculatedCurve() override;

    void setOperator( Operators operatorValue );
    void setWellLogCurves( RimWellLogCurve* firstWellLogCurve, RimWellLogCurve* secondWellLogCurve );

    // Inherited via RimWellLogCurve
    QString wellName() const override;
    QString wellLogChannelUiName() const override;
    QString wellLogChannelUnits() const override;

    static double calculateValue( double firstValue, double secondValue, Operators operatorValue );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    // Inherited via RimWellLogCurve
    QString createCurveAutoName() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    void setAutomaticName();

    void onWellLogCurveChanged( const SignalEmitter* emitter );
    void connectWellLogCurveChangedToSlots( RimWellLogCurve* wellLogCurve );
    void disconnectWellLogCurveChangedFromSlots( RimWellLogCurve* wellLogCurve );

private:
    caf::PdmPtrField<RimCase*> m_case;

    caf::PdmField<caf::AppEnum<Operators>> m_operator;

    caf::PdmPtrField<RimWellLogCurve*> m_firstWellLogCurve;
    caf::PdmPtrField<RimWellLogCurve*> m_secondWellLogCurve;
};
