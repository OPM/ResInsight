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

//==================================================================================================
///
///
//==================================================================================================
class RigWellDiskData
{
public:
    RigWellDiskData();

    void setSinglePropertyValue( double value );
    void setOilGasWater( double oil, double gas, double water );

    double total() const;
    double oil() const;
    double gas() const;
    double water() const;
    double singlePropertyValue() const;
    bool   isSingleProperty() const;

private:
    bool   m_isSingleProperty;
    double m_singlePropertyValue;
    double m_oilValue;
    double m_waterValue;
    double m_gasValue;
};
