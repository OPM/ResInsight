/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimCheckableObject.h"

#include "cafPdmField.h"

//==================================================================================================
///
///
//==================================================================================================
class RimFaciesInitialPressureConfig : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaciesInitialPressureConfig();
    ~RimFaciesInitialPressureConfig() override;

    caf::Signal<> changed;

    void setEnabled( bool enable );
    bool isEnabled() const;

    void           setFaciesName( const QString& name );
    const QString& faciesName() const;

    void setFaciesValue( int faciesValue );
    int  faciesValue() const;

    void   setFraction( double fraction );
    double fraction() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<QString> m_faciesName;
    caf::PdmField<int>     m_faciesValue;
    caf::PdmField<double>  m_fraction;
};
